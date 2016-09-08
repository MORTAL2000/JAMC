#include "Player.h"
#include "WormBlock.h"
#include "Client.h"

Player::Player( ) :
	EntityLoader {
		"Player",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECPlayer >( client ) ) {
				return ErrorEntity::EE_Failed;
			}

			entity.id = 0;

			auto & ec_state = entity.h_state.get( );
			ec_state.is_gravity = true;
			ec_state.dim = { 0.8f, 1.8f, 0.8f };

			ec_state.pos = { 0.0f, Chunk::size_y / 2.0f + 5.0f, 0.0f };
			ec_state.rot.y = 45;
			entity.is_visible = false;

			auto & ec_player = entity.get_data< ECPlayer >( ).get( );
			ec_player.veloc_vmax = 9.81f * 4.0f;
			ec_player.veloc_hmax = 32.0f;

			ec_player.veloc_walk = 6.0f;
			ec_player.veloc_run = 24.0f;
			ec_player.accel_walk = 24.0f;
			ec_player.accel_run = 48.0f;

			ec_player.is_godmode = false;
			ec_player.cld_input = 250;
			ec_player.last_input = client.time_mgr.get_time( TimeStrings::GAME );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			entity.clear_data< ECPlayer >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_player = entity.get_data< ECPlayer >( ).get( );
			float mouse_sensitivity = 0.1f;
			int time_now = client.time_mgr.get_time( TimeStrings::GAME );

			if( time_now - ec_player.last_input > ec_player.cld_input ) {
				if( client.input_mgr.is_key( VK_OEM_3 ) ) {
					ec_player.is_godmode = !ec_player.is_godmode;
					if( ec_player.is_godmode ) {
						ec_state.is_gravity = false;
					}
					else {
						ec_state.is_gravity = true;
					}
					if( ec_player.is_godmode ) {
						client.gui_mgr.print_to_console( "Godmode set!" );
					}
					else {
						client.gui_mgr.print_to_console( "Godmode unset!" );
					}

					ec_player.last_input = time_now;
				}
				else if( client.input_mgr.is_key( 'Y' ) ) { 
					client.thread_mgr.task_main( 10, [ & ] ( ) {
						client.entity_mgr.entity_add( "WormBlock", [ & ] ( Client & client, Entity & entity ) {
							auto & ec_state_block = entity.h_state.get( );
							ec_state_block.pos = ec_state.pos;

							return ErrorEntity::EE_Ok;
						} );
					} );

					ec_player.last_input = time_now;
				}
				else if( client.input_mgr.is_key( 'U' ) ) {
					client.thread_mgr.task_main( 10, [ & ] ( ) {
						client.entity_mgr.entity_add( "Water Block", [ & ] ( Client & client, Entity & entity ) {
							client.gui_mgr.print_to_console( "Adding water thing" );
							auto & ec_state_block = entity.h_state.get( );
							ec_state_block.pos = ec_state.pos;

							return ErrorEntity::EE_Ok;
						} );
					} );

					ec_player.last_input = time_now;
				}
				else if( client.input_mgr.is_key( 'P' ) ) {
					client.chunk_mgr.next_skybox( );

					ec_player.last_input = time_now;
				}
			}

			if( !ec_player.is_godmode ) { 
				client.entity_mgr.entity_stop( ec_state );

				if( !client.input_mgr.is_cursor_vis( ) ) {
					ec_state.rot.x -= client.input_mgr.get_mouse_delta( ).y * mouse_sensitivity;
					ec_state.rot.y += client.input_mgr.get_mouse_delta( ).x * mouse_sensitivity;

					while( ec_state.rot.x >= 360 ) ec_state.rot.x -= 360;
					while( ec_state.rot.x < 0 ) ec_state.rot.x += 360;
					if( ec_state.rot.x > 180 && ec_state.rot.x < 270 ) ec_state.rot.x = 270;
					if( ec_state.rot.x < 180 && ec_state.rot.x > 90 ) ec_state.rot.x = 90;
				}

				glm::vec3 accel_input = { 0, 0, 0 };

				if( client.input_mgr.is_key( 'W' ) ||
					client.input_mgr.is_key( VK_UP ) ) {
					accel_input += Directional::get_fwd_aa( ec_state.rot );
				}
				else if( client.input_mgr.is_key( 'S' ) ||
					client.input_mgr.is_key( VK_DOWN ) ) {
					accel_input -= Directional::get_fwd_aa( ec_state.rot );
				}

				if( client.input_mgr.is_key( 'A' ) ||
					client.input_mgr.is_key( VK_LEFT ) ) {
					accel_input += Directional::get_left( ec_state.rot );
				}
				else if( client.input_mgr.is_key( 'D' ) ||
					client.input_mgr.is_key( VK_RIGHT ) ) {
					accel_input -= Directional::get_left( ec_state.rot );
				}

				if( ec_state.is_coll_face[ FaceDirection::FD_Down ] &&
					client.input_mgr.is_key( VK_SPACE ) ) {
					ec_state.veloc.y += 5.2f;
				}

				if( accel_input != glm::vec3 { 0, 0, 0 } )
					accel_input = glm::normalize( accel_input );

				glm::vec2 veloc_xz = glm::vec2( ec_state.veloc.x, ec_state.veloc.z );

				if( client.input_mgr.is_key( VK_SHIFT ) ) {
					accel_input *= ec_player.accel_run;

					if( glm::length( veloc_xz ) > ec_player.veloc_run ) {
						veloc_xz = glm::normalize( veloc_xz ) *  ec_player.veloc_run;
						ec_state.veloc.x = veloc_xz.x;
						ec_state.veloc.z = veloc_xz.y;
					}
				}
				else {
					accel_input *= ec_player.accel_walk;

					if( glm::length( veloc_xz ) > ec_player.veloc_walk ) {
						veloc_xz = glm::normalize( veloc_xz ) *  ec_player.veloc_walk;
						ec_state.veloc.x = veloc_xz.x;
						ec_state.veloc.z = veloc_xz.y;
					}
				}

				ec_state.accel += accel_input;

				if( ec_state.is_coll_face[ FaceDirection::FD_Down ] ) {
					float frict = 0.2f;

					if( glm::length( ec_state.veloc ) > 0 &&
						ec_state.accel.x == 0 &&
						ec_state.accel.z == 0 ) {
						ec_state.veloc -= glm::normalize( ec_state.veloc ) * frict;

						if( ec_state.veloc.x <= frict && ec_state.veloc.x >= -frict ) ec_state.veloc.x = 0;
						if( ec_state.veloc.z <= frict && ec_state.veloc.z >= -frict ) ec_state.veloc.z = 0;
					}
				}
			}
			else { 
				if( !client.input_mgr.is_cursor_vis( ) ) {
					ec_state.rot.x -= client.input_mgr.get_mouse_delta( ).y * mouse_sensitivity;
					ec_state.rot.y += client.input_mgr.get_mouse_delta( ).x * mouse_sensitivity;

					while( ec_state.rot.x >= 360 ) ec_state.rot.x -= 360;
					while( ec_state.rot.x < 0 ) ec_state.rot.x += 360;
					if( ec_state.rot.x > 180 && ec_state.rot.x < 270 ) ec_state.rot.x = 270;
					if( ec_state.rot.x < 180 && ec_state.rot.x > 90 ) ec_state.rot.x = 90;
				}

				glm::vec3 accel_input = { 0, 0, 0 };

				if( client.input_mgr.is_key( 'W' ) ||
					client.input_mgr.is_key( VK_UP ) ) {
					accel_input += Directional::get_fwd( ec_state.rot );
				}
				else if( client.input_mgr.is_key( 'S' ) ||
					client.input_mgr.is_key( VK_DOWN ) ) {
					accel_input -= Directional::get_fwd( ec_state.rot );
				}

				if( client.input_mgr.is_key( 'A' ) ||
					client.input_mgr.is_key( VK_LEFT ) ) {
					accel_input += Directional::get_left( ec_state.rot );
				}
				else if( client.input_mgr.is_key( 'D' ) ||
					client.input_mgr.is_key( VK_RIGHT ) ) {
					accel_input -= Directional::get_left( ec_state.rot );
				}

				if( client.input_mgr.is_key( 'Q' ) ) {
					accel_input -= Directional::get_up_aa( ec_state.rot );
				}
				else if( client.input_mgr.is_key( 'E' ) ) {
					accel_input += Directional::get_up_aa( ec_state.rot );
				}

				if( accel_input != glm::vec3 { 0, 0, 0 } )
					accel_input = glm::normalize( accel_input );

				accel_input *= ec_player.accel_run;

				ec_state.accel += accel_input;

				float frict = 1.0f;
				float veloc = glm::length( ec_state.veloc );

				if( veloc > 0 ) {
					if( veloc > 128.0f ) { 
						ec_state.veloc = glm::normalize( ec_state.veloc ) * 128.0f;
					}

					if( ec_state.accel.x == 0 ) {
						if( ec_state.veloc.x > 0 ) {
							ec_state.veloc.x -= frict;
						}
						else if( ec_state.veloc.x < 0 ) {
							ec_state.veloc.x += frict;
						}

						if( ec_state.veloc.x <= frict && ec_state.veloc.x >= -frict ) ec_state.veloc.x = 0;
					}

					if( ec_state.accel.y == 0 ) {
						if( ec_state.veloc.y > 0 ) {
							ec_state.veloc.y -= frict;
						}
						else if( ec_state.veloc.y < 0 ) {
							ec_state.veloc.y += frict;
						}

						if( ec_state.veloc.y <= frict && ec_state.veloc.y >= -frict ) ec_state.veloc.y = 0;
					}

					if( ec_state.accel.z == 0 ) {
						if( ec_state.veloc.z > 0 ) {
							ec_state.veloc.z -= frict;
						}
						else if( ec_state.veloc.z < 0 ) {
							ec_state.veloc.z += frict;
						}
						if( ec_state.veloc.z <= frict && ec_state.veloc.z >= -frict ) ec_state.veloc.z = 0;
					}
				}
			}

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			return ErrorEntity::EE_Ok;
		}
	}
{}

Player::~Player( ) { }