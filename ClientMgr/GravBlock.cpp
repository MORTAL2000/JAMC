#include "GravBlock.h"

#include "Client.h"

#include "TimeMgr.h"
#include "ChunkMgr.h"
#include "EntityMgr.h"

#include "Directional.h"

GravBlock::GravBlock( ) :
	EntityLoader { 
		"Grav Block",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECGravBlock >( client ) ) {
				return ErrorEntity::EE_Failed;
			}

			auto & ec_state = entity.h_state.get( );
			ec_state.gravity = { 0.0f, -9.81f, 0.0f };
			ec_state.rot_veloc = glm::vec3(
				( rand( ) % 10000 - 5000 ) / 10.0f,
				0,
				( rand( ) % 10000 - 5000 ) / 10.0f );
			ec_state.dim = { 1.0f, 1.0f, 1.0f };

			auto & ec_block = entity.get< ECGravBlock >( ).get( );
			ec_block.time_life = std::rand( ) % 15000 + 15000;
			//ec_block.state = ECGravBlock::SB_Active;
			//ec_block.time_snap_length = 1000;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECGravBlock >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_block = entity.get< ECGravBlock >( ).get( );

			//switch( ec_block.state ) {
				//case( ECGravBlock::SB_Active ): {


			float frict = 0.5f;

			if( ec_state.is_coll ) {
				client.entity_mgr.entity_stop( ec_state );

				ec_state.rot = { 0, 0, 0 };
				ec_state.rot_veloc = { 0, 0, 0 };

				if( ec_state.is_coll_face[ FaceDirection::FD_Down ] ) {
					if( glm::length( ec_state.veloc ) > 0 )
						ec_state.veloc -= glm::normalize( ec_state.veloc ) * frict;

					if( ec_state.veloc.x <= frict && ec_state.veloc.x >= -frict ) ec_state.veloc.x = 0;
					if( ec_state.veloc.z <= frict && ec_state.veloc.z >= -frict ) ec_state.veloc.z = 0;

					if( ec_state.veloc.x == 0 && ec_state.veloc.z == 0 ) {
						//ec_block.time_snap_start = client.time_mgr.get_time( TimeStrings::GAME );
						//ec_block.time_snap_end = ec_block.time_snap_start + ec_block.time_snap_length;
						//ec_block.state = ECGravBlock::SB_Snap;

						//ec_block.pos_snap_start = ec_state.pos;
						//ec_block.pos_snap_end = glm::floor( ec_block.pos_snap_start ) + glm::vec3( 0.5f );
								
						client.chunk_mgr.set_block( ec_state.pos, entity.id );
						entity.is_shutdown = true;
					}
				}
			}

				//} break;
/*
				case( ECGravBlock::SB_Snap ):
				{
					float game_time = client.time_mgr.get_time( TimeStrings::GAME );

					if( game_time > ec_block.time_snap_end ) {
						client.chunk_mgr.set_block( ec_state.pos, entity.id );
						entity.is_shutdown = true;
						break;
					}

					ec_state.pos = ec_block.pos_snap_start +
						( ec_block.pos_snap_end - ec_block.pos_snap_start ) *
						( ( game_time - ec_block.time_snap_start ) / ec_block.time_snap_length );
				} break;
			};*/

			if( client.time_mgr.get_time( TimeStrings::GAME ) - entity.time_live > ec_block.time_life ) {
				entity.is_shutdown = true;
			}

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			return ErrorEntity::EE_Ok;
		}
	} { }


GravBlock::~GravBlock( ) { }
