#include "InputMgr.h"

#include "Client.h"
#include "Tnt.h"
#include "WorldSize.h"

#include <iostream>

// *** Input Manager ***
InputMgr::InputMgr( Client & client ) :
	Manager( client ) {}

InputMgr::~InputMgr() {}

void InputMgr::init() {
	printTabbedLine( 0, "Init InputMgr..." );
	toggle_cursor_vis( );
	last_command = client.time_mgr.get_time( std::string( "Game" ) );
	toggle_cursor_vis( );
	printTabbedLine( 0, "...Init InputMgr" );
	std::cout << std::endl;
}

void InputMgr::update() {
	poll_mouse_pos();
	resolve_mouse_action();

	process_input();
}

void InputMgr::poll_mouse_pos() {
	mouse.pos_reset = client.display_mgr.get_window() / 2;

	GetCursorPos( &mouse.point_poll );
	ScreenToClient( client.display_mgr.get_HWND( ), &mouse.point_poll );

	mouse.pos_last = mouse.pos;
	mouse.pos = glm::ivec2( mouse.point_poll.x, client.display_mgr.get_window().y - mouse.point_poll.y );
	mouse.pos_delta = mouse.pos - mouse.pos_last;

	mouse.wheel_delta = mouse.accum_wheel_delta;
	mouse.accum_wheel_delta = 0;

	if( !mouse.is_visible ) {
		mouse.pos_last = mouse.pos_reset;
		mouse.pos = mouse.pos_reset;

		mouse.point_poll.x = mouse.pos.x;
		mouse.point_poll.y = client.display_mgr.get_window().y - mouse.pos.y;

		ClientToScreen( client.display_mgr.get_HWND( ), &mouse.point_poll );
		SetCursorPos( mouse.point_poll.x, mouse.point_poll.y );
	}
}

void InputMgr::resolve_mouse_action() {
	if( mouse.action[ 0 ] == false &&
		mouse.down[ 0 ] == true ) {
		mouse.down[ 0 ] = false;
	}

	if( mouse.action[ 0 ] == false &&
		mouse.up[ 0 ] == true ) {
		mouse.up[ 0 ] = false;
	}

	if( mouse.hold[ 0 ] != 0 ) {
		mouse.hold[ 0 ] += 1;
	}

	mouse.action[ 0 ] = false;

	if( mouse.action[ 1 ] == false &&
		mouse.down[ 1 ] == true ) {
		mouse.down[ 1 ] = false;
	}

	if( mouse.action[ 1 ] == false &&
		mouse.up[ 1 ] == true ) {
		mouse.up[ 1 ] = false;
	}

	if( mouse.hold[ 1 ] != 0 ) {
		mouse.hold[ 1 ] += 1;
	}

	mouse.action[ 1 ] = false;
}

const static int time_cmd_cd = 100;
static int time_last_cmd = 0;
static const float move_speed_def = 5.0f;

