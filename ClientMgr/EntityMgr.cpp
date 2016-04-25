#include "EntityMgr.h"
#include "Client.h"

const char * ErrorEntityLookup::to_text[ ] = {
	"No entity error!",
	"Entity creation failed!",
	"EE_Size"
};

EntityMgr::EntityMgr( Client & client ) :
	Manager( client ) { }


EntityMgr::~EntityMgr( ) { }

void EntityMgr::entity_vbo( Entity & entity ) { 
	glGenVertexArrays( 1, &entity.id_vao );
	glGenBuffers( 1, &entity.id_vbo );

	glBindVertexArray( entity.id_vao );
	glBindBuffer( GL_ARRAY_BUFFER, entity.id_vbo );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glVertexPointer( 3, GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 0 ) );
	glColorPointer( 4, GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 12 ) );
	glNormalPointer( GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 28 ) );
	glTexCoordPointer( 2, GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 40 ) );

	//glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
}

static int num_entity = 5000;

void EntityMgr::init( ) { 
	client.resource_mgr.reg_pool< Entity >( num_entity );
	client.resource_mgr.reg_pool< ECState >( num_entity );
	client.resource_mgr.reg_pool< ECTnt >( num_entity );
	client.resource_mgr.reg_pool< ECGravBlock >( num_entity );
	list_entity.reserve( num_entity );
	map_entity.reserve( num_entity );

	client.resource_mgr.pool< Entity>( ).apply_func_all( [ & ] ( Entity & entity ) { 
		entity_vbo( entity );
	} );

	alloc_base = [ ] ( Client & client, Entity & entity ) {
		entity.time_live = client.time_mgr.get_time( TimeStrings::GAME );
		entity.is_shutdown = false;
		entity.color.set( 0, 0, 0, 0 );
		entity.is_dirty = true;

		if( !client.resource_mgr.allocate( entity.h_state ) ) { 
			return ErrorEntity::EE_Failed;
		}

		auto & ec_state = entity.h_state.get( );
		ec_state.pos = glm::vec3( 0, 0, 0 );
		ec_state.veloc = glm::vec3( 0, 0, 0 );
		ec_state.accel = glm::vec3( 0, 0, 0 );
		ec_state.is_coll = false;
		ec_state.id_block = -1;

		ec_state.rot = glm::vec3( 0, 0, 0 );
		ec_state.rot_veloc = glm::vec3( 0, 0, 0 );
		ec_state.rot_accel = glm::vec3( 0, 0, 0 );

		return ErrorEntity::EE_Ok;
	};

	custom_base = [ ] ( Client & client, Entity & entity ) { 
		return ErrorEntity::EE_Ok;
	};

	release_base = [ ] ( Client & client, Entity & entity ) { 
		entity.h_state.release( );

		return ErrorEntity::EE_Ok;
	};

	loader_add( 
		"Tnt",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECTnt >( client ) ) {
				return ErrorEntity::EE_Failed;
			}

			auto & ec_state = entity.h_state.get( );
			ec_state.is_gravity = true;

			auto & ec_tnt = entity.get_data< ECTnt >( ).get( );
			ec_tnt.time_last = client.time_mgr.get_time( TimeStrings::GAME );
			ec_tnt.time_life = rand( ) % 250 + 250;
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
				entity.color = Color4( c_val, c_val, c_val, 1.0f );
				entity.is_dirty = true;
				ec_tnt.time_last += ec_tnt.time_update;
			}

			if( time_now - entity.time_live > ec_tnt.time_life ) {
				client.thread_mgr.task_main( 10, [ &, pos = ec_state.pos, size = ec_tnt.size_explosion ] ( ) {
					client.chunk_mgr.explode_sphere( pos, size );
				} );

				entity.is_shutdown = true;
			}

			if( !ec_state.is_coll ) {
				return ErrorEntity::EE_Ok;
			}

			client.entity_mgr.entity_stop( ec_state );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) { 
			return ErrorEntity::EE_Ok;
		}
	);

	loader_add(
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

			auto & ec_block = entity.get_data< ECGravBlock >( ).get( );
			ec_block.time_life = 10000;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECGravBlock >( );
		
			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_block = entity.get_data< ECGravBlock >( ).get( );

			if( ec_state.is_coll ) {
				client.entity_mgr.entity_stop( ec_state );
				//client.chunk_mgr.set_block( ec_state.pos, ec_block.id_block );
				client.chunk_mgr.set_block( ec_state.pos, entity.id );
				entity.is_shutdown = true;
			}
			else if( ec_state.id_block == -2 ) {
				entity.is_shutdown = true;
			}
			else if( client.time_mgr.get_time( TimeStrings::GAME ) - entity.time_live > ec_block.time_life ) {
				entity.is_shutdown = true;
			}

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			return ErrorEntity::EE_Ok;
		}
	);

	loader_add( 
		"Line Block",
		[ ] ( Client & client, Entity & entity ) {
			entity.id = client.display_mgr.block_select.id_block;
			entity.color.set( 0.5f, 0.5f, 0.5f, 1.0f );

			auto & state = entity.h_state.get( );
			state.pos = client.display_mgr.camera.pos_camera;
			state.veloc = client.display_mgr.camera.vec_front * 50.0f;
			state.is_gravity = false;

			state.rot = client.display_mgr.camera.rot_camera;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );

			if( !ec_state.is_coll ) {
				if( ec_state.id_block == -2 ) {
					entity.is_shutdown = true;
				}

				return ErrorEntity::EE_Ok;
			}

			if( entity.id == client.chunk_mgr.get_block_data( "Tnt" ).id ) {
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
	);
}

