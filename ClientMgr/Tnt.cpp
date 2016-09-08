#include "Tnt.h"

#include "Client.h"

Tnt::Tnt( ) :
	EntityLoader { 
		"Tnt",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECTnt >( client ) ) {
				return ErrorEntity::EE_Failed;
			}

			auto & ec_state = entity.h_state.get( );
			ec_state.is_gravity = true;

			auto & ec_tnt = entity.get_data< ECTnt >( ).get( );
			ec_tnt.time_last = client.time_mgr.get_time( TimeStrings::GAME );
			ec_tnt.time_life = rand( ) % 2500 + 2500;
			ec_tnt.time_update = 64;
			ec_tnt.size_explosion = 7;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECTnt >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_tnt = entity.get_data< ECTnt >( ).get( );
			int time_now = client.time_mgr.get_time( TimeStrings::GAME );

			if( time_now - ec_tnt.time_last >= ec_tnt.time_update ) {
				float c_val = glm::sin( ( ( time_now % 1000 ) / 1000.0f ) * 360 * 2 * PI / 180 ) * 0.5f + 0.5f;
				entity.color = glm::vec4( c_val, c_val, c_val, 1.0f );
				entity.is_dirty = true;
				ec_tnt.time_last += ec_tnt.time_update;
			}

			if( time_now - entity.time_live > ec_tnt.time_life ) {
				client.thread_mgr.task_main( 10,[ &, pos = ec_state.pos, size = ec_tnt.size_explosion ]( ) {
					client.chunk_mgr.explode_sphere( pos, size );
				} );

				entity.is_shutdown = true;
			}

			client.entity_mgr.entity_stop( ec_state );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			return ErrorEntity::EE_Ok;
		}
	} { }


Tnt::~Tnt( ) { }
