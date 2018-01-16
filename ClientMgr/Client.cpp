#include "Client.h"

#include <sstream>
#include <iomanip>
#include <timeapi.h>

#include "Errors.h"

Client::Client( ) :
	Manager( *this ), is_running( true ),
	update_last( 0 ), render_last( 0 ), update_cnt( 0 ), render_cnt( 0 ),
	resource_mgr( client ), texture_mgr( client ), time_mgr( client ), thread_mgr( client ),
	display_mgr( client ), input_mgr( client ), gui_mgr( client ), block_mgr( client ),
	biome_mgr( client ), chunk_mgr( client ), entity_mgr( client ) { }

Client::~Client( ) { }

void Client::thread_main_loop( ) {
	is_running = true;

	time_mgr.set_time( TimeStrings::GAME, 0 );
	time_mgr.set_time( TimeStrings::GAME_ACCUM, 0 );
	time_mgr.set_time( TimeStrings::RENDER_ACCUM, 0 );
	time_mgr.begin_record( RecordStrings::FRAME );

	while( is_running ) {
		main_loop( );
	}
}

static int cnt_sleep = 0;

void Client::main_loop( ) { 
	time_mgr.end_record( RecordStrings::FRAME );

	time_mgr.begin_record( RecordStrings::UPDATE_PRE );
	if( time_mgr.get_record_curr( RecordStrings::FRAME ) > TIME_FRAME_MILLI / 1000.0f ) {
		if( time_mgr.get_record_curr( RecordStrings::FRAME ) > TIME_MILLISEC / 4.0f ) {
			time_mgr.add_time( TimeStrings::GAME_ACCUM, TIME_MILLISEC / 4.0f );
		}
		else {
			time_mgr.add_time( TimeStrings::GAME_ACCUM, time_mgr.get_record_curr( RecordStrings::FRAME ) );
		}

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
	//if( time_main < 0.0f ) time_main = 0.0f;
	if( time_main < 1.5f ) time_main = 1.5f;
	thread_mgr.loop_main( time_main );

	client.time_mgr.begin_record( RecordStrings::RENDER_SWAP );
	display_mgr.swap_buffers( );
	client.time_mgr.push_record( RecordStrings::RENDER_SWAP );
	client.time_mgr.end_record( RecordStrings::RENDER_SWAP );

	if( !display_mgr.is_vsync && display_mgr.is_limiter ) {
		
		///*
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
		//*/
		
		/*
		static float time_min_prec = 5.0f;

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

			if( time_sleep > time_min_prec ) { 
				while( ( time_curr_sleep = time_mgr.get_record_curr( RecordStrings::SLEEP ) ) < time_sleep - time_min_prec ) {
					std::this_thread::sleep_for( std::chrono::nanoseconds( 0 ) );
					time_mgr.end_record( RecordStrings::SLEEP );
					cnt_sleep += 1;
				}
			}

			while( ( time_curr_sleep = time_mgr.get_record_curr( RecordStrings::SLEEP ) ) < time_sleep ) {
				time_mgr.end_record( RecordStrings::SLEEP );
			}

			time_mgr.push_record( RecordStrings::SLEEP );
		}

		*/
		
	}
}

void Client::init( ) {
	init_mgrs( );

	thread_mgr.post_init( );

	#if defined( GL_DEBUG ) || defined( _DEBUG )
	system( "PAUSE" );
	#endif

	thread_main_loop( );
}

#include "BlockSet.h"

void Client::init_mgrs( ) { 
	resource_mgr.init( );
	thread_mgr.init( );
	display_mgr.init( );

	GL_CHECK( time_mgr.init( ) );
	GL_CHECK( input_mgr.init( ) );
	GL_CHECK( texture_mgr.init( ) );
	GL_CHECK( gui_mgr.init( ) );

	GL_CHECK( block_mgr.init( ) );
	GL_CHECK( biome_mgr.init( ) );
	GL_CHECK( chunk_mgr.init( ) );
	GL_CHECK( entity_mgr.init( ) );

	auto & out = display_mgr.out;
	out.str( "" );
	out << "BlockSet sizeof: " << sizeof( BlockSet< WorldSize::Chunk::size_x, WorldSize::Chunk::size_z > );
	gui_mgr.print_to_console( out.str( ) );

	out.str( "" );
	out << "BlockSet::mat_runs sizeof: " << sizeof( BlockSet< WorldSize::Chunk::size_x, WorldSize::Chunk::size_z >::mat_runs );
	gui_mgr.print_to_console( out.str( ) );

	//out.str( "" );
	//out << "BlockSet::mat_mutex sizeof: " << sizeof( BlockSet< WorldSize::Chunk::size_x, WorldSize::Chunk::size_z >::mat_mutex );
	gui_mgr.print_to_console( out.str( ) );
}

void Client::update( ) {
	//gui_mgr.clear_static( );
	auto & out = display_mgr.out;
	out.str( "" );
	out << "[FRAMES] Update: " << std::setw( 4 ) << update_last << 
		" Render: " << std::setw( 4 ) << render_last << 
		" MS: " << std::setw( 4 ) << ( 1000.0f / render_last );
	//gui_mgr.print_to_static( out.str( ) );

	GL_CHECK( input_mgr.update( ) );

	time_mgr.begin_record( RecordStrings::UPDATE_TIME );
	GL_CHECK( time_mgr.update( ) );
	time_mgr.end_record( RecordStrings::UPDATE_TIME );
	time_mgr.push_record( RecordStrings::UPDATE_TIME );

	GL_CHECK( display_mgr.update( ) );
	GL_CHECK( texture_mgr.update( ) );

	time_mgr.begin_record( RecordStrings::UPDATE_GUI );
	GL_CHECK( gui_mgr.update( ) );
	time_mgr.end_record( RecordStrings::UPDATE_GUI );
	time_mgr.push_record( RecordStrings::UPDATE_GUI );

	GL_CHECK( chunk_mgr.update( ) );
	GL_CHECK( entity_mgr.update( ) );
	GL_CHECK( thread_mgr.update( ) );
}

void Client::render( ) {
	display_mgr.clear_buffers( );
	display_mgr.set_camera( );

	client.time_mgr.begin_record( RecordStrings::RENDER_DRAW );

	GL_CHECK( chunk_mgr.render( ) );

	GL_CHECK( entity_mgr.render( ) );

	GL_CHECK( render_output( ) );
	client.time_mgr.end_record( RecordStrings::RENDER_DRAW );
	client.time_mgr.push_record( RecordStrings::RENDER_DRAW );
}

void Client::render_output( ) {
	client.texture_mgr.unbind_program( );

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );

	display_mgr.set_ortho( );

	display_mgr.draw_key( 30 );
	
	glEnable( GL_TEXTURE_2D );

	glDisable( GL_CULL_FACE );

	glEnable( GL_CULL_FACE );

	GL_CHECK( gui_mgr.render( ) );
	GL_CHECK( time_mgr.render( ) );

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
	cnt_sleep = 0;

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

int Client::cnt_update( ) { 
	return update_cnt;
}