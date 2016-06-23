#include "EntityMgr.h"
#include "Client.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

const char * ErrorEntityLookup::to_text[ ] = {
	"No entity error!",
	"Entity creation failed!",
	"EE_Size"
};

EntityMgr::EntityMgr( Client & client ) :
	Manager( client ) { }


EntityMgr::~EntityMgr( ) { }

const ChunkFaceIndices indicies[ 6 ] { 
	{ 0, 1, 2,		2, 3, 0 },
	{ 4, 5, 6,		6, 7, 4 },
	{ 8, 9, 10,		10, 11, 8 },
	{ 12, 13, 14,	14, 15, 12 },
	{ 16, 17, 18,	18, 19, 16 },
	{ 20, 21, 22,	22, 23, 20 }
};

static int num_entity = 10000;
static std::vector< ChunkFaceVertices > list_faces;

void EntityMgr::init( ) { 
	client.resource_mgr.reg_pool< Entity >( num_entity );
	client.resource_mgr.reg_pool< ECState >( num_entity );
	client.resource_mgr.reg_pool< ECTnt >( num_entity );
	client.resource_mgr.reg_pool< ECGravBlock >( num_entity );
	client.resource_mgr.reg_pool< ECSpawnBlock >( num_entity );

	list_entity.reserve( num_entity );
	map_entity.reserve( num_entity );

	vbo.init( );

	for( int i = 0; i < FaceDirection::FD_Size; i++ ) {
		put_face( 
			list_faces, 
			glm::ivec3( 0, 0, 0 ), 
			Block::get_verts( ( FaceDirection ) i ), 
			Color4( 1.0f, 1.0f, 1.0f, 1.0f ),
			Directional::get_vec_dir_f( ( FaceDirection ) i ),
			client.chunk_mgr.get_block_data( 0 ).get_uvs( ( FaceDirection ) i ) 
		);
	}

	vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
		"Entity",
		client.texture_mgr.id_terrain,
		std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 } 
	) );

	for( int i = 0; i < FaceDirection::FD_Size; ++i ) {
		auto & verts = Block::get_verts( ( FaceDirection ) i );
		auto color = Color4( 1.0f, 1.0f, 1.0f, 1.0f );
		auto & norm = Directional::get_vec_dir_f( ( FaceDirection ) i );
		auto & uvs = client.chunk_mgr.get_block_data( 0 ).get_uvs( ( FaceDirection ) i );

		for( int j = 0; j < 4; ++j ) { 
			vbo.push_data( VBO::Vertex { 
				verts[ j ][ 0 ] - 0.5f, verts[ j ][ 1 ] - 0.5f, verts[ j ][ 2 ] - 0.5f,
				color.r, color.g, color.b, color.a,
				norm[ 0 ], norm[ 1 ], norm[ 2 ],
				uvs[ j ][ 0 ], uvs[ j ][ 1 ], 0 
			} );
		}
	}

	vbo.finalize_set( );
	vbo.buffer( );

	alloc_base = [ ] ( Client & client, Entity & entity ) {
		entity.time_live = client.time_mgr.get_time( TimeStrings::GAME );
		entity.is_shutdown = false;
		entity.color.set( 1.0f, 1.0f, 1.0f, 1.0f );
		entity.is_dirty = true;

		if( !client.resource_mgr.allocate( entity.h_state ) ) { 
			return ErrorEntity::EE_Failed;
		}

		auto & ec_state = entity.h_state.get( );
		ec_state.pos = glm::vec3( 0, 0, 0 );
		ec_state.veloc = glm::vec3( 0, 0, 0 );
		ec_state.accel = glm::vec3( 0, 0, 0 );
		ec_state.is_coll = false;
		fill_array( ec_state.is_coll_face, false );
		ec_state.id_block = -1;

		ec_state.rot = glm::vec3( 0, 0, 0 );
		ec_state.rot_veloc = glm::vec3( 0, 0, 0 );
		ec_state.rot_accel = glm::vec3( 0, 0, 0 );

		ec_state.dim = glm::vec3( 1.0f, 1.0f, 1.0f );

		return ErrorEntity::EE_Ok;
	};

	custom_base = [ ] ( Client & client, Entity & entity ) { 
		return ErrorEntity::EE_Ok;
	};

	release_base = [ ] ( Client & client, Entity & entity ) { 
		entity.h_state.release( );

		return ErrorEntity::EE_Ok;
	};

	loader_add( "Player",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECPlayer >( client ) ) { 
				return ErrorEntity::EE_Failed;
			}

			entity.id = 0;

			auto & ec_state = entity.h_state.get( );
			ec_state.is_gravity = true;
			ec_state.dim = { 1.0f, 2.6f, 1.0f };

			ec_state.pos = { 0.0f, Chunk::size_y / 2.0f + 5.0f, 0.0f };

			//ec_state.rot.y = 180;

			auto & ec_player = entity.get_data< ECPlayer >( ).get( );
			ec_player.veloc_vmax = 9.81f * 4.0f;
			ec_player.veloc_hmax = 32.0f;

			ec_player.veloc_walk = 6.0f;
			ec_player.veloc_run = 12.0f;
			ec_player.accel_walk = 1.0f;
			ec_player.accel_run = 1.0f;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			entity.clear_data< ECPlayer >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_player = entity.get_data< ECPlayer >( ).get( );

			if( client.input_mgr.is_key( VK_UP ) ) {
				ec_state.accel += Directional::get_fwd( ec_state.rot ) * ec_player.accel_walk;
			}
			else if( client.input_mgr.is_key( VK_DOWN ) ) {
				ec_state.accel -= Directional::get_fwd( ec_state.rot ) * ec_player.accel_walk;
			}

			if( client.input_mgr.is_key( VK_LEFT ) ) {
				ec_state.accel += Directional::get_left( ec_state.rot ) * ec_player.accel_walk;
			}
			else if( client.input_mgr.is_key( VK_RIGHT ) ) {
				ec_state.accel -= Directional::get_left( ec_state.rot ) * ec_player.accel_walk;
			}

			client.entity_mgr.entity_stop( ec_state );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			return ErrorEntity::EE_Ok;
		}
	);

	loader_add( "Tnt",
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

			client.entity_mgr.entity_stop( ec_state );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) { 
			return ErrorEntity::EE_Ok;
		}
	);

	loader_add( "Grav Block",
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
			ec_block.time_life = 20000;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECGravBlock >( );
		
			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_block = entity.get_data< ECGravBlock >( ).get( );
			float frict = 0.1f;
			client.entity_mgr.entity_stop( ec_state );

			if( ec_state.is_coll ) {
				ec_state.rot = { 0, 0, 0 };
				ec_state.rot_veloc = { 0, 0, 0 };

				if( ec_state.is_coll_face[ FaceDirection::FD_Down ] ) { 
					ec_state.veloc -= glm::normalize( ec_state.veloc ) * frict;

					if( ec_state.veloc.x <= frict ) ec_state.veloc.x = 0;
					if( ec_state.veloc.z <= frict ) ec_state.veloc.z = 0;

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
	);

	loader_add( "Line Block",
		[ ] ( Client & client, Entity & entity ) {
			entity.id = client.display_mgr.block_selector.get_id_block( );
			entity.color = client.chunk_mgr.get_block_data( entity.id ).color;

			auto & state = entity.h_state.get( );
			state.pos = client.display_mgr.camera.pos_camera;
			state.veloc = client.display_mgr.camera.vec_front * 50.0f;
			state.rot = client.display_mgr.camera.rot_camera;
			state.is_gravity = false;

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

	loader_add( "Spin Block",
		[ ] ( Client & client, Entity & entity ) {
			entity.id = client.display_mgr.block_selector.get_id_block( );
			entity.color = client.chunk_mgr.get_block_data( entity.id ).color;

			auto & state = entity.h_state.get( );
			state.pos = client.display_mgr.camera.pos_camera;
			state.rot_veloc.x = 25.0f;
			state.is_gravity = false;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );

			if( client.time_mgr.get_time( TimeStrings::GAME ) - entity.time_live > 10000 ) {
				entity.is_shutdown = true;
			}

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {

			return ErrorEntity::EE_Ok;
		}
	);

	loader_add( "Spawn Block",
		[ ] ( Client & client, Entity & entity ) {
			if( !entity.add_data< ECSpawnBlock >( client ) ) { 
				return ErrorEntity::EE_Failed;
			}

			entity.id = client.display_mgr.block_selector.get_id_block( );
			entity.color = client.chunk_mgr.get_block_data( entity.id ).color;

			auto & ec_state = entity.h_state.get( );
			ec_state.pos = client.display_mgr.camera.pos_camera;
			ec_state.veloc = client.display_mgr.camera.vec_front * 50.0f;
			ec_state.rot = client.display_mgr.camera.rot_camera;
			ec_state.is_gravity = false;

			auto & ec_spawn = entity.get_data< ECSpawnBlock >( ).get( );
			ec_spawn.time_life = 3000;
			ec_spawn.time_last = client.time_mgr.get_time( TimeStrings::GAME );
			ec_spawn.time_update = 100;
			ec_spawn.num_spawn = 200;

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			entity.clear_data< ECSpawnBlock >( );

			return ErrorEntity::EE_Ok;
		},
		[ ] ( Client & client, Entity & entity ) {
			auto & ec_state = entity.h_state.get( );
			auto & ec_spawn = entity.get_data< ECSpawnBlock >( ).get( );

			if( client.time_mgr.get_time( TimeStrings::GAME ) - ec_spawn.time_last > ec_spawn.time_update ) {
				client.thread_mgr.task_main( 10, [ &, pos = ec_state.pos, num_spawn = ec_spawn.num_spawn ]( ) {
					for( int i = 0; i < num_spawn; i++ ) {
						client.entity_mgr.entity_add( "Grav Block", [ pos = pos ] ( Client & client, Entity & entity ) {
							entity.id = std::rand( ) % client.chunk_mgr.get_num_blocks( );
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
							entity.color = client.chunk_mgr.get_block_data( entity.id ).color;

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
	);

	entity_add( "Player", [ ] ( Client & client, Entity & entity ) {

		return ErrorEntity::EE_Ok;
	} );
}

void EntityMgr::update( ) { 
	//Entity * entity;

	std::lock_guard< std::mutex > lock( mtx_entity );

	client.resource_mgr.pool< Entity >().apply_func_live_threads( client.thread_mgr,
		10, client.thread_mgr.cnt_thread_sync( ), [ & ] ( Entity & entity ) { 

		auto & state = entity.h_state.get( );

		entity.loader->ef_update( client, entity );
		entity_integrate( state );
		entity_terrain_collide( state );

		state.mat_model = glm::mat4( 1.0f );
		state.mat_model = glm::translate( state.mat_model, state.pos );
		state.mat_model = glm::rotate( state.mat_model, glm::radians( -state.rot.z ), glm::vec3( 0, 0, 1 ) );
		state.mat_model = glm::rotate( state.mat_model, glm::radians( -state.rot.y ), glm::vec3( 0, 1, 0 ) );
		state.mat_model = glm::rotate( state.mat_model, glm::radians( -state.rot.x ), glm::vec3( 1, 0, 0 ) );
		state.mat_model = glm::scale( state.mat_model, state.dim );
		state.mat_norm = glm::inverseTranspose( glm::mat3( state.mat_model ) );
	} );

	auto iter_entity = list_entity.begin( );
	while( iter_entity != list_entity.end( ) ) {
		if( iter_entity->get( ).is_shutdown ) {
			entity_remove( *iter_entity );
			iter_entity = list_entity.erase( iter_entity );
			continue;
		}
		iter_entity++;
	}

	auto & out = client.display_mgr.out;
	out.str( "" );
	out << "[Entity Live] Entity: " << list_entity.size( );
	client.gui_mgr.print_to_static( out.str( ) );
}

void EntityMgr::render( ) {
	Entity * entity;
	client.texture_mgr.bind_program( "Entity" );

	static GLuint idx_mat_model = glGetUniformLocation( client.texture_mgr.id_prog, "mat_model" );
	static GLuint idx_mat_norm = glGetUniformLocation( client.texture_mgr.id_prog, "mat_norm" );
	static GLuint idx_idx_layer = glGetUniformLocation( client.texture_mgr.id_prog, "idx_layer" );
	static GLuint idx_frag_color = glGetUniformLocation( client.texture_mgr.id_prog, "entity_color" );

	auto iter = list_entity.begin( );
	while( iter != list_entity.end( ) ) {
		entity = &iter->get( );

		glUniformMatrix4fv( idx_mat_model, 1, GL_FALSE, glm::value_ptr( entity->h_state.get( ).mat_model ) );
		glUniformMatrix3fv( idx_mat_norm, 1, GL_FALSE, glm::value_ptr( entity->h_state.get( ).mat_norm ) );
		glUniform1f( idx_idx_layer, client.chunk_mgr.get_block_data( entity->id ).get_uvs( FaceDirection::FD_Front )[ 0 ][ 2 ] );
		glUniform4fv( idx_frag_color, 1, ( const GLfloat * ) &entity->color );

		vbo.render( client );

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
	ec_state.pos_last = ec_state.pos;

	if( ec_state.is_gravity ) ec_state.accel += vec_gravity;
	ec_state.veloc += ec_state.accel * DELTA_CORRECT;
	ec_state.pos_delta = ( ec_state.veloc + ( ec_state.accel / 2.0f ) * DELTA_CORRECT ) * DELTA_CORRECT;
	ec_state.pos += ec_state.pos_delta;

	ec_state.rot_veloc += ec_state.rot_accel * DELTA_CORRECT;
	ec_state.rot += ( ec_state.rot_veloc + ( ec_state.rot_accel / 2.0f ) * DELTA_CORRECT ) * DELTA_CORRECT;

	ec_state.rot_accel = glm::vec3( 0, 0, 0 );
	ec_state.accel = glm::vec3( 0, 0, 0 );
}

void EntityMgr::entity_terrain_collide( ECState & ec_state ) {
	ec_state.is_coll = false;
	fill_array( ec_state.is_coll_face, false );

	if( ec_state.pos_delta.z < 0 ) entity_terrain_collide_f( ec_state );
	else if( ec_state.pos_delta.z > 0 ) entity_terrain_collide_b( ec_state );

	if( ec_state.pos_delta.x < 0 ) entity_terrain_collide_l( ec_state );
	else if( ec_state.pos_delta.x > 0 ) entity_terrain_collide_r( ec_state );

	if( ec_state.pos_delta.y > 0 ) entity_terrain_collide_u( ec_state );
	else if( ec_state.pos_delta.y < 0 ) entity_terrain_collide_d( ec_state );
}

void EntityMgr::entity_terrain_collide_f( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_x; int num_y;
	float step_x; float step_y;
	int id_block;

	pos_coll_s = ec_state.pos_last - ec_state.dim / 2.0f;
	pos_coll_s.z += ec_state.pos_delta.z;

	num_x = std::ceil( ec_state.dim.x );
	num_y = std::ceil( ec_state.dim.y );

	step_x = ec_state.dim.x / num_x;
	step_y = ec_state.dim.y / num_y;

	for( int i = 0; i <= num_x; ++i ) {
		for( int j = 0; j <= num_y; ++j ) {
			id_block = client.chunk_mgr.get_block(
				pos_coll_s + glm::vec3( step_x * i, step_y * j, 0 ) );
			ec_state.is_coll_face[ FaceDirection::FD_Front ] =
				id_block != -1 && id_block != -2;

			if( ec_state.is_coll_face[ FaceDirection::FD_Front ] ) {
				ec_state.is_coll = true;
				return;
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_b( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_x; int num_y;
	float step_x; float step_y;
	int id_block;

	pos_coll_s = ec_state.pos_last - ec_state.dim / 2.0f;
	pos_coll_s.z += ec_state.dim.z;
	pos_coll_s.z += ec_state.pos_delta.z;

	num_x = std::ceil( ec_state.dim.x );
	num_y = std::ceil( ec_state.dim.y );

	step_x = ec_state.dim.x / num_x;
	step_y = ec_state.dim.y / num_y;

	for( int i = 0; i <= num_x; ++i ) {
		for( int j = 0; j <= num_y; ++j ) {
			id_block = client.chunk_mgr.get_block(
				pos_coll_s + glm::vec3( step_x * i, step_y * j, 0 ) );
			ec_state.is_coll_face[ FaceDirection::FD_Back ] =
				id_block != -1 && id_block != -2;

			if( ec_state.is_coll_face[ FaceDirection::FD_Back ] ) {
				ec_state.is_coll = true;
				return;
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_l( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_z; int num_y;
	float step_z; float step_y;
	int id_block;

	pos_coll_s = ec_state.pos_last - ec_state.dim / 2.0f;
	pos_coll_s.x += ec_state.pos_delta.x;

	num_z = std::ceil( ec_state.dim.z );
	num_y = std::ceil( ec_state.dim.y );

	step_z = ec_state.dim.z / num_z;
	step_y = ec_state.dim.y / num_y;

	for( int i = 0; i <= num_z; ++i ) {
		for( int j = 0; j <= num_y; ++j ) {
			id_block = client.chunk_mgr.get_block(
				pos_coll_s + glm::vec3( 0, step_y * j, step_z * i ) );
			ec_state.is_coll_face[ FaceDirection::FD_Left ] =
				id_block != -1 && id_block != -2;

			if( ec_state.is_coll_face[ FaceDirection::FD_Left ] ) {
				ec_state.is_coll = true;
				return;
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_r( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_z; int num_y;
	float step_z; float step_y;
	int id_block;

	pos_coll_s = ec_state.pos_last - ec_state.dim / 2.0f;
	pos_coll_s.x += ec_state.dim.x;
	pos_coll_s.x += ec_state.pos_delta.x;

	num_z = std::ceil( ec_state.dim.z );
	num_y = std::ceil( ec_state.dim.y );

	step_z = ec_state.dim.z / num_z;
	step_y = ec_state.dim.y / num_y;

	for( int i = 0; i <= num_z; ++i ) {
		for( int j = 0; j <= num_y; ++j ) {
			id_block = client.chunk_mgr.get_block(
				pos_coll_s + glm::vec3( 0, step_y * j, step_z * i ) );
			ec_state.is_coll_face[ FaceDirection::FD_Right ] =
				id_block != -1 && id_block != -2;

			if( ec_state.is_coll_face[ FaceDirection::FD_Right ] ) {
				ec_state.is_coll = true;
				return;
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_u( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_x; int num_z;
	float step_x; float step_z;
	int id_block;

	pos_coll_s = ec_state.pos_last - ec_state.dim / 2.0f;
	pos_coll_s.y += ec_state.dim.y;
	pos_coll_s.y += ec_state.pos_delta.y;

	num_x = std::ceil( ec_state.dim.x );
	num_z = std::ceil( ec_state.dim.z );

	step_x = ec_state.dim.x / num_x;
	step_z = ec_state.dim.z / num_z;

	for( int i = 0; i <= num_x; ++i ) {
		for( int j = 0; j <= num_z; ++j ) {
			id_block = client.chunk_mgr.get_block(
				pos_coll_s + glm::vec3( step_x * i, 0, step_z * j ) );
			ec_state.is_coll_face[ FaceDirection::FD_Up ] =
				id_block != -1 && id_block != -2;

			if( ec_state.is_coll_face[ FaceDirection::FD_Up ] ) {
				ec_state.is_coll = true;
				return;
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_d( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_x; int num_z;
	float step_x; float step_z;
	int id_block;

	pos_coll_s = ec_state.pos_last - ec_state.dim / 2.0f;
	pos_coll_s.y += ec_state.pos_delta.y;

	num_x = std::ceil( ec_state.dim.x );
	num_z = std::ceil( ec_state.dim.z );

	step_x = ec_state.dim.x / num_x;
	step_z = ec_state.dim.z / num_z;

	for( int i = 0; i <= num_x; ++i ) {
		for( int j = 0; j <= num_z; ++j ) {
			id_block = client.chunk_mgr.get_block( 
				pos_coll_s + glm::vec3( step_x * i, 0, step_z * j ) );
			ec_state.is_coll_face[ FaceDirection::FD_Down ] =
				id_block != -1 && id_block != -2;

			if( ec_state.is_coll_face[ FaceDirection::FD_Down ] ) {
				ec_state.is_coll = true;
				return;
			}
		}
	}
}

void EntityMgr::entity_stop( ECState & ec_state ) {
	if( !ec_state.is_coll ) {
		return;
	}

	if( ec_state.is_coll_face[ FaceDirection::FD_Front ] ||
		ec_state.is_coll_face[ FaceDirection::FD_Back ] ) {

		ec_state.pos.z -= ec_state.pos_delta.z;
		ec_state.veloc.z = 0;
	}

	if( ec_state.is_coll_face[ FaceDirection::FD_Up ] ||
		ec_state.is_coll_face[ FaceDirection::FD_Down ] ) {

		ec_state.pos.y -= ec_state.pos_delta.y;
		ec_state.veloc.y = 0;
	}

	if( ec_state.is_coll_face[ FaceDirection::FD_Left ] ||
		ec_state.is_coll_face[ FaceDirection::FD_Right ] ) {

		ec_state.pos.x -= ec_state.pos_delta.x;
		ec_state.veloc.x = 0;
	}
}

void EntityMgr::entity_mesh( Entity & entity ) { 

}