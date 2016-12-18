#include "WaterBlock.h"
#include "Client.h"

WaterBlock::WaterBlock( ) : EntityLoader {
	"Water Block",
	[ ] ( Client & client, Entity & entity ) {
		if( !entity.add_data< ECWater >( client ) ) {
			return ErrorEntity::EE_Failed;
		}

		auto & ec_state = entity.h_state.get( );
		ec_state.is_gravity = false;

		auto & ec_water = entity.get_data< ECWater >( ).get( );
		ec_water.time_now = client.time_mgr.get_time( TimeStrings::GAME );
		ec_water.time_update = ec_water.time_now;
		ec_water.time_static = ec_water.time_now;

		ec_water.cld_update = 50;
		ec_water.depth_flow = 8;

		ec_water.cld_static = 250;

		entity.id = client.block_mgr.get_block_loader( "Water" )->id;

		ec_water.map_grow.reserve( 64 );
		//ec_water.list_static.reserve( 64 );

		ec_water.is_start = false;

		return ErrorEntity::EE_Ok;
	},
	[ ] ( Client & client, Entity & entity ) {

		entity.clear_data< ECWater >( );

		return ErrorEntity::EE_Ok;
	},
	[ ] ( Client & client, Entity & entity ) {
		auto & ec_state = entity.h_state.get( );
		auto & ec_water = entity.get_data< ECWater >( ).get( );
		auto block_water = client.block_mgr.get_block_loader( "Water" );
		auto block_grass_blade = client.block_mgr.get_block_loader( "Grass Blade" );

		ec_water.time_now = client.time_mgr.get_time( TimeStrings::GAME );

		if( !ec_water.is_start ) {
			Directional::pos_trim( ec_state.pos, ec_water.pos_curr );
			client.chunk_mgr.set_block( ec_water.pos_curr, block_water->id );
			ec_water.map_grow.insert( { ec_water.pos_curr, ec_water.depth_flow } );
			ec_water.is_start = true;
		}

		if( client.chunk_mgr.get_block( ec_state.pos ) == block_water->id ) {
			if( ec_water.time_now - ec_water.time_update > ec_water.cld_update ) {
				auto iter_grow = ec_water.map_grow.begin( );

				while( iter_grow != ec_water.map_grow.end( ) ) { 
					// Down
					ec_water.pos_check = iter_grow->first + Directional::get_vec_dir_i( FaceDirection::FD_Down );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( iter_grow->second == 1 ) { 
						if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
							client.chunk_mgr.set_block( ec_water.pos_check, block_water->id );
							ec_water.list_add.push_back( { ec_water.pos_check, ec_water.depth_flow } );
						}

						ec_water.list_static.push_back( *iter_grow );
						iter_grow = ec_water.map_grow.erase( iter_grow );
						continue;
					}

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						client.chunk_mgr.set_block( ec_water.pos_check, block_water->id );
						ec_water.list_add.push_back( { ec_water.pos_check, ec_water.depth_flow } );

						ec_water.list_static.push_back( *iter_grow );
						iter_grow = ec_water.map_grow.erase( iter_grow );
						continue;
					}
					else if( ec_water.id_check == block_water->id ) { 
						ec_water.list_static.push_back( *iter_grow );
						iter_grow = ec_water.map_grow.erase( iter_grow );
						continue;
					}

					bool is_grow = false;

					// Front
					ec_water.pos_check = iter_grow->first + Directional::get_vec_dir_i( FaceDirection::FD_Front );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						client.chunk_mgr.set_block( ec_water.pos_check, block_water->id );
						ec_water.list_add.push_back( { ec_water.pos_check, iter_grow->second - 1 } );

						is_grow = true;
					}

					// Back
					ec_water.pos_check = iter_grow->first + Directional::get_vec_dir_i( FaceDirection::FD_Back );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						client.chunk_mgr.set_block( ec_water.pos_check, block_water->id );
						ec_water.list_add.push_back( { ec_water.pos_check, iter_grow->second - 1 } );

						is_grow = true;
					}

					// Left
					ec_water.pos_check = iter_grow->first + Directional::get_vec_dir_i( FaceDirection::FD_Left );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						client.chunk_mgr.set_block( ec_water.pos_check, block_water->id );
						ec_water.list_add.push_back( { ec_water.pos_check, iter_grow->second - 1 } );

						is_grow = true;
					}

					// Right
					ec_water.pos_check = iter_grow->first + Directional::get_vec_dir_i( FaceDirection::FD_Right );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						client.chunk_mgr.set_block( ec_water.pos_check, block_water->id );
						ec_water.list_add.push_back( { ec_water.pos_check, iter_grow->second - 1 } );

						is_grow = true;
					}

					if( !is_grow ) { 
						ec_water.list_static.push_back( *iter_grow );
						iter_grow = ec_water.map_grow.erase( iter_grow );
						continue;
					}

					iter_grow++;
				}

				for( auto & pair_add : ec_water.list_add ) { 
					ec_water.map_grow.insert( pair_add );
				}

				ec_water.list_add.clear( );

				ec_water.time_update += ec_water.cld_update;
			}

			
			if( ec_water.time_now - ec_water.time_static > ec_water.cld_static ) { 
				auto iter_static = ec_water.list_static.begin( );
				while( iter_static != ec_water.list_static.end( ) ) { 
					// Down
					ec_water.pos_check = iter_static->first + Directional::get_vec_dir_i( FaceDirection::FD_Down );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						ec_water.map_grow.insert( *iter_static );
						iter_static = ec_water.list_static.erase( iter_static );
						continue;
					}

					// Front
					ec_water.pos_check = iter_static->first + Directional::get_vec_dir_i( FaceDirection::FD_Front );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						ec_water.map_grow.insert( *iter_static );
						iter_static = ec_water.list_static.erase( iter_static );
						continue;
					}

					// Back
					ec_water.pos_check = iter_static->first + Directional::get_vec_dir_i( FaceDirection::FD_Back );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						ec_water.map_grow.insert( *iter_static );
						iter_static = ec_water.list_static.erase( iter_static );
						continue;
					}

					// Left
					ec_water.pos_check = iter_static->first + Directional::get_vec_dir_i( FaceDirection::FD_Left );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						ec_water.map_grow.insert( *iter_static );
						iter_static = ec_water.list_static.erase( iter_static );
						continue;
					}

					// Right
					ec_water.pos_check = iter_static->first + Directional::get_vec_dir_i( FaceDirection::FD_Right );
					ec_water.id_check = client.chunk_mgr.get_block( ec_water.pos_check );

					if( ec_water.id_check == -1 || ec_water.id_check == block_grass_blade->id ) {
						ec_water.map_grow.insert( *iter_static );
						iter_static = ec_water.list_static.erase( iter_static );
						continue;
					}

					iter_static++;
				}

				ec_water.time_static += ec_water.cld_static;
			}
		}
		else {
			if( ec_water.time_now - ec_water.time_update > ec_water.cld_update ) {


				ec_water.time_update += ec_water.cld_update;
			}

			for( auto & pair_grow : ec_water.map_grow ) { 
				client.chunk_mgr.set_block( pair_grow.first, -1 );
			}

			ec_water.map_grow.clear( );

			for( auto & pair_static : ec_water.list_static ) { 
				client.chunk_mgr.set_block( pair_static.first, -1 );
			}

			ec_water.list_static.clear( );

			if( ec_water.list_static.size( ) == 0 && ec_water.map_grow.size( ) == 0 ) {
				entity.is_shutdown = true;
			}
		}

		return ErrorEntity::EE_Ok;
	},
	[ ] ( Client & client, Entity & entity ) {

		return ErrorEntity::EE_Ok;
	}
} {}

WaterBlock::~WaterBlock( ) { }
