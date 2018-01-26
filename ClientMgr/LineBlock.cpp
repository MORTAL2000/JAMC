#include "LineBlock.h"

#include "Client.h"

#include "GuiMgr.h"
#include "DisplayMgr.h"
#include "BlockMgr.h"
#include "EntityMgr.h"

LineBlock::LineBlock( ) :
	EntityLoader { 
		"Line Block",
		[ ] ( Client & client, Entity & entity ) {
			entity.id = client.gui_mgr.block_selector.get_id_block( );
			entity.color = client.block_mgr.get_block_loader( entity.id )->color;

			auto & state = entity.h_state.get( );
			state.pos = client.display_mgr.camera.pos_camera;
			state.veloc = client.display_mgr.camera.vec_front * 50.0f;
			state.rot = client.display_mgr.camera.rot_camera;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );

			if( !ec_state.is_coll ) {
				return ErrorEntity::EE_Ok;
			}

			if( ec_state.id_block == -2 ) {
				entity.is_shutdown = true;
				return ErrorEntity::EE_Ok;
			}

			if( entity.id == client.block_mgr.get_block_loader( "Tnt" )->id ) {
				client.thread_mgr.task_main( 6, [ &, pos = ec_state.pos ]( ) {
					client.chunk_mgr.explode_sphere( pos, 7 );
				} );
			}
			else {
				client.entity_mgr.entity_stop( ec_state );
				client.chunk_mgr.set_block( ec_state.pos, entity.id );
			}

			entity.is_shutdown = true;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			return ErrorEntity::EE_Ok;
		}
	} { }


LineBlock::~LineBlock( ) { }
