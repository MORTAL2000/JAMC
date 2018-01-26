#include "SpawnBlock.h"

#include "Client.h"

#include "GuiMgr.h"
#include "BlockMgr.h"
#include "DisplayMgr.h"
#include "EntityMgr.h"

SpawnBlock::SpawnBlock( ) :
	EntityLoader { 
		"Spawn Block",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECSpawnBlock >( client ) ) {
				return ErrorEntity::EE_Failed;
			}

			entity.id = client.gui_mgr.block_selector.get_id_block( );
			entity.color = client.block_mgr.get_block_loader( entity.id )->color;

			auto & ec_state = entity.h_state.get( );
			ec_state.pos = client.display_mgr.camera.pos_camera;
			ec_state.veloc = client.display_mgr.camera.vec_front * 50.0f;
			ec_state.rot = client.display_mgr.camera.rot_camera;

			auto & ec_spawn = entity.get< ECSpawnBlock >( ).get( );
			ec_spawn.time_life = 3000;
			ec_spawn.time_last = client.time_mgr.get_time( TimeStrings::GAME );
			ec_spawn.time_update = 100;
			ec_spawn.num_spawn = 20;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECSpawnBlock >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_spawn = entity.get< ECSpawnBlock >( ).get( );

			if( client.time_mgr.get_time( TimeStrings::GAME ) - ec_spawn.time_last > ec_spawn.time_update ) {
				client.thread_mgr.task_main( 10,[ &, pos = ec_state.pos, num_spawn = ec_spawn.num_spawn ]( ) {
					for( int i = 0; i < num_spawn; i++ ) {
						client.entity_mgr.entity_add( "Grav Block", [ pos = pos ] ( Client & client, Entity & entity ) {
							entity.id = std::rand( ) % client.block_mgr.get_num_blocks( );
							auto & ec_state = entity.h_state.get( );

							ec_state.pos = pos;
							ec_state.veloc =
								glm::normalize(
									Directional::get_fwd(
										glm::vec3(
											std::rand( ) % 360,
											std::rand( ) % 360,
											std::rand( ) % 360
										)
									)
								)
								* ( float ) ( std::rand( ) % 25 + 15 );
							entity.color = client.block_mgr.get_block_loader( entity.id )->color;

							return ErrorEntity::EE_Ok;
						} );
					}
				} );

				ec_spawn.time_last = client.time_mgr.get_time( TimeStrings::GAME );
			}

			if( client.time_mgr.get_time( TimeStrings::GAME ) - entity.time_live > ec_spawn.time_life ) {
				entity.is_shutdown = true;
			}

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			return ErrorEntity::EE_Ok;
		}
	} { }


SpawnBlock::~SpawnBlock( ) { }
