#include "GravBlock.h"

#include "Client.h"

GravBlock::GravBlock( ) :
	EntityLoader { 
		"Grav Block",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECGravBlock >( client ) ) {
				return ErrorEntity::EE_Failed;
			}

			auto & ec_state = entity.h_state.get( );
			ec_state.is_gravity = true;
			ec_state.rot_veloc = glm::vec3(
				( rand( ) % 10000 - 5000 ) / 10.0f,
				0,
				( rand( ) % 10000 - 5000 ) / 10.0f );
			ec_state.dim = { 1.0f, 1.0f, 1.0f };

			auto & ec_block = entity.get_data< ECGravBlock >( ).get( );
			ec_block.time_life = std::rand( ) % 5000 + 5000;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECGravBlock >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_block = entity.get_data< ECGravBlock >( ).get( );
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
						client.chunk_mgr.set_block( ec_state.pos, entity.id );
						entity.is_shutdown = true;
					}
				}
			}

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