void InputMgr::process_input( ) {
	auto & camera = client.display_mgr.camera;
	float move_speed = move_speed_def;

	if( is_key( VK_SHIFT ) ) move_speed = 15.0f;
	if( is_key( VK_CONTROL ) ) move_speed = 100.0f;

	if( is_key( 'W' ) ) {
		camera.pos_camera += camera.vec_front * move_speed * DELTA_CORRECT;
	}
	else if( is_key( 'S' ) ) {
		camera.pos_camera -= camera.vec_front * move_speed * DELTA_CORRECT;
	}

	if( is_key( 'A' ) ) {
		camera.pos_camera += camera.vec_left * move_speed * DELTA_CORRECT;
	}
	else if( is_key( 'D' ) ) {
		camera.pos_camera -= camera.vec_left * move_speed * DELTA_CORRECT;
	}

	if( is_key( 'E' ) ) { 
		camera.pos_camera += camera.vec_up * move_speed * DELTA_CORRECT;
	}
	else if( is_key( 'Q' ) ) { 
		camera.pos_camera -= camera.vec_up * move_speed * DELTA_CORRECT;
	}

	if( is_key( VK_ESCAPE ) ) { 
		client.is_running = false;
	}

	int time = client.time_mgr.get_time( TimeStrings::GAME );
	if( time - time_last_cmd > time_cmd_cd ) { 
		if( is_key( '1' ) ) { 
			toggle_cursor_vis( );
			time_last_cmd = time;
		}
		else if( is_key( '2' ) ) { 
			client.chunk_mgr.toggle_chunk_debug( );
			time_last_cmd = time;
		}
		else if( is_key( '3' ) ) { 
			client.display_mgr.out.str( "" );
			client.display_mgr.out << "Returning home!!!";
			client.gui_mgr.print_to_console( client.display_mgr.out.str( ) );

			client.display_mgr.camera.pos_camera = glm::vec3( 0, WorldSize::Chunk::size_y / 2 + 5, 0 );
			time_last_cmd = time;
		}
		else if( is_key( '4' ) ) {
			client.display_mgr.out.str( "" );
			client.display_mgr.out <<
				Directional::print_vec( client.display_mgr.camera.pos_camera ) <<
				" Block: " << client.chunk_mgr.get_block_string( client.chunk_mgr.get_block( client.display_mgr.camera.pos_camera ) );
			client.gui_mgr.print_to_console( client.display_mgr.out.str( ) );

			time_last_cmd = time;
		}
		else if( is_key( '5' ) ) {					
			client.thread_mgr.task_async( 10, [ & ] ( ) { 
				float step = 0.05f;
				float max = 100.0f;
				int num_steps = max / step;

				glm::vec3 pos_gw = client.display_mgr.camera.pos_camera;
				glm::vec3 pos_step = client.display_mgr.camera.vec_front * step;

				for( int i = 0; i < num_steps; i++ ) {
					pos_gw += pos_step;
					if( client.chunk_mgr.get_block( pos_gw + pos_step ) != -1 ) {
						client.chunk_mgr.set_sphere( pos_gw, 100, client.gui_mgr.block_selector.get_id_block( ) );
						break;
					}
				}
			} );
			time_last_cmd = time;
		}
		else if( is_key( '6' ) ) {
			client.thread_mgr.task_async( 10, [ & ] ( ) {
				float step = 0.05f;
				float max = 100.0f;
				int num_steps = max / step;

				glm::vec3 pos_gw = client.display_mgr.camera.pos_camera;
				glm::vec3 pos_step = client.display_mgr.camera.vec_front * step;

				for( int i = 0; i < num_steps; i++ ) {
					pos_gw += pos_step;
					if( client.chunk_mgr.get_block( pos_gw + pos_step ) != -1 ) {
						client.chunk_mgr.set_rect( pos_gw, glm::vec3( 1, 80, 1 ), client.gui_mgr.block_selector.get_id_block( ) );
						break;
					}
				}
			} );
			time_last_cmd = time;
		}
		else if( is_key( '7' ) ) {
			client.display_mgr.out.str( "" );
			client.display_mgr.out <<
				Directional::print_vec( client.display_mgr.camera.pos_camera ) <<
				" Block: " << client.chunk_mgr.get_block_string( client.chunk_mgr.get_block( client.display_mgr.camera.pos_camera ) );
			client.gui_mgr.print_to_console( client.display_mgr.out.str( ) );
			time_last_cmd = time;
		}
		else if( is_key( '8' ) ) {
			client.gui_mgr.page_show_all( );
			time_last_cmd = time;
		}
		else if( is_key( 'F' ) ) {
			client.entity_mgr.entity_add(
				"Line Block",
				client.entity_mgr.custom_base
			);

			time_last_cmd = time;
		}
		else if( is_key( 'Z' ) ) {
			static int select = 0;

			if( select % 3 == 0 ) { 
				client.entity_mgr.entity_add(
					"Spin Block",
					[ & ] ( Client & client, Entity & entity ) { 
						entity.h_state.get( ).rot_veloc = { 180.0f, 0.0f, 0.0f };
						return ErrorEntity::EE_Ok;
					}
				);
			}
			else if( select % 3 == 1 ) {
				client.entity_mgr.entity_add(
					"Spin Block",
					[ & ] ( Client & client, Entity & entity ) {
					entity.h_state.get( ).rot_veloc = { 0.0f, 180.0f, 0.0f };
					return ErrorEntity::EE_Ok;
				}
				);
			}
			else if( select % 3 == 2 ) {
				client.entity_mgr.entity_add(
					"Spin Block",
					[ & ] ( Client & client, Entity & entity ) {
					entity.h_state.get( ).rot_veloc = { 0.0f, 0.0f, 180.0f };
					return ErrorEntity::EE_Ok;
				}
				);
			}

			select++;
			time_last_cmd = time;
		}
		else if( is_key( 'X' ) ) {
			client.entity_mgr.entity_add(
				"Spawn Block",
				client.entity_mgr.custom_base
			);

			time_last_cmd = time;
		}
		else if( is_key( 'G' ) ) { 
			glm::vec3 pos_gw = client.display_mgr.camera.pos_camera + client.display_mgr.camera.vec_front * 10.0f;

			client.entity_mgr.entity_add( 
				"Tnt",
				[ &, pos_gw ] ( Client & client, Entity & entity ) {
					auto & block_tnt = client.chunk_mgr.get_block_data( "Tnt" );
					entity.id = block_tnt.id;
					entity.color = client.chunk_mgr.get_block_data( block_tnt.id ).color;

					auto & ec_state = entity.h_state.get( );
					ec_state.pos = glm::floor( pos_gw );

					auto & ec_tnt = entity.get_data< ECTnt >( ).get( );
					ec_tnt.time_life = 5000;

					return ErrorEntity::EE_Ok;
				}
			);

			time_last_cmd = time;
		}
		else if( is_key( 'H' ) ) {
			Emitter e;
			auto & camera = client.display_mgr.camera.pos_camera;
			e.pos = glm::vec4( camera.x, camera.y, camera.z, 0.0f );
			e.color = glm::vec4(
				std::rand( ) % 255 / 255.0f,
				std::rand( ) % 255 / 255.0f,
				std::rand( ) % 255 / 255.0f,
				0.0f
			);

			e.radius = std::rand( ) % 20 + 20;

			client.chunk_mgr.add_emitter( e );

			time_last_cmd = time;
		}

		if( !client.input_mgr.is_cursor_vis() ) {
			if( get_mouse_hold( 0 ) ) {
				float step = 0.05f;
				float max = 100.0f;
				int num_steps = max / step;

				glm::vec3 pos_gw = client.display_mgr.camera.pos_camera;
				glm::vec3 pos_step = client.display_mgr.camera.vec_front * step;

				for( int i = 0; i < num_steps; i++ ) {
					pos_gw += pos_step;
					if( client.chunk_mgr.get_block( pos_gw + pos_step ) != -1 ) {
						client.chunk_mgr.set_block( pos_gw, client.gui_mgr.block_selector.get_id_block( ) );
						break;
					}
				}

				time_last_cmd = time;
			}
			else if( get_mouse_hold( 1 ) ) {
				float step = 0.05f;
				float max = 100.0f;
				int num_steps = max / step;

				glm::vec3 pos_gw = client.display_mgr.camera.pos_camera;
				glm::vec3 pos_step = client.display_mgr.camera.vec_front * step;

				for( int i = 0; i < num_steps; i++ ) {
					pos_gw += pos_step;
					if( client.chunk_mgr.get_block( pos_gw ) != -1 ) {
						client.chunk_mgr.set_block( pos_gw, -1 );
						break;
					}
				}

				time_last_cmd = time;
			}
		}
	}
}

