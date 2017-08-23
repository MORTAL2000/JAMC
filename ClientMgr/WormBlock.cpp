#include "WormBlock.h"

#include "Client.h"
#include "simplexnoise.h"

WormBlock::WormBlock( )	:
	EntityLoader {
		"WormBlock",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECWorm >( client ) ) {
				return ErrorEntity::EE_Failed;
			}

			entity.id = client.block_mgr.get_block_loader( "Gold Block" )->id;

			auto & ec_state = entity.h_state.get( );
			ec_state.is_gravity = false;

			auto & ec_worm = entity.get< ECWorm >( ).get( );
			ec_worm.num_visited = 0;
			ec_worm.cnt_step = 0;

			ec_worm.scale_x = 0.1f;
			ec_worm.scale_y = 0.1f;

			ec_worm.seed_x = rand( ) % 1000 / 10.0f;
			ec_worm.seed_y = rand( ) % 1000 / 10.0f;

			ec_worm.max_x = 15;
			ec_worm.max_y = 15;

			ec_worm.heading = { rand() % 360, rand() % 360, 0 };

			ec_worm.time_life = 10000;

			ec_worm.speed = 1.0f;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECWorm >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_worm = entity.get< ECWorm >( ).get( );
			int time_now = client.time_mgr.get_time( TimeStrings::GAME );

			ec_worm.cnt_step++;
			ec_worm.heading.x += raw_noise_2d( ec_worm.seed_x, ec_worm.cnt_step * ec_worm.scale_x ) * ec_worm.max_x;
			ec_worm.heading.y += raw_noise_2d( ec_worm.seed_y, ec_worm.cnt_step * ec_worm.scale_y ) * ec_worm.max_y;

			ec_state.pos += Directional::get_fwd( ec_worm.heading ) * ec_worm.speed;

			Directional::pos_trim( ec_state.pos, ec_worm.pos_curr );

			if( ec_worm.pos_curr != ec_worm.pos_last ) { 
				client.chunk_mgr.set_sphere( ec_state.pos, 5, -1 );

				ec_worm.pos_last = ec_worm.pos_curr;
			}

			if( time_now - entity.time_live > ec_worm.time_life ) {
				entity.is_shutdown = true;
			}

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			return ErrorEntity::EE_Ok;
		} 
	} { }


WormBlock::~WormBlock( ) { }