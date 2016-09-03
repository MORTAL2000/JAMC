#include "EntityMgr.h"
#include "Client.h"
#include "Player.h"
#include "Tnt.h"
#include "GravBlock.h"
#include "LineBlock.h"
#include "SpawnBlock.h"
#include "WormBlock.h"
#include "WaterBlock.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

const char * ErrorEntityLookup::to_text[ ] = {
	"No entity error!",
	"Entity creation failed!",
	"EE_Size"
};

glm::vec3 verts_entity[ FaceDirection::FD_Size ][ 4 ] = {
	{ { -0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f } },
	{ { 0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f } },
	{ { 0.5f, -0.5f, 0.5f }, { 0.5f, -0.5f, -0.5f  }, { 0.5f, 0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } },
	{ { -0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f, -0.5f } },
	{ { 0.5f, 0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f }, { -0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f } },
	{ { -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, 0.5f }, { -0.5f, -0.5f, 0.5f } }
};

glm::vec2 uvs_entity[ 4 ] = { 
	{ 0, 0 },
	{ 1, 0 },
	{ 1, 1 },
	{ 0, 1 }
};

EntityMgr::EntityMgr( Client & client ) :
	Manager( client ) { }


EntityMgr::~EntityMgr( ) { }

static int num_entity = 10000;
static std::vector< ChunkFaceVertices > list_faces;