void InputMgr::set_key( int const key, bool const is_down ) {
	if( key == VK_RETURN && is_down ) {
		client.gui_mgr.toggle_input( );
	}
	else if( client.gui_mgr.get_is_input( ) ) {
		client.gui_mgr.handle_input_char( key, is_down );
	}
	else {
		keyboard.keys[ key ] = is_down;
	}
}

bool InputMgr::is_key( int const key ) {
	return keyboard.keys[ key ];
}

glm::ivec2 & InputMgr::get_mouse() {
	return mouse.pos;
}

glm::ivec2 & InputMgr::get_mouse_relative() {
	return mouse.pos_relative;
}

glm::ivec2 & InputMgr::get_mouse_delta() {
	return mouse.pos_delta;
}

glm::ivec2 & InputMgr::get_mouse_down( int const button ) {
	return mouse.pos_down;
}

glm::ivec2 & InputMgr::get_mouse_up( int const button ) {
	return mouse.pos_up;
}

int InputMgr::get_mouse_dur( int const button ) {
	return mouse.dur[ button ];
}

int InputMgr::get_mouse_hold( int const button ) {
	return mouse.hold[ button ];
}

void InputMgr::set_mouse( glm::ivec2 const & new_pos ) {
	POINT new_point;
	new_point.x = new_pos.x;
	new_point.y = new_pos.y;
	new_point.y = client.display_mgr.get_window().y - new_point.y;
	ClientToScreen( client.display_mgr.get_HWND( ), &new_point );
	SetCursorPos( new_point.x, new_point.y );
}

void InputMgr::set_mouse_button( int const button, bool const is_down ) {
	if( is_down ) {
		mouse.down[ button ] = true;
		mouse.action[ button ] = true;

		mouse.hold[ button ] = 1;

		mouse.pos_down = mouse.pos;
	}
	else {
		mouse.down[ button ] = false;
		mouse.up[ button ] = true;
		mouse.action[ button ] = true;

		mouse.dur[ button ] = mouse.hold[ button ];
		mouse.hold[ button ] = 0;

		mouse.pos_up = mouse.pos;
	}
}

bool InputMgr::is_mouse_down( int const button ) {
	return mouse.down[ button ];
}

bool InputMgr::is_mouse_up( int const button ) {
	return mouse.up[ button ];
}

void InputMgr::add_wheel_delta( int const wheel_delta ) {
	mouse.accum_wheel_delta += wheel_delta;
}

int InputMgr::get_wheel_delta() {
	return mouse.wheel_delta;
}

bool InputMgr::is_cursor_vis( ) {
	return mouse.is_visible;
}

void InputMgr::toggle_cursor_vis() {
	mouse.is_visible = !mouse.is_visible;

	if( mouse.is_visible ) {
		ShowCursor( true );
	}
	else {
		mouse.pos_delta = glm::ivec2( 0, 0 );
		mouse.pos_last = mouse.pos_reset;
		mouse.pos = mouse.pos_reset;
		mouse.point_poll.x = mouse.pos.x;
		mouse.point_poll.y = client.display_mgr.get_window( ).y - mouse.pos.y;
		ClientToScreen( client.display_mgr.get_HWND( ), &mouse.point_poll );
		SetCursorPos( mouse.point_poll.x, mouse.point_poll.y );

		ShowCursor( false );
	}
}