static std::vector< ChunkFace > list_faces;

void EntityMgr::update( ) { 
	Entity * entity;
	
	std::lock_guard< std::mutex > lock( mtx_entity );

	auto iter_entity = list_entity.begin( );
	while( iter_entity != list_entity.end( ) ) {
		entity = &iter_entity->get( );

		if( entity->is_shutdown ) {
			entity_remove( *iter_entity );
			iter_entity = list_entity.erase( iter_entity );
			continue;
		}

		auto & state = entity->h_state.get( );

		entity_integrate( state );
		entity_terrain_collide( state );
		entity->loader->ef_update( client, *entity );

		entity_mesh( *entity );

		iter_entity++;
	}

	auto & out = client.display_mgr.out;
	out.str( "" );
	out << "[Entity Live] Entity: " << list_entity.size( );
	client.gui_mgr.print_to_static( out.str( ) );
}

void EntityMgr::render( ) { 
	Entity * entity;
	glm::vec3 * pos;
	glm::vec3 * rot;

	client.texture_mgr.unuse_prog( );

	auto iter = list_entity.begin( );
	while( iter != list_entity.end( ) ) {
		entity = &iter->get( );

		if( !entity->is_dirty ) {
			pos = &entity->h_state.get( ).pos;
			rot = &entity->h_state.get( ).rot;

			glPushMatrix( );

			glTranslatef( pos->x, pos->y, pos->z );
			glTranslatef( 0.5f, 0.5f, 0.5f );

			glRotatef( -rot->z, 0, 0, 1 );
			glRotatef( -rot->y, 0, 1, 0 );
			glRotatef( -rot->x, 1, 0, 0 );

			glTranslatef( -0.5f, -0.5f, -0.5f );

			glBindVertexArray( entity->id_vao );

			glDrawArrays( GL_TRIANGLES, 0, 6 * 6 );

			glBindVertexArray( 0 );

			glPopMatrix( );
		}

		iter++;
	}
}

void EntityMgr::end( ) { 

}

void EntityMgr::sec( ) { 

}

void EntityMgr::loader_add( std::string const & str_name, EFAlloc ef_alloc, 
	EFRelease ef_release, EFUpdate ef_update, EFMesh ef_mesh ) { 

	auto & out = client.display_mgr.out;
	out.str( "" );

	auto iter_map = map_loader.find( str_name );
	if( iter_map != map_loader.end( ) ) {
		out << "ERROR: Loader duplicate: " << str_name;
		client.gui_mgr.print_to_console( out.str( ) );
		return;
	}

	list_loader.emplace_back( 
		EntityLoader { 
			str_name,
			ef_alloc,
			ef_release,
			ef_update,
			ef_mesh
		} 
	);

	map_loader.emplace( 
		std::pair< std::string, int > {
			str_name,
			list_loader.size( ) - 1
		} 
	);

	out << "SUCCESS: Loader added: " << str_name;
	client.gui_mgr.print_to_console( out.str( ) );
}