void EntityMgr::init( ) { 
	printf( "\n*** EntityMgr ***\n" );
	client.resource_mgr.reg_pool< Entity >( num_entity );
	client.resource_mgr.reg_pool< ECState >( num_entity );
	client.resource_mgr.reg_pool< ECTnt >( num_entity );
	client.resource_mgr.reg_pool< ECGravBlock >( num_entity );
	client.resource_mgr.reg_pool< ECSpawnBlock >( num_entity );
	client.resource_mgr.reg_pool< ECWater >( num_entity );

	list_entity.reserve( num_entity );
	map_entity.reserve( num_entity );

	GL_CHECK( init_mesh( ) );

	alloc_base = [ ] ( Client & client, Entity & entity ) {
		entity.time_live = client.time_mgr.get_time( TimeStrings::GAME );
		entity.is_shutdown = false;
		entity.is_visible = true;
		entity.color = { 1.0f, 1.0f, 1.0f, 1.0f };
		entity.is_dirty = true;

		if( !client.resource_mgr.allocate( entity.h_state ) ) { 
			return ErrorEntity::EE_Failed;
		}

		auto & ec_state = entity.h_state.get( );
		ec_state.pos = glm::vec3( 0, 0, 0 );
		ec_state.pos_last = glm::vec3( 0, 0, 0 );
		ec_state.pos_delta = glm::vec3( 0, 0, 0 );
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
	
	loader_add( &Player( ) );
	loader_add( &Tnt( ) );
	loader_add( &GravBlock( ) );
	loader_add( &LineBlock( ) );
	loader_add( &SpawnBlock( ) );
	loader_add( &WormBlock( ) );
	loader_add( &WaterBlock( ) );

	entity_add( "Player", [ ] ( Client & client, Entity & entity ) {

		return ErrorEntity::EE_Ok;
	} );

	entity_player = &list_entity[ 0 ].get( );
}

void EntityMgr::update( ) { 
	std::lock_guard< std::mutex > lock( mtx_entity );

	auto iter_entity = list_entity.begin( );
	while( iter_entity != list_entity.end( ) ) {
		if( iter_entity->get( ).is_shutdown ) {
			entity_remove( *iter_entity );
			iter_entity = list_entity.erase( iter_entity );
		}
		else {
			iter_entity++;
		}
	}

	client.resource_mgr.pool< Entity >().apply_func_live_threads( client.thread_mgr,
		10, client.thread_mgr.cnt_thread_sync( ) * 10, [ & ] ( Entity & entity ) { 

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

	auto & out = client.display_mgr.out;
	out.str( "" );
	out << "[Entity Live] Entity: " << list_entity.size( );
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	auto & ecp_state = entity_player->h_state.get( );
	out << "[Player ] Veloc: " << Directional::print_vec( ecp_state.veloc );
	client.gui_mgr.print_to_static( out.str( ) );
}

void EntityMgr::render( ) {
	Entity * entity;
	client.texture_mgr.bind_program( "Entity" );

	static GLuint idx_mat_model = glGetUniformLocation( client.texture_mgr.id_bound_program, "mat_model" );
	static GLuint idx_mat_norm = glGetUniformLocation( client.texture_mgr.id_bound_program, "mat_norm" );
	static GLuint idx_idx_layer = glGetUniformLocation( client.texture_mgr.id_bound_program, "idx_layer" );
	static GLuint idx_frag_color = glGetUniformLocation( client.texture_mgr.id_bound_program, "entity_color" );

	auto iter = list_entity.begin( );
	while( iter != list_entity.end( ) ) {
		entity = &iter->get( );

		if( !entity->is_visible ) {
			iter++;
			continue;
		}

		glUniformMatrix4fv( idx_mat_model, 1, GL_FALSE, glm::value_ptr( entity->h_state.get( ).mat_model ) );
		glUniformMatrix3fv( idx_mat_norm, 1, GL_FALSE, glm::value_ptr( entity->h_state.get( ).mat_norm ) );
		glUniform1f( idx_idx_layer, client.chunk_mgr.get_block_data( entity->id ).faces[ 0 ].id_subtex );
		glUniform4fv( idx_frag_color, 1, ( const GLfloat * ) &entity->color );

		vbo.render( client );

		iter++;
	}
}

void EntityMgr::render_shadow( glm::mat4 & mat_light ) { 
	Entity * entity;
	client.texture_mgr.bind_program( "ShadowMap" );

	static GLuint idx_mat_model = glGetUniformLocation( client.texture_mgr.id_bound_program, "mat_model" );
	static GLuint idx_mat_norm = glGetUniformLocation( client.texture_mgr.id_bound_program, "mat_norm" );
	static GLuint idx_idx_layer = glGetUniformLocation( client.texture_mgr.id_bound_program, "idx_layer" );
	static GLuint idx_frag_color = glGetUniformLocation( client.texture_mgr.id_bound_program, "entity_color" );

	auto iter = list_entity.begin( );
	while( iter != list_entity.end( ) ) {
		entity = &iter->get( );

		if( !entity->is_visible ) {
			iter++;
			continue;
		}

		glUniformMatrix4fv( idx_mat_model, 1, GL_FALSE, glm::value_ptr( entity->h_state.get( ).mat_model ) );
		glUniformMatrix3fv( idx_mat_norm, 1, GL_FALSE, glm::value_ptr( entity->h_state.get( ).mat_norm ) );
		glUniform1f( idx_idx_layer, client.chunk_mgr.get_block_data( entity->id ).faces[ 0 ].id_subtex );
		glUniform4fv( idx_frag_color, 1, ( const GLfloat * ) &entity->color );

		vbo.render( client );

		iter++;
	}
}

void EntityMgr::end( ) { 

}

void EntityMgr::sec( ) { 

}

void EntityMgr::init_mesh( ) { 
	vbo.init( );

	vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
		"Entity",
		client.texture_mgr.get_texture_id( "Blocks" ),
		std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
	) );

	for( int i = 0; i < FaceDirection::FD_Size; ++i ) {
		auto color = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
		auto & norm = Directional::get_vec_dir_f( ( FaceDirection ) i );

		for( int j = 0; j < 4; ++j ) {
			vbo.push_data( VBO::Vertex {
				verts_entity[ i ][ j ].x, verts_entity[ i ][ j ].y, verts_entity[ i ][ j ].z,
				color.r, color.g, color.b, color.a,
				norm[ 0 ], norm[ 1 ], norm[ 2 ],
				uvs_entity[ j ].x, uvs_entity[ j ].y, 0
			} );
		}
	}

	vbo.finalize_set( );
	vbo.buffer( );
}

void EntityMgr::loader_add( EntityLoader * entity_loader ) { 
	auto & out = client.display_mgr.out;
	out.str( "" );

	auto iter_map = map_loader.find( entity_loader->name );
	if( iter_map != map_loader.end( ) ) {
		out << "ERROR: Loader duplicate: " << entity_loader->name;
		client.gui_mgr.print_to_console( out.str( ) );
		return;
	}

	list_loader.emplace_back( EntityLoader {
		entity_loader->name,
		entity_loader->ef_alloc,
		entity_loader->ef_release,
		entity_loader->ef_update,
		entity_loader->ef_mesh
	} );

	map_loader.emplace(
		std::pair< std::string, int > {
		entity_loader->name,
			( GLuint ) list_loader.size( ) - 1
	}
	);

	out << "SUCCESS: Loader added: " << entity_loader->name;
	client.gui_mgr.print_to_console( out.str( ) );
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

	list_loader.emplace_back( EntityLoader {
		str_name,
		ef_alloc,
		ef_release,
		ef_update,
		ef_mesh
	} );

	map_loader.emplace( 
		std::pair< std::string, int > {
			str_name,
			( int ) list_loader.size( ) - 1
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
			std::cout << out.str( ) << std::endl;
		} );
		return;
	}

	if( !client.resource_mgr.allocate( h_entity ) ) {
		client.thread_mgr.task_main( 5, [ & ] ( ) {
			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "ENTITY ERROR: Out of entities!";
			client.gui_mgr.print_to_console( out.str( ) );
			std::cout << out.str( ) << std::endl;
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
			std::cout << out.str( ) << std::endl;
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
	Block * ptr_block;

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
				ptr_block = &client.chunk_mgr.get_block_data( id_block );

				if( ptr_block->is_coll ) {
					ec_state.is_coll = true;
					return;
				}
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_b( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_x; int num_y;
	float step_x; float step_y;
	int id_block;
	Block * ptr_block;

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
				ptr_block = &client.chunk_mgr.get_block_data( id_block );

				if( ptr_block->is_coll ) {
					ec_state.is_coll = true;
					return;
				}
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_l( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_z; int num_y;
	float step_z; float step_y;
	int id_block;
	Block * ptr_block;

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
				ptr_block = &client.chunk_mgr.get_block_data( id_block );

				if( ptr_block->is_coll ) {
					ec_state.is_coll = true;
					return;
				}
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_r( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_z; int num_y;
	float step_z; float step_y;
	int id_block;
	Block * ptr_block;

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
				ptr_block = &client.chunk_mgr.get_block_data( id_block );

				if( ptr_block->is_coll ) {
					ec_state.is_coll = true;
					return;
				}
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_u( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_x; int num_z;
	float step_x; float step_z;
	int id_block;
	Block * ptr_block;

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
				ptr_block = &client.chunk_mgr.get_block_data( id_block );

				if( ptr_block->is_coll ) {
					ec_state.is_coll = true;
					return;
				}
			}
		}
	}
}

void EntityMgr::entity_terrain_collide_d( ECState & ec_state ) {
	glm::vec3 pos_coll_s;
	int num_x; int num_z;
	float step_x; float step_z;
	int id_block;
	Block * ptr_block;

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
				ptr_block = &client.chunk_mgr.get_block_data( id_block );

				if( ptr_block->is_coll ) {
					ec_state.is_coll = true;
					return;
				}
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