#include "Client.h"

#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <timeapi.h>

Client::Client( ) :
	Manager( *this ),
	is_running( true ),
	update_last( 0 ), render_last( 0 ),
	update_cnt( 0 ), render_cnt( 0 ),
	resource_mgr( client ), texture_mgr( client ),
	time_mgr( client ), thread_mgr( client ),
	display_mgr( client ), input_mgr( client ),
	gui_mgr( client ), chunk_mgr( client ),
	entity_mgr( client ) { }

Client::~Client( ) { }

void Client::thread_main_loop( ) {
	MSG msg;
	is_running = true;

	time_mgr.set_time( TimeStrings::GAME, 0 );
	time_mgr.set_time( TimeStrings::GAME_ACCUM, 0 );
	time_mgr.set_time( TimeStrings::RENDER_ACCUM, 0 );
	time_mgr.begin_record( RecordStrings::FRAME );

	while( is_running ) {
		time_mgr.end_record( RecordStrings::FRAME );

		time_mgr.begin_record( RecordStrings::UPDATE_PRE );
		if( time_mgr.get_record_curr( RecordStrings::FRAME ) > TIME_FRAME_MILLI / 1000.0f ) {
			time_mgr.add_time( TimeStrings::GAME_ACCUM, time_mgr.get_record_curr( RecordStrings::FRAME ) );
			//time_mgr.add_time( TimeStrings::RENDER_ACCUM, time_mgr.get_record_curr( RecordStrings::FRAME ) );

			time_mgr.push_record( RecordStrings::FRAME );
			time_mgr.begin_record( RecordStrings::FRAME );
		}

		while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		time_mgr.end_record( RecordStrings::UPDATE_PRE );
		time_mgr.push_record( RecordStrings::UPDATE_PRE );

		time_mgr.begin_record( RecordStrings::UPDATE );

		while( time_mgr.get_time( TimeStrings::GAME_ACCUM ) > TIME_FRAME_MILLI ) {
			if( time_mgr.get_time( TimeStrings::GAME ) -
				time_mgr.get_time( TimeStrings::SEC ) >
				TIME_MILLISEC ) {
				sec( );
				time_mgr.add_time( TimeStrings::SEC, TIME_MILLISEC );
			}

			update( );
			update_cnt++;

			time_mgr.add_time( TimeStrings::GAME_ACCUM, -TIME_FRAME_MILLI );
			time_mgr.add_time( TimeStrings::GAME, TIME_FRAME_MILLI );
		}

		time_mgr.end_record( RecordStrings::UPDATE );
		time_mgr.push_record( RecordStrings::UPDATE );

		time_mgr.begin_record( RecordStrings::RENDER );

		render( );
		render_cnt++;

		time_mgr.end_record( RecordStrings::RENDER );
		time_mgr.push_record( RecordStrings::RENDER );

		float time_main = TIME_FRAME_MILLI -
			time_mgr.get_record_curr( RecordStrings::UPDATE_PRE ) -
			time_mgr.get_record_curr( RecordStrings::UPDATE ) -
			time_mgr.get_record_curr( RecordStrings::RENDER ) -
			0.25f;
		//if( time_main < 0.5f ) time_main = 0.5f;
		thread_mgr.loop_main( time_main );

		client.time_mgr.begin_record( RecordStrings::RENDER_SWAP );
		display_mgr.swap_buffers( );
		client.time_mgr.push_record( RecordStrings::RENDER_SWAP );
		client.time_mgr.end_record( RecordStrings::RENDER_SWAP );
		
		if( !display_mgr.is_vsync && display_mgr.is_limiter ) {
			float time_sleep = TIME_FRAME_MILLI -
				time_mgr.get_record_curr( RecordStrings::UPDATE_PRE ) -
				time_mgr.get_record_curr( RecordStrings::UPDATE ) -
				time_mgr.get_record_curr( RecordStrings::TASK_MAIN ) -
				time_mgr.get_record_curr( RecordStrings::RENDER ) -
				time_mgr.get_record_curr( RecordStrings::RENDER_SWAP );

			if( time_sleep > 0 ) {
				time_mgr.begin_record( RecordStrings::SLEEP );
				time_mgr.end_record( RecordStrings::SLEEP );

				float time_curr_sleep;
				while( ( time_curr_sleep = time_mgr.get_record_curr( RecordStrings::SLEEP ) ) < time_sleep ) {
					time_mgr.end_record( RecordStrings::SLEEP );
				}

				time_mgr.push_record( RecordStrings::SLEEP );
			}
		}
	}
}

void Client::init( ) {
	resource_mgr.init( );
	thread_mgr.init( );
	display_mgr.init( );
	gui_mgr.init( );

	// Init things that do not depend on opengl
	time_mgr.init( );
	input_mgr.init( );
	texture_mgr.init( );
	chunk_mgr.init( );
	entity_mgr.init( );

	// Sets pointer to thing
	SetWindowLongPtr( display_mgr.get_HWND(), GWLP_USERDATA, ( LONG_PTR )this );

	thread_main_loop( );
}

void Client::update( ) {
	gui_mgr.clear_static( );
	auto & out = display_mgr.out;
	out.str( "" );
	out << "[FRAMES] Update: " << std::setw( 4 ) << update_last << " Render: " << std::setw( 4 ) << render_last;
	gui_mgr.print_to_static( out.str( ) );

	input_mgr.update( );

	time_mgr.begin_record( RecordStrings::UPDATE_TIME );
	time_mgr.update( );
	time_mgr.end_record( RecordStrings::UPDATE_TIME );
	time_mgr.push_record( RecordStrings::UPDATE_TIME );

	display_mgr.update( );
	texture_mgr.update( );

	time_mgr.begin_record( RecordStrings::UPDATE_GUI );
	gui_mgr.update( );
	time_mgr.end_record( RecordStrings::UPDATE_GUI );
	time_mgr.push_record( RecordStrings::UPDATE_GUI );

	chunk_mgr.update( );
	entity_mgr.update( );
	thread_mgr.update( );

	gui_mgr.update_static( );
}

void Client::render( ) {
	display_mgr.clear_buffers( );
	display_mgr.set_camera( );

	client.time_mgr.begin_record( RecordStrings::RENDER_DRAW );

	chunk_mgr.render( );

	entity_mgr.render( );

	client.texture_mgr.unbind_program( );

	render_output( );
	client.time_mgr.end_record( RecordStrings::RENDER_DRAW );
	client.time_mgr.push_record( RecordStrings::RENDER_DRAW );

	/*client.time_mgr.begin_record( RecordStrings::RENDER_SWAP );

	display_mgr.swap_buffers( );

	client.time_mgr.end_record( RecordStrings::RENDER_SWAP );
	client.time_mgr.push_record( RecordStrings::RENDER_SWAP );*/
}

void Client::render_output( ) {
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );

	display_mgr.set_ortho( );

	display_mgr.draw_key( 30 );
	
	glEnable( GL_TEXTURE_2D );

	gui_mgr.render( );
	time_mgr.render( );
	display_mgr.block_selector.render( );

	glEnable( GL_LIGHTING );

	display_mgr.set_proj( );
}

void Client::end( ) {
	thread_mgr.end( );
	entity_mgr.end( );
	chunk_mgr.end( );
	gui_mgr.end( );
	input_mgr.end( );
	display_mgr.end( );
	resource_mgr.end( );
	time_mgr.end( );
}

void Client::sec( ) {
	resource_mgr.sec( );
	time_mgr.sec( );
	thread_mgr.sec( );
	display_mgr.sec( );
	chunk_mgr.sec( );

	update_last = update_cnt;
	update_cnt = 0;
	render_last = render_cnt;
	render_cnt = 0;
}

LRESULT CALLBACK Client::WndProc( HWND p_hWnd, UINT p_uiMessage, WPARAM p_wParam, LPARAM p_lParam ) {
	switch( p_uiMessage ) {
		case WM_CLOSE:
		is_running = false;
		return 0;

		case WM_LBUTTONDOWN:
		input_mgr.set_mouse_button( 0, true );
		return 0;

		case WM_RBUTTONDOWN:
		input_mgr.set_mouse_button( 1, true );
		return 0;
		break;

		case WM_LBUTTONUP:
		input_mgr.set_mouse_button( 0, false );
		return 0;

		case WM_RBUTTONUP:
		input_mgr.set_mouse_button( 1, false );
		return 0;
		break;

		case WM_MOUSEWHEEL:
		input_mgr.add_wheel_delta( GET_WHEEL_DELTA_WPARAM( p_wParam ) / 120.0f );
		return 0;

		case WM_KEYDOWN:
		input_mgr.set_key( p_wParam, true );
		return 0;

		case WM_KEYUP:
		input_mgr.set_key( p_wParam, false );
		return 0;

		case WM_SIZE:
		display_mgr.resize_window( glm::ivec2( LOWORD( p_lParam ), HIWORD( p_lParam ) ) );
		return 0;

		default:
		return DefWindowProc( p_hWnd, p_uiMessage, p_wParam, p_lParam );
	}
}