void EntityMgr::entity_add( std::string const & str_name, EFCustom ef_custom ) {
	Handle< Entity > h_entity;
	std::lock_guard< std::mutex > lock( mtx_entity );

	auto iter_loader = map_loader.find( str_name );
	if( iter_loader == map_loader.end( ) ) { 
		client.thread_mgr.task_main( 5, [ &, str_name ] ( ) {
			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "No entity loader exists matching: " << str_name;
			client.gui_mgr.print_to_console( out.str( ) );
		} );
		return;
	}

	if( !client.resource_mgr.allocate( h_entity ) ) {
		client.thread_mgr.task_main( 5, [ & ] ( ) {
			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "ENTITY ERROR: Out of entities!";
			client.gui_mgr.print_to_console( out.str( ) );
		} );
		return;
	}

	auto & entity = h_entity.get( );
	entity.loader = &list_loader[ iter_loader->second ];

	int error = ErrorEntity::EE_Ok;

	if( ( error = alloc_base( client, entity ) ) != ErrorEntity::EE_Ok ||
		( error = entity.loader->ef_alloc( client, entity ) ) != ErrorEntity::EE_Ok ||
		( error = ef_custom( client, entity ) ) != ErrorEntity::EE_Ok ) {

		client.thread_mgr.task_main( 5, [ &, error ] ( ) { 
			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "ENTITY ERROR: " << ErrorEntityLookup::to_text[ error ];
			client.gui_mgr.print_to_console( out.str( ) );
		} );
	
		release_base( client, entity );
		entity.loader->ef_release( client, entity );
		h_entity.release( );

		return;
	}

	list_entity.push_back( h_entity );
}

void EntityMgr::entity_remove( Handle< Entity > & h_entity ) { 
	//std::lock_guard< std::mutex > lock( mtx_entity );
	auto & entity = h_entity.get( );

	entity.h_state.release( );
	release_base( client, entity );
	entity.loader->ef_release( client, entity );
	h_entity.release( );
}

static glm::vec3 vec_gravity( 0, -9.81, 0 );

void EntityMgr::entity_integrate( ECState & ec_state ) { 
	ec_state.accel = glm::vec3( 0, 0, 0 );
	if( ec_state.is_gravity ) ec_state.accel += vec_gravity;
	ec_state.veloc += ec_state.accel * DELTA_CORRECT;
	ec_state.pos += ( ec_state.veloc + ( ec_state.accel / 2.0f ) * DELTA_CORRECT ) * DELTA_CORRECT;

	ec_state.rot_accel = glm::vec3( 0, 0, 0 );
	ec_state.rot_veloc += ec_state.rot_accel * DELTA_CORRECT;
	ec_state.rot += ( ec_state.rot_veloc + ( ec_state.rot_accel / 2.0f ) * DELTA_CORRECT ) * DELTA_CORRECT;
}

void EntityMgr::entity_terrain_collide( ECState & ec_state ) { 
	ec_state.id_block = client.chunk_mgr.get_block( ec_state.pos );

	if( ec_state.id_block == -1 ||
		ec_state.id_block == -2 ) {
		ec_state.is_coll = false;
		return;
	}

	ec_state.is_coll = true;
}

void EntityMgr::entity_stop( ECState & ec_state ) {
	ec_state.pos -= ( ec_state.veloc + ( ec_state.accel / 2.0f ) * DELTA_CORRECT ) * DELTA_CORRECT;
	ec_state.veloc = glm::vec3( 0, 0, 0 );
}

void EntityMgr::entity_mesh( Entity & entity ) { 
	if( entity.is_dirty ) {
		FaceVerts const * verts;
		Color4 * color;
		glm::vec3 const * normal;
		FaceUvs const * uvs;
		Block & block = client.chunk_mgr.get_block_data( entity.id );

		list_faces.clear( );

		for( int i = 0; i < FaceDirection::FD_Size; i++ ) {
			verts = &Block::get_verts( ( FaceDirection ) i );
			color = &entity.color;
			normal = &Directional::get_vec_dir_f( ( FaceDirection ) i );
			uvs = &block.get_uvs( ( FaceDirection ) i );

			put_face( list_faces, glm::ivec3( 0, 0, 0 ), *verts, *color, *normal, *uvs );
		}

		glBindBuffer( GL_ARRAY_BUFFER, entity.id_vbo );
		glBufferData( GL_ARRAY_BUFFER, list_faces.size( ) * sizeof( ChunkFace ), nullptr, GL_STATIC_DRAW );
		glBufferData( GL_ARRAY_BUFFER, list_faces.size( ) * sizeof( ChunkFace ), list_faces.data( ), GL_STATIC_DRAW );
		entity.is_dirty = false;
	}
}