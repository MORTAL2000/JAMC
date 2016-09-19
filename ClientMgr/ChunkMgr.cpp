#include "ChunkMgr.h"

#include "Client.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "tinyxml2-master\tinyxml2.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>

#include "simplexnoise.h"

ChunkMgr::ChunkMgr( Client & client ) :
	Manager( client ),
	SHADOW_WIDTH( 4096 ), SHADOW_HEIGHT( 4096 ),
	dim_shadow { 200.0f, 200.0f },
	pos_shadow { 0.0f, 0.0f } { }

ChunkMgr::~ChunkMgr( ) { }

void ChunkMgr::init( ) {
	printf( "\n*** ChunkMgr ***\n" );

	client.resource_mgr.reg_pool< Chunk >( WorldSize::World::num_chunks );

	map_chunks.reserve( WorldSize::World::num_chunks );
	map_dirty.reserve( WorldSize::World::num_chunks );
	map_noise.reserve( ( WorldSize::World::size_x * 2 + 1 ) * ( WorldSize::World::size_z * 2 + 1 ) + 32 );

	pos_center_chunk_lw = glm::ivec3( 0, 0, 0 );

	load_block_data( );

	GLuint size_verts = WorldSize::Chunk::size_x * WorldSize::Chunk::size_z * 4;
	GLuint size_inds = WorldSize::Chunk::size_x * WorldSize::Chunk::size_z * 6;

	GLuint num_verts = ( WorldSize::World::size_x * 2 + 1 ) * ( WorldSize::World::size_z * 2 + 1 ) * 8;
	GLuint num_inds = ( WorldSize::World::size_x * 2 + 1 ) * ( WorldSize::World::size_z * 2 + 1 ) * 8;

	GLuint num_buff = 1024;
	GLuint size_buff_verts = WorldSize::Chunk::size_x * WorldSize::Chunk::size_z * 4 * 2;
	GLuint size_buff_inds = WorldSize::Chunk::size_x * WorldSize::Chunk::size_z * 6 * 2;

	std::cout << "Vert size: " << sizeof( VertTerrain ) << std::endl;
	std::cout << "Uint size: " << sizeof( GLuint ) << std::endl;

	sm_terrain.init(
		size_verts, num_verts, size_inds, num_inds,
		num_buff, size_buff_verts, size_buff_inds );

	client.texture_mgr.update_uniform( "SMTerrain", "dist_fade", ( GLfloat ) WorldSize::Chunk::size_x * ( WorldSize::World::size_x - 1 ) );
	client.texture_mgr.update_uniform( "SMTerrain", "dist_fade_cutoff", ( GLfloat ) WorldSize::Chunk::size_x * WorldSize::World::size_x );

	client.texture_mgr.update_uniform( "SMTerrainBasic", "dist_fade", ( GLfloat ) WorldSize::Chunk::size_x * ( WorldSize::World::size_x - 1 ) );
	client.texture_mgr.update_uniform( "SMTerrainBasic", "dist_fade_cutoff", ( GLfloat ) WorldSize::Chunk::size_x * WorldSize::World::size_x );

	/*
	shared_mesh_inclusive.init(
		4 * 6 * 50, ( GLuint ) list_block_data.size( ),
		6 * 6 * 50, ( GLuint ) list_block_data.size( ),
		4, 4 * 6 * 50, 6 * 6 * 50 );

	load_block_mesh( );

	shared_mesh.clear_commands( );

	for( GLuint i = 0; i < list_handles_inclusive.size( ); ++i ) { 
		list_handles_inclusive[ i ].submit_commands( glm::translate( glm::mat4( 1.0f ), glm::vec3( 0, 100, 0 + i ) ) );
	}

	shared_mesh_inclusive.buffer_commands( );
	*/

	GL_CHECK( init_light( ) );
	GL_CHECK( init_skybox( ) );
	GL_CHECK( init_shadowmap( ) );
	GL_CHECK( init_debug( ) );

	using namespace std::tr2::sys;
	path path_world( "./World" );

	if( !exists( path_world ) ) {
		create_directory( path_world );
	}

	std::error_code error;

	for( directory_iterator iter_dir( path_world ); iter_dir != directory_iterator( ); ++iter_dir ) { 
		remove_all( iter_dir->path( ), error );

		if( error ) {
			printf( "\nError removing directory: %s\n", error.message( ).c_str( ) );
		}
	}

	/*
	else { 
		std::error_code error;

		remove_all( path_world, error );

		if( error ) {
			printf( "\nError removing directory: %s\n", error.message( ).c_str( ) );
		}

		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

		create_directory( path_world, error );

		if( error ) { 
			printf( "\nError creating directory: %s\n", error.message( ).c_str( ) );
		}
	}
	*/

	// Base Biome
	{
		Biome biome;
		biome.name_biome = "Base";
		biome.id_block_surface = get_block_data( "Grass" ).id;
		biome.id_block_depth = get_block_data( "Dirt" ).id;

		biome.list_noise.push_back( {
			3,
			-7, 13,
			500.0f, 500.0f,
			0.001f, 0.001f, 10.0f,
			{ { -10.0f, 10.0f } }
		} );

		list_biomes.push_back( biome );
		map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
	}

	// Ice Shard Biome
	{
		Biome biome;
		biome.name_biome = "Ice Shards";
		biome.id_block_surface = get_block_data( "Ice" ).id;
		biome.id_block_depth = get_block_data( "Ice" ).id;

		biome.noise_biome = {
			0,
			-100, 100,
			991.2f, 2991.2f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 100.0f } }
		};

		biome.noise_lerp = {
			0,
			-100, 100,
			991.2f, 2991.2f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 52.5f } }
		};

		biome.list_noise.push_back( {
			10,
			10, 10,
			0, 0,
			0.1f, 0.1f, 0.0f,
			{ { 0.0f, 0.0f } }
		} );

		biome.list_noise.push_back( {
			0,
			0, 15,
			0, 0,
			0.05f, 0.05f, 15.0f,
			{ { -15.0f, 15.0f } }
		} );

		list_biomes.push_back( biome );
		map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
	}

	// Sand Plateau Biome
	{
		Biome biome;
		biome.name_biome = "Sand Plateau";
		biome.id_block_surface = get_block_data( "Sand" ).id;
		biome.id_block_depth = get_block_data( "Sand" ).id;

		biome.noise_biome = {
			0,
			-100, 100,
			10101.2f, 10101.2f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 100.0f } }
		};
		biome.noise_lerp = {
			0,
			-100, 100,
			10101.2f, 10101.2f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 55.0f } }
		};

		biome.list_noise.push_back( {
			10,
			0, 12,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 0.0f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 12,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 0.0f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 15,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 2.5f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 17,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 5.0f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 20,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 7.5f, 10.0f } }
		} );

		list_biomes.push_back( biome );
		map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
	}

	// Cobblestone Chasm Biome
	{
		Biome biome;
		biome.name_biome = "Cobblestone Chasm";
		biome.id_block_surface = get_block_data( "Cobblestone" ).id;
		biome.id_block_depth = get_block_data( "Cobblestone" ).id;

		biome.noise_biome = {
			0,
			-100, 100,
			1901.2f, 1101.2f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 100.0f } }
		};
		biome.noise_lerp = {
			0,
			-100, 100,
			1901.2f, 1101.2f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 55.0f } }
		};

		biome.list_noise.push_back( {
			0,
			-6, 6,
			1231.2f, 1231.2f,
			0.01f, 0.01f, 6.0f,
			{ { -6.0f, 6.0f } }
		} );

		biome.list_noise.push_back( {
			30,
			-30, 0,
			234.2f, 234.2f,
			0.03f, 0.009f, 60.0f,
			{ { -60.0f, -30.0f } }
		} );

		list_biomes.push_back( biome );
		map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
	}

	// Grass Plateau Biome
	{
		Biome biome;
		biome.name_biome = "Grass Plateau";
		biome.id_block_surface = get_block_data( "Grass" ).id;
		biome.id_block_depth = get_block_data( "Dirt" ).id;

		biome.noise_biome = {
			0,
			-100, 100,
			331.9f, 331.9f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 100.0f } }
		};
		biome.noise_lerp = {
			0,
			-100, 100,
			331.9f, 331.9f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 55.0f } }
		};

		biome.list_noise.push_back( {
			10,
			0, 12,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 0.0f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 12,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 0.0f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 15,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 2.5f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 17,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 5.0f, 10.0f } }
		} );

		biome.list_noise.push_back( {
			10,
			0, 20,
			10101.2f, 10101.2f,
			0.01f, 0.01f, 10.0f,
			{ { 7.5f, 10.0f } }
		} );

		list_biomes.push_back( biome );
		map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
	}

	// Grass Hill Biome
	{
		Biome biome;
		biome.name_biome = "Grass Hill";
		biome.id_block_surface = get_block_data( "Grass" ).id;
		biome.id_block_depth = get_block_data( "Dirt" ).id;

		biome.noise_biome = {
			0,
			-100, 100,
			3031.9f, 3031.9f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 100.0f } }
		};
		biome.noise_lerp = {
			0,
			-100, 100,
			3031.9f, 3031.9f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 55.0f } }
		};

		biome.list_noise.push_back( {
			0,
			-30, 30,
			101.2f, 101.2f,
			0.008f, 0.008f, 30.0f,
			{ { -30.0f, 30.0f } }
		} );

		list_biomes.push_back( biome );
		map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
	}

	// Ocean
	{
		Biome biome;
		biome.name_biome = "Ocean";
		biome.id_block_surface = get_block_data( "Sand" ).id;
		biome.id_block_depth = get_block_data( "Sand" ).id;

		biome.noise_biome = {
			0,
			-100, 100,
			6031.9f, 6031.9f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 100.0f } }
		};
		biome.noise_lerp = {
			0,
			-100, 100,
			6031.9f, 6031.9f,
			0.001f, 0.001f, 100.0f,
			{ { 50.0f, 65.0f } }
		};

		biome.list_noise.push_back( {
			-5,
			-30, -3,
			6631.9f, 6631.9f,
			0.01f, 0.01f, 30.0f,
			{ { -30.0f, 30.0f } }
		} );

		biome.list_noise.push_back( {
			-10,
			-50, -5,
			6931.9f, 6931.9f,
			0.01f, 0.01f, 40.0f,
			{ { -40.0f, 40.0f } }
		} );


		list_biomes.push_back( biome );
		map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
	}

	is_chunk_debug = false;
	is_shadow_debug = false;
	is_shadows = true;
}

int time_last_map = 0;
int cooldown_map = 100;

int time_last_remesh = 0;
int cooldown_remesh = 200;
glm::ivec3 vect_refresh = { 1, 1, 1 };

void ChunkMgr::update( ) {
	client.time_mgr.begin_record( RecordStrings::UPDATE_MAP );

	calc_light( );

	Directional::pos_gw_to_lw( client.display_mgr.camera.pos_camera, pos_center_chunk_lw );

	int const time_now = client.time_mgr.get_time( TimeStrings::GAME );

	if( time_now - time_last_map > cooldown_map ) {
		client.thread_mgr.task_async( 10, [ & ] ( ) {
			chunk_add( pos_center_chunk_lw );

			std::lock_guard< std::recursive_mutex > lock_chunks( mtx_chunks );

			for( auto & pair_handle : map_chunks ) { 
				auto & chunk = pair_handle.second.get( );

				if( chunk.cnt_adj != 6
					&& chunk.is_loaded
					&& !chunk.is_shutdown ) {

					if( !Directional::is_within_range( chunk.pos_lw, WorldSize::World::vec_size, pos_center_chunk_lw ) ) {
						chunk_state( chunk, ChunkState::CS_Save, true );
						chunk_state( chunk, ChunkState::CS_Remove, true );
						chunk.is_shutdown = true;
					}
					else {
						glm::ivec3 pos_adj;

						for( int i = 0; i < FaceDirection::FD_Size; i++ ) {
							if( chunk.ptr_adj[ i ] == nullptr ) {
								pos_adj = chunk.pos_lw + Directional::get_vec_dir_i( ( FaceDirection ) i );

								if( Directional::is_within_range( pos_adj, WorldSize::World::vec_size, pos_center_chunk_lw ) ) {
									chunk_add( pos_adj );
								}
							}
						}
					}
				}
			}
		} );

		time_last_map = time_now;
	}

	if( time_now - time_last_remesh > cooldown_remesh ) {
		client.thread_mgr.task_async( 10, [ & ] ( ) {
			std::lock_guard< std::recursive_mutex > lock_chunks( mtx_chunks );

			glm::ivec3 pos_chunk;
			auto iter_chunks = map_chunks.end( );
			for( int i = pos_center_chunk_lw.x - vect_refresh.x; i <= pos_center_chunk_lw.x + vect_refresh.x; i++ ) { 
				for( int j = pos_center_chunk_lw.y - vect_refresh.y; j <= pos_center_chunk_lw.y + vect_refresh.y; j++ ) {
					for( int k = pos_center_chunk_lw.z - vect_refresh.z; k <= pos_center_chunk_lw.z + vect_refresh.z; k++ ) {
						pos_chunk = glm::ivec3( i, j, k );
						iter_chunks = map_chunks.find( Directional::get_hash( pos_chunk ) );
						if( iter_chunks != map_chunks.end( ) ) {
							auto & chunk = iter_chunks->second.get( );
							if( chunk.is_loaded && !chunk.is_shutdown ) {
								chunk_state( chunk, ChunkState::CS_TMesh, true );
							}
						}
					}
				}
			}
		} );

		time_last_remesh = time_now;
	}

	//client.thread_mgr.task_async( 10, [ & ] ( ) {
		std::lock_guard< std::recursive_mutex > lock_dirty( mtx_dirty );

		auto iter = map_dirty.begin( );
		while( iter != map_dirty.end() ){
			auto & chunk = iter->second;

			if( chunk.cnt_states ) { 
				chunk_update( chunk );
				iter++;
			}
			else { 
				iter = map_dirty.erase( iter );
			}
		}
	//} );

	client.time_mgr.end_record( RecordStrings::UPDATE_MAP );
	client.time_mgr.push_record( RecordStrings::UPDATE_MAP );

	auto & out = client.display_mgr.out;
	out.str( "" );
	out << "[Map Size]" << 
		" Chunk: " << map_chunks.size( ) << 
		" Noise: " << map_noise.size( ) << 
		" File: " << map_file.size( ) <<
		" Dirty: " << map_dirty.size( );
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Map Capacity]" << 
		" Chunk: " << map_chunks.bucket_count( ) <<
		" Noise: " << map_noise.bucket_count( ) <<
		" File: " << map_file.bucket_count( ) <<
		" Dirty: " << map_dirty.bucket_count( );
	client.gui_mgr.print_to_static( out.str( ) );
	out.str( "" );
	out << "[Sun] Deg:" << pos_deg_light;
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Render List]" <<
		" size: " << list_render.size( );
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Shared Mesh]" <<
		" commands: " << num_cmds <<
		" Triangles: " << num_triangles;
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Shared Mesh Buffers]" <<
		" avail: " << sm_terrain.size_buffer_avail( ) <<
		" live: " << sm_terrain.size_buffer_live( );
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Shared Mesh Vbo]" <<
		" live: " << sm_terrain.size_vbo_live( ) <<
		" avail: " << sm_terrain.size_vbo_avail( );
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Shared Mesh Ibo]" <<
		" live: " << sm_terrain.size_ibo_live( ) <<
		" avail: " << sm_terrain.size_ibo_avail( );
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Sun] Deg:" << pos_deg_light;
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Sun] Pos:" << Directional::print_vec( light_data.sun_data.pos_sun );
	client.gui_mgr.print_to_static( out.str( ) );
}

void ChunkMgr::render( ) {
	GL_CHECK( render_skybox( ) );
	GL_CHECK( render_exlude( ) );
	GL_CHECK( render_sort( ) );
	GL_CHECK( render_build( ) );
	GL_CHECK( render_pass_shadow( ) );
	GL_CHECK( render_pass_solid( ) );
	GL_CHECK( render_pass_trans( ) );
	GL_CHECK( render_debug( ) );
}

void ChunkMgr::render_skybox( ) {
	GLuint dim_skybox = ( WorldSize::World::size_x + 1 ) * WorldSize::Chunk::size_x;
	glm::mat4 mat_model = 
		glm::translate( glm::mat4( 1.0f ), client.display_mgr.camera.pos_camera ) * 
		glm::rotate( glm::mat4( 1.0f ), glm::radians( client.time_mgr.get_time( TimeStrings::GAME ) / 1000.0f ), glm::vec3( 0, 1, 0 ) ) *
		glm::scale( glm::mat4( 1.0f ), glm::vec3( dim_skybox, dim_skybox, dim_skybox ) );

	client.texture_mgr.update_uniform( "BasicPersp", "mat_model", mat_model );
	vbo_skybox.render( client, true );

	auto & pos_sun = light_data.sun_data.pos_sun;

	mat_model = glm::mat4( 1.0f ) *
		glm::translate( glm::mat4( 1.0f ), client.display_mgr.camera.pos_camera + glm::vec3( pos_sun ) ) *
		glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 0, 1, 0 ) ) *
		glm::rotate( glm::mat4( 1.0f ), -glm::atan( pos_sun.y, pos_sun.x ), glm::vec3( 1, 0, 0 ) ) *
		glm::scale( glm::mat4( 1.0f ), glm::vec3( 50.0f, 50.0f, 1.0f ) );

	client.texture_mgr.update_uniform( "BasicPersp", "mat_model", mat_model );
	vbo_sun.render( client, true );
}

void ChunkMgr::render_exlude( ) {
	static glm::ivec3 pos_check[ 8 ] = { 
		{ 0, 0, 0 }, { WorldSize::Chunk::vec_size.x, 0, 0 }, { WorldSize::Chunk::vec_size.x, WorldSize::Chunk::vec_size.y, 0 }, 
		{ 0, WorldSize::Chunk::vec_size.y, 0 }, { 0, 0, WorldSize::Chunk::vec_size.z }, { WorldSize::Chunk::vec_size.x, 0, WorldSize::Chunk::vec_size.z },
		{ WorldSize::Chunk::vec_size.x, WorldSize::Chunk::vec_size.y, WorldSize::Chunk::vec_size.z }, { 0, WorldSize::Chunk::vec_size.y, WorldSize::Chunk::vec_size.z }
	};

	auto & camera = client.display_mgr.camera;
	float dist = 0;
	bool in_cone = false;

	client.time_mgr.begin_record( RecordStrings::RENDER_EXLUSION );

	list_render.clear( );

	for( auto & pair_render : map_render ) {
		auto & chunk = pair_render.second;
		dist = glm::distance( 
			glm::vec2( chunk.pos_lw.x, chunk.pos_lw.z ), 
			glm::vec2( pos_center_chunk_lw.x, pos_center_chunk_lw.z ) 
		);

		for( GLuint i = 0; i < 8; ++i ) {
			if( dist < 4.0f ) { 
				list_render.push_back( &chunk );
				break;
			}

			if( dist > ( WorldSize::World::size_x + WorldSize::World::size_z ) / 2.0f + 1 ) {
				continue;
			}

			in_cone = Directional::is_point_in_cone(
				chunk.pos_gw + pos_check[ i ], camera.pos_camera,
				camera.pos_camera + camera.vec_front, client.display_mgr.fov / 3.0f * 4.0f + 10.0f );

			if( in_cone ) {
				list_render.push_back( &chunk );
				break; 
			}
		}
	}

	client.time_mgr.end_record( RecordStrings::RENDER_EXLUSION );
	client.time_mgr.push_record( RecordStrings::RENDER_EXLUSION );
}

void ChunkMgr::render_sort( ) {
	client.time_mgr.begin_record( RecordStrings::RENDER_SORT );

	std::sort( list_render.begin( ), list_render.end( ), [ & ] ( Chunk * lro, Chunk * rho ) {
		return	glm::length( client.display_mgr.camera.pos_camera - glm::vec3( lro->pos_gw + WorldSize::Chunk::vec_size / 2 ) ) >
			glm::length( client.display_mgr.camera.pos_camera - glm::vec3( rho->pos_gw + WorldSize::Chunk::vec_size / 2 ) );
	} );

	client.time_mgr.end_record( RecordStrings::RENDER_SORT );
	client.time_mgr.push_record( RecordStrings::RENDER_SORT );
}

void ChunkMgr::render_build( ) {
	idx_solid = 0;
	idx_trans = 0;

	num_cmds = 0;
	num_triangles = 0;

	sm_terrain.clear_commands( );

	for( auto chunk : list_render ) {
		chunk->handle_solid.submit_commands( chunk->pos_gw );
	}

	idx_solid = sm_terrain.size_commands( );

	for( auto chunk : list_render ) {
		chunk->handle_trans.submit_commands( chunk->pos_gw );
	}

	idx_trans = sm_terrain.size_commands( );

	sm_terrain.buffer_commands( );

	/*
	shared_mesh_inclusive.clear_commands( );

	for( auto chunk : list_render ) {
		for( auto & tuple_pos : chunk->list_incl_solid ) { 
			list_handles_inclusive[ std::get< 1 >( tuple_pos ) ].submit_commands( glm::translate( glm::mat4( 1.0f ), std::get< 2 >( tuple_pos ) ) );
		}
		for( auto & tuple_pos : chunk->list_incl_trans ) {
			list_handles_inclusive[ std::get< 1 >( tuple_pos ) ].submit_commands( glm::translate( glm::mat4( 1.0f ), std::get< 2 >( tuple_pos ) ) );
		}
	}

	shared_mesh_inclusive.buffer_commands( );
	*/

	num_cmds += sm_terrain.size_commands( );
	num_triangles += sm_terrain.num_primitives( );
}

void ChunkMgr::render_pass_shadow( ) {
	static GLuint id_blocks = client.texture_mgr.get_texture_id( "Blocks" );

	if( is_shadow_debug ) {
		client.texture_mgr.unbind_program( );
		client.display_mgr.set_ortho( );

		dim_shadow = { 300, 300 };

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		for( GLuint i = 0; i < num_cascades; ++i ) {
			glBindTexture( GL_TEXTURE_2D, id_tex_depth[ i ] );

			pos_shadow = { 25 + ( i * dim_shadow.x + 25 ), 250 };

			glBegin( GL_QUADS );

			glTexCoord2f( 0.0f, 0.0f );
			glVertex2f( pos_shadow.x, pos_shadow.y );

			glTexCoord2f( 1.0f, 0.0f );
			glVertex2f( pos_shadow.x + dim_shadow.x, pos_shadow.y );

			glTexCoord2f( 1.0f, 1.0f );
			glVertex2f( pos_shadow.x + dim_shadow.x, pos_shadow.y + dim_shadow.y );

			glTexCoord2f( 0.0f, 1.0f );
			glVertex2f( pos_shadow.x, pos_shadow.y + dim_shadow.y );

			glEnd( );
		}

		client.display_mgr.set_proj( );
	}

	if( is_shadows ) {
		static GLuint idx_mat_light_solid = glGetUniformLocation( client.texture_mgr.get_program_id( "SMShadowMapSolid" ), "mat_light" );
		static GLuint idx_mat_light_trans = glGetUniformLocation( client.texture_mgr.get_program_id( "SMShadowMapTrans" ), "mat_light" );

		auto & window = client.display_mgr.get_window( );
		auto & camera = client.display_mgr.camera;

		glm::mat4 mat_inv_cam = glm::inverse( client.display_mgr.camera.mvp_matrices.mat_view );

		float ar =  float( 4.0f ) / 3.0f;
		float h_hfov = client.display_mgr.fov * ar / 2.0f;
		float h_vfov = client.display_mgr.fov / 2.0f;
		float tanh_hfov = glm::tan( glm::radians( h_hfov ) );
		float tanh_vfov = glm::tan( glm::radians( h_vfov ) );

		/*
		float tanh_hfov = glm::tan( client.display_mgr.fov / 2.0f );
		float tanh_vfov = glm::tan( ( client.display_mgr.fov / ar ) / 2.0f );
		*/

		/*
		GLuint num_skip = 2;
		GLuint i = ++idx_cascade % ( num_cascades * num_skip );

		if( i % 3 != 0 ) { 
			return;
		}

		i = i / num_skip;
		*/

		GLuint i = ++idx_cascade % num_cascades;

		float zclip[ num_cascades + 1 ];
		zclip[ 0 ] = 0.2f;
		for( GLuint j = 0; j < num_cascades; ++j ) { 
			zclip[ j + 1 ] = depth_cascades[ j ];
		}

		//for( GLuint i = 0; i < num_cascades; i++ ) {
			mat_view_light = glm::lookAt( 
				client.display_mgr.camera.pos_camera, 
				client.display_mgr.camera.pos_camera - glm::vec3( client.chunk_mgr.light_data.sun_data.pos_sun ),
				glm::vec3( 0.0f, 1.0f, 0.0f ) );

			float xn = zclip[ i ] * tanh_hfov;
			float xf = zclip[ i + 1 ] * tanh_hfov;

			float yn = zclip[ i ] * tanh_vfov;
			float yf = zclip[ i + 1 ] * tanh_vfov;

			/*
			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "h_hfov: " << h_hfov << " h_vfov: " << h_vfov << "\n";
			out << "tanh_hfov: " << tanh_hfov << " tanh_vfov: " << tanh_vfov << "\n";
			out << "zclip0: " << zclip[ 0 ] << " zclip1: " << zclip[ 1 ] << "\n";
			out << "xn: " << xn << " yn: " << yn << " xf: " << xf << " yf: " << yf << "\n";
			client.gui_mgr.print_to_console( out.str( ) );
			*/

			corners_frustrum[ i ][ 0 ] = glm::vec4 { xn, yn, zclip[ i ], 1.0f };
			corners_frustrum[ i ][ 1 ] = glm::vec4 { -xn, yn, zclip[ i ], 1.0f };
			corners_frustrum[ i ][ 2 ] = glm::vec4 { xn, -yn, zclip[ i ], 1.0f };
			corners_frustrum[ i ][ 3 ] = glm::vec4 { -xn, -yn, zclip[ i ], 1.0f };

			corners_frustrum[ i ][ 4 ] = glm::vec4 { xf, yf, zclip[ i + 1 ], 1.0f };
			corners_frustrum[ i ][ 5 ] = glm::vec4 { -xf, yf, zclip[ i + 1 ], 1.0f };
			corners_frustrum[ i ][ 6 ] = glm::vec4 { xf, -yf, zclip[ i + 1 ], 1.0f };
			corners_frustrum[ i ][ 7 ] = glm::vec4 { -xf, -yf, zclip[ i + 1 ], 1.0f };

			sides_ortho[ i ][ 0 ] = std::numeric_limits< float >::max( );
			sides_ortho[ i ][ 1 ] = -std::numeric_limits< float >::max( );

			sides_ortho[ i ][ 2 ] = std::numeric_limits< float >::max( );
			sides_ortho[ i ][ 3 ] = -std::numeric_limits< float >::max( );

			sides_ortho[ i ][ 4 ] = std::numeric_limits< float >::max( );
			sides_ortho[ i ][ 5 ] = -std::numeric_limits< float >::max( );

			for( GLuint j = 0; j < 8; ++j ) {
				corners_frustrum[ i ][ j ] = mat_inv_cam * corners_frustrum[ i ][ j ];
				//corners_frustrum[ i ][ j ] -= glm::vec4( client.display_mgr.camera.pos_camera, 0 );
				corners_frustrum[ i ][ j ] = mat_view_light * corners_frustrum[ i ][ j ];

				//corners_frustrum[ i ][ j ] = mat_view_light * corners_frustrum[ i ][ j ];

				sides_ortho[ i ][ 0 ] = std::min( sides_ortho[ i ][ 0 ], corners_frustrum[ i ][ j ].x );
				sides_ortho[ i ][ 1 ] = std::max( sides_ortho[ i ][ 1 ], corners_frustrum[ i ][ j ].x );

				sides_ortho[ i ][ 2 ] = std::min( sides_ortho[ i ][ 2 ], corners_frustrum[ i ][ j ].y );
				sides_ortho[ i ][ 3 ] = std::max( sides_ortho[ i ][ 3 ], corners_frustrum[ i ][ j ].y );

				sides_ortho[ i ][ 4 ] = std::min( sides_ortho[ i ][ 4 ], corners_frustrum[ i ][ j ].z );
				sides_ortho[ i ][ 5 ] = std::max( sides_ortho[ i ][ 5 ], corners_frustrum[ i ][ j ].z );
			}

			//printf( "z: %f to %f\n", minZ, maxZ );
			//printf( "x: %f to %f, y: %f to %f, z: %f to %f\n", minX, maxX, minY, maxY, minZ, maxZ );
			
			mat_ortho_light[ i ] = glm::ortho( 
				-sides_ortho[ i ][ 0 ],
				-sides_ortho[ i ][ 1 ],
				-sides_ortho[ i ][ 2 ],
				-sides_ortho[ i ][ 3 ],
				-768.0f, 768.0f
				//sides_ortho[ i ][ 4 ], 
				//sides_ortho[ i ][ 5 ]
				//sides_ortho[ i ][ 4 ], sides_ortho[ i ][ 5 ]
			) * mat_view_light;
			

			/*
			mat_ortho_light[ i ] = glm::ortho(
				-10.0f, 10.0f,
				-10.0f, 10.0f,
				-500.0f, 500.0f
				//sides_ortho[ i ][ 4 ], sides_ortho[ i ][ 5 ]
			) * mat_view_light;
			*/
		//}

		glCullFace( GL_FRONT );

		glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
		glBindFramebuffer( GL_FRAMEBUFFER, id_depth_fbo );

		//for( GLuint i = 0; i < num_cascades; i++ ) {
			client.texture_mgr.bind_texture( i + 1, id_tex_depth[ i ] );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id_tex_depth[ i ], 0 );

			glViewport( 0, 0, SHADOW_WIDTH, SHADOW_HEIGHT );
			glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

			client.texture_mgr.bind_program( "SMShadowMapSolid" );
			glUniformMatrix4fv( idx_mat_light_solid, 1, GL_FALSE, glm::value_ptr( mat_ortho_light[ i ] ) );

			client.texture_mgr.bind_texture_array( 0, id_blocks );

			glDisable( GL_BLEND );
			sm_terrain.render_range( client, 0, idx_solid );
			glEnable( GL_BLEND );

			client.texture_mgr.bind_program( "SMShadowMapTrans" );
			glUniformMatrix4fv( idx_mat_light_trans, 1, GL_FALSE, glm::value_ptr( mat_ortho_light[ i ] ) );

			glDisable( GL_CULL_FACE );
			sm_terrain.render_range( client, idx_solid, idx_trans - idx_solid );
			glEnable( GL_CULL_FACE );

			client.entity_mgr.render( );
		//}

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		glCullFace( GL_BACK );
	}
	else { 
		glBindFramebuffer( GL_FRAMEBUFFER, id_depth_fbo );

		for( GLuint i = 0; i < num_cascades; i++ ) {
			client.texture_mgr.bind_texture( 1 + i, id_tex_depth[ i ] );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id_tex_depth[ i ], 0 );
			glClear( GL_DEPTH_BUFFER_BIT );
		}

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	client.display_mgr.resize_window( client.display_mgr.get_window ( ) );
}

void ChunkMgr::render_pass_solid( ) {
	client.texture_mgr.update_uniform( "SMTerrain", "mat_light", mat_ortho_light );
	if( is_flatshade )
		client.texture_mgr.bind_program( "SMTerrainBasic" );
	else
		client.texture_mgr.bind_program( "SMTerrain" );
	client.texture_mgr.bind_texture_array( 0, client.texture_mgr.get_texture_id( "Blocks" ) );

	//glDisable( GL_CULL_FACE );
	sm_terrain.render_range( client, 0, idx_solid );
	//glEnable( GL_CULL_FACE );
}

void ChunkMgr::render_pass_trans( ) {
	client.texture_mgr.update_uniform( "SMTerrain", "mat_light", mat_ortho_light );
	if( is_flatshade )
		client.texture_mgr.bind_program( "SMTerrainBasic" );
	else
		client.texture_mgr.bind_program( "SMTerrain" );
	client.texture_mgr.bind_texture_array( 0, client.texture_mgr.get_texture_id( "Blocks" ) );

	glDisable( GL_CULL_FACE );

	GL_CHECK( sm_terrain.render_range( client, idx_solid, idx_trans - idx_solid ) );
	//GL_CHECK( shared_mesh_inclusive.render( client ) );

	glEnable( GL_CULL_FACE );
}

void ChunkMgr::render_debug( ) {
	/*
	if( is_chunk_debug ) {
		glLoadIdentity( );
		client.display_mgr.set_camera( );

		auto & camera = client.display_mgr.camera.pos_camera;

		client.texture_mgr.bind_texture( 0, client.texture_mgr.id_materials );
		client.texture_mgr.bind_program( "BasicPersp" );
		GLuint idx_model = glGetUniformLocation( client.texture_mgr.id_bound_program, "mat_model" );

		glBindBuffer( GL_ARRAY_BUFFER, id_vbo_chunk_outline );
		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 3, GL_FLOAT, sizeof( GLfloat ) * 3, BUFFER_OFFSET( 0 ) );

		Chunk * chunk;
		std::lock_guard< std::recursive_mutex > lock( mtx_render );

		for( auto & pair_render : map_render ) {
			chunk = &pair_render.second;

			glm::mat4 mat_model = glm::translate( glm::mat4( 1.0f ), glm::vec3( chunk->pos_gw ) );
			glUniformMatrix4fv( idx_model, 1, GL_FALSE, glm::value_ptr( mat_model ) );

			glDrawArrays( GL_LINES, 0, size_chunk_outline * 2 );
		}

		glDisableClientState( GL_VERTEX_ARRAY );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );

		glDisable( GL_VERTEX_ARRAY );
	}
	*/
}

void ChunkMgr::end( ) { 
}

void ChunkMgr::sec( ) {

}

void ChunkMgr::next_skybox( ) {
	id_skybox++;
	mesh_skybox( );
}

int const level_sea = WorldSize::Chunk::size_y / 2;

GLfloat amb_light[ ] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat diff_light[ ] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat spec_light[ ] = { 0.5f, 0.5f, 0.3f, 1.0f };

GLfloat amb_mat[ ] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat diff_mat[ ] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat spec_mat[ ] = { 0.5f, 0.5f, 0.3f, 1.0f };

void ChunkMgr::init_light( ) {
	light_data.num_emitters.x = 0;
	pos_deg_light = 50;
	is_sun_pause = false;

	//glEnable( GL_LIGHT0 );
	//glMaterialf( GL_FRONT, GL_SHININESS, 20.0f );

	//glMaterialfv( GL_FRONT, GL_SPECULAR, spec_mat );
	//glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diff_light );

	//glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
	//glLightfv( GL_LIGHT0, GL_AMBIENT, diff_light );
	//glLightfv( GL_LIGHT0, GL_DIFFUSE, diff_light );
	//glLightfv( GL_LIGHT0, GL_SPECULAR, spec_light );
}

float const max_amb = 0.5f;
float const min_amb = 0.2f;

void ChunkMgr::calc_light( ) {
	float time_game = client.time_mgr.get_time( TimeStrings::GAME );
	if( !is_sun_pause ) pos_deg_light += DELTA_CORRECT;
	while( pos_deg_light >= 360 ) pos_deg_light -= 360;
	float rad_time = pos_deg_light  * PI / 180.0f;
	float light_amb = 0.0f;

	if( pos_deg_light >= 350 ) {
		light_amb = min_amb + ( pos_deg_light - 350 ) / 55 * ( max_amb - min_amb );
	}
	else if( pos_deg_light >= 190 && pos_deg_light < 350 ) {
		light_amb = min_amb;
	}
	else if( pos_deg_light >= 135 && pos_deg_light < 180 ) {
		light_amb = min_amb + ( 1 - ( pos_deg_light - 135 ) / 55 ) * ( max_amb - min_amb );
	}
	else if( pos_deg_light >= 180 && pos_deg_light < 190 ) {
		light_amb = min_amb + ( 1 - ( pos_deg_light - 135 ) / 55 ) * ( max_amb - min_amb );
	}
	else if( pos_deg_light >= 0 && pos_deg_light < 45 ) {
		light_amb = min_amb + ( 10 + pos_deg_light ) / 55 * ( max_amb - min_amb );
	}
	else {
		light_amb = max_amb;
	}

	light_data.sun_data.pos_sun = glm::vec4(
		cos( rad_time ) * dist_sun,
		sin( rad_time ) * dist_sun,
		sin( rad_time + PI / 2.0f ) * ( dist_sun / 2.0f ),
		1.0 );

	light_data.sun_data.ambient = glm::vec4( light_amb, light_amb, light_amb, 1.0f );
	light_data.sun_data.diffuse = glm::vec4( light_amb, light_amb, light_amb, 0.0f );

	GLfloat pos[ ] = { light_data.sun_data.pos_sun.x, light_data.sun_data.pos_sun.y, light_data.sun_data.pos_sun.z, 0.0 };
	glLightfv( GL_LIGHT0, GL_POSITION, pos );
	glLightfv( GL_LIGHT0, GL_AMBIENT, glm::value_ptr( light_data.sun_data.ambient ) );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, glm::value_ptr( light_data.sun_data.diffuse ) );
}

void ChunkMgr::init_skybox( ) { 
	vbo_skybox.init( );
	vbo_sun.init( );
	id_skybox = 0;
	mesh_skybox( );
}

void ChunkMgr::mesh_skybox( ) { 
	client.thread_mgr.task_main( 5, [ & ] ( ) {
		vbo_skybox.clear( );

		vbo_skybox.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicPersp",
			client.texture_mgr.get_texture_id( "Skyboxes" ),
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 } ) );

		float constexpr dx = 1.0f / 1024.0f;
		float constexpr dy = 1.0f / 1024.0f;

		static std::vector< std::string > const name_skyboxes = { 
			"Default",
			"Grimm Night",
			"Interstellar Day",
			"Interstellar Night",
			"Miramar Day",
			"Miramar Day NS",
			"Stormy Day",
			"Summer Sky",
			"The Wizard"
		};

		static glm::vec4 const color = { 0.5f, 0.5f, 0.5f, 1.0f };
		static glm::vec2 const uvs[ 4 ] = {
			{ 0.0f + dx / 2, 0.0f + dy / 2 },
			{ 1.0f - dx / 2, 0.0f + dy / 2 },
			{ 1.0f - dx / 2, 1.0f - dy / 2 },
			{ 0.0f + dx / 2, 1.0f - dy / 2 }
		};

		client.gui_mgr.print_to_console( "Setting skybox to:" + name_skyboxes[ id_skybox % name_skyboxes.size( ) ] );

		std::string name_subtex;
		GLfloat id_subtex;

		name_subtex = name_skyboxes[ id_skybox % name_skyboxes.size( ) ];
		id_subtex = ( GLfloat ) client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex + "/Front" );

		//Front
		id_subtex = client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex + "/Front" );
		vbo_skybox.push_data( { { 1, -1, 1, }, color, { 0, 0, -1 }, { uvs[ 0 ].x, uvs[ 0 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, -1, 1, }, color, { 0, 0, -1 }, { uvs[ 1 ].x, uvs[ 1 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, 1, 1, }, color, { 0, 0, -1 }, { uvs[ 2 ].x, uvs[ 2 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, 1, 1, }, color, { 0, 0, -1 }, { uvs[ 3 ].x, uvs[ 3 ].y, id_subtex } } );

		//Back
		id_subtex = client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex + "/Back" );
		vbo_skybox.push_data( { { -1, -1, -1, }, color, { 0, 0, -1 }, { uvs[ 0 ].x, uvs[ 0 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, -1, -1, }, color, { 0, 0, -1 }, { uvs[ 1 ].x, uvs[ 1 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, 1, -1, }, color, { 0, 0, -1 }, { uvs[ 2 ].x, uvs[ 2 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, 1, -1, }, color, { 0, 0, -1 }, { uvs[ 3 ].x, uvs[ 3 ].y, id_subtex } } );

		//Right
		id_subtex = client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex + "/Right" );
		vbo_skybox.push_data( { { -1, -1, 1, }, color, { 1, 0, 0 }, { uvs[ 0 ].x, uvs[ 0 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, -1, -1, }, color, { 1, 0, 0 }, { uvs[ 1 ].x, uvs[ 1 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, 1, -1, }, color, { 1, 0, 0 }, { uvs[ 2 ].x, uvs[ 2 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, 1, 1, }, color, { 1, 0, 0 }, { uvs[ 3 ].x, uvs[ 3 ].y, id_subtex } } );

		//Left
		id_subtex = client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex + "/Left" );
		vbo_skybox.push_data( { { 1, -1, -1, }, color, { -1, 0, 0 }, { uvs[ 0 ].x, uvs[ 0 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, -1, 1, }, color, { -1, 0, 0 }, { uvs[ 1 ].x, uvs[ 1 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, 1, 1, }, color, { -1, 0, 0 }, { uvs[ 2 ].x, uvs[ 2 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, 1, -1, }, color, { -1, 0, 0 }, { uvs[ 3 ].x, uvs[ 3 ].y, id_subtex } } );

		//Top
		id_subtex = client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex + "/Up" );
		vbo_skybox.push_data( { { -1, 1, 1, }, color, { 0, -1, 0 }, { uvs[ 0 ].x, uvs[ 0 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, 1, -1, }, color, { 0, -1, 0 }, { uvs[ 1 ].x, uvs[ 1 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, 1, -1, }, color, { 0, -1, 0 }, { uvs[ 2 ].x, uvs[ 2 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, 1, 1, }, color, { 0, -1, 0 }, { uvs[ 3 ].x, uvs[ 3 ].y, id_subtex } } );

		//Bot
		id_subtex = client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex + "/Down" );
		vbo_skybox.push_data( { { 1, -1, 1, }, color, { 0, 1, 0 }, { uvs[ 0 ].x, uvs[ 0 ].y, id_subtex } } );
		vbo_skybox.push_data( { { 1, -1, -1, }, color, { 0, 1, 0 }, { uvs[ 1 ].x, uvs[ 1 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, -1, -1, }, color, { 0, 1, 0 }, { uvs[ 2 ].x, uvs[ 2 ].y, id_subtex } } );
		vbo_skybox.push_data( { { -1, -1, 1, }, color, { 0, 1, 0 }, { uvs[ 3 ].x, uvs[ 3 ].y, id_subtex } } );

		vbo_skybox.finalize_set( );

		vbo_skybox.buffer( );

		vbo_sun.clear( );

		vbo_sun.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicPersp",
			client.texture_mgr.get_texture_id( "Skyboxes" ),
			std::vector< GLuint > { 0, 1, 2, 2, 3, 0 }
		) );

		name_subtex = "Sun/Basic";
		id_subtex = client.texture_mgr.get_texture_layer( "Skyboxes", name_subtex );
		vbo_sun.push_data( { { 1, -1, 0 }, color, { 0, 0, 1 }, { uvs[ 0 ].x, uvs[ 0 ].y, ( GLfloat ) id_subtex } } );
		vbo_sun.push_data( { { -1, -1, 0 }, color, { 0, 0, 1 }, { uvs[ 1 ].x, uvs[ 1 ].y, ( GLfloat ) id_subtex } } );
		vbo_sun.push_data( { { -1, 1, 0 }, color, { 0, 0, 1 }, { uvs[ 2 ].x, uvs[ 2 ].y, ( GLfloat ) id_subtex } } );
		vbo_sun.push_data( { { 1, 1, 0 }, color, { 0, 0, 1 }, { uvs[ 3 ].x, uvs[ 3 ].y, ( GLfloat ) id_subtex } } );

		vbo_sun.finalize_set( );

		vbo_sun.buffer( );
	} );
}

void ChunkMgr::init_shadowmap( ) { 
	glGenFramebuffers( 1, &id_depth_fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, id_depth_fbo );

	printf( "Creating shadowmap textures..." );
	glGenTextures( num_cascades, &id_tex_depth[ 0 ] );

	for( GLuint i = 0; i < num_cascades; ++i ) {
		client.texture_mgr.bind_texture( 1 + i, id_tex_depth[ i ] );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		GLfloat borderColor[ ] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id_tex_depth[ i ], 0 );
		glDrawBuffer( GL_NONE );
		glReadBuffer( GL_NONE );
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	GLint idx_cascades[ ] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	for( GLuint i = 0; i < num_cascades; ++i ) { 
		depth_cascades[ i ] = 0;
	}
	depth_cascades[ 0 ] = 16.0f;
	depth_cascades[ 1 ] = 64.0f;
	depth_cascades[ 2 ] = 256.0f;

	client.texture_mgr.update_uniform( "SMShadowMapSolid", "frag_sampler", ( GLint ) 0 );
	client.texture_mgr.update_uniform( "SMShadowMapTrans", "frag_sampler", ( GLint ) 0 );

	client.texture_mgr.update_uniform( "SMTerrain", "frag_shadow", idx_cascades );
	client.texture_mgr.update_uniform( "SMTerrain", "num_cascades", num_cascades );
	client.texture_mgr.update_uniform( "SMTerrain", "depth_cascades", depth_cascades );
	client.texture_mgr.update_uniform( "SMTerrain", "bias_l", 0.000005f );
	client.texture_mgr.update_uniform( "SMTerrain", "bias_h", 0.000005f );

	glActiveTexture( GL_TEXTURE0 );
}

void ChunkMgr::init_debug( ) {
	/*
	float padding = 0.1f;

	vbo_debug_chunk.init( );

	vbo_debug_chunk.push_set( VBO::IndexSet(
		VBO::TypeGeometry::TG_Lines, "BasicPersp",
		client.texture_mgr.get_texture_id( "Materials" ),
		std::vector< GLuint >{ 0, 1 }
	) );

	std::vector< glm::vec3 > temp_verts;

	temp_verts.push_back( { padding, padding, padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, padding, padding } );

	temp_verts.push_back( { padding, WorldSize::Chunk::size_y - padding, padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, WorldSize::Chunk::size_y - padding, padding } );

	temp_verts.push_back( { padding, padding, WorldSize::Chunk::size_z - padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, padding, WorldSize::Chunk::size_z - padding } );

	temp_verts.push_back( { padding, WorldSize::Chunk::size_y - padding, WorldSize::Chunk::size_z - padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, WorldSize::Chunk::size_y - padding, WorldSize::Chunk::size_z - padding } );

	temp_verts.push_back( { padding, padding, padding } );
	temp_verts.push_back( { padding, padding, WorldSize::Chunk::size_z - padding } );

	temp_verts.push_back( { padding, WorldSize::Chunk::size_y - padding, padding } );
	temp_verts.push_back( { padding, WorldSize::Chunk::size_y - padding, WorldSize::Chunk::size_z - padding } );

	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, padding, padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, padding, WorldSize::Chunk::size_z - padding } );

	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, WorldSize::Chunk::size_y - padding, padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, WorldSize::Chunk::size_y - padding, WorldSize::Chunk::size_z - padding } );

	temp_verts.push_back( { padding, padding, padding } );
	temp_verts.push_back( { padding, WorldSize::Chunk::size_y - padding, padding } );

	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, padding, padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, WorldSize::Chunk::size_y - padding, padding } );

	temp_verts.push_back( { padding, padding, WorldSize::Chunk::size_z - padding } );
	temp_verts.push_back( { padding, WorldSize::Chunk::size_y - padding, WorldSize::Chunk::size_z - padding } );

	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, padding, WorldSize::Chunk::size_z - padding } );
	temp_verts.push_back( { WorldSize::Chunk::size_x - padding, WorldSize::Chunk::size_y - padding, WorldSize::Chunk::size_z - padding } );

	vbo_debug_chunk.push_data( temp_verts );

	glBufferData( GL_ARRAY_BUFFER, size_chunk_outline * sizeof( glm::vec3 ), temp_verts.data( ), GL_STATIC_DRAW );
	*/
}

void ChunkMgr::proc_set_state( SetState & state ) {
	for( auto & pair_chunk : state.map_queue_dirty ) {
		chunk_state( pair_chunk.second, ChunkState::CS_SMesh, true );
		chunk_state( pair_chunk.second, ChunkState::CS_TMesh, true );
	}
}

void ChunkMgr::chunk_state( Chunk & chunk, ChunkState const state, bool flag ) {
	if( flag && !chunk.is_shutdown ) {
		{
			std::lock_guard< std::recursive_mutex > lock( chunk.mtx_state );
			if( !chunk.states[ state ] ) {
				chunk.cnt_states++;
				chunk.states[ state ] = true;
			}
		}
		{
			std::lock_guard < std::recursive_mutex > lock( mtx_dirty );
			map_dirty.insert( { chunk.hash_lw, chunk } );
		}
	}
	else {
		{
			std::unique_lock< std::recursive_mutex > lock( chunk.mtx_state );
			if( chunk.states[ state ] ) {
				chunk.cnt_states--;
				chunk.states[ state ] = false;
			}
		}
	}
}

void ChunkMgr::chunk_state_clear( Chunk & chunk ) { 
	std::lock_guard< std::recursive_mutex > lock( chunk.mtx_state );
	for( int i = 0; i < ChunkState::CS_Size; i++ ) { 
		chunk.states[ i ] = false;
	}
	chunk.cnt_states = 0;
}

void ChunkMgr::chunk_update( Chunk & chunk ) {
	std::lock_guard< std::recursive_mutex > lock( chunk.mtx_state );

	if( !chunk.cnt_states ||
		chunk.is_working ) {
		return;
	}

	if( chunk.is_shutdown ) {
		if( chunk.is_loaded && chunk.states[ CS_Save ] ) {
			chunk_save( chunk );
		}
		else if( chunk.states[ CS_Remove ] ) {
			chunk_remove( chunk );
		}
	}
	else if( chunk.is_loaded ) {
		if( chunk.states[ CS_Gen ] ) {
			chunk_gen( chunk );
		}
		else if( chunk.states[ CS_SBuffer ] || chunk.states[ CS_TBuffer ] ) {
			chunk_buffer( chunk );
		}
		else if( chunk.states[ CS_SMesh ] || chunk.states[ CS_TMesh ] ) {
			chunk_mesh( chunk );
		}
		else if( chunk.states[ CS_Save ] ) {
			chunk_save( chunk );
		}
	}
	else if( !chunk.is_loaded ) {
		if( chunk.states[ CS_Init ] ) {
			chunk_init( chunk );
		}
		else if( chunk.states[ CS_Read ] ) {
			chunk_read( chunk );
		}
		else if( chunk.states[ CS_Load ] ) {
			chunk_load( chunk );
		}
	}
}

void ChunkMgr::chunk_add( glm::ivec3 const & pos_lw ) {
	int hash = Directional::get_hash( pos_lw );
	Handle< Chunk > handle;
	Chunk * chunk = nullptr;

	{
		std::lock_guard< std::recursive_mutex > lock( mtx_chunks );
		auto iter_chunk = map_chunks.find( hash );
		if( iter_chunk == map_chunks.end( ) ) {
			if( client.resource_mgr.allocate( handle ) ) {
				chunk = &handle.get( );
				chunk->is_working = true;
				map_chunks.insert( { hash, handle } );
			}
			else { 
				//client.gui_mgr.print_to_console( std::string( "Error: Out of chunks!" ) );
				return;
			}
		}
	}

	if( !chunk ) {
		return;
	}

	chunk->pos_lw = pos_lw;
	chunk->hash_lw = hash;
	chunk->hash_lw_2d = Directional::get_hash( glm::ivec2( pos_lw.x, pos_lw.z ) );
	chunk->pos_gw = pos_lw * WorldSize::Chunk::vec_size;

	chunk->mat_model = glm::translate( glm::mat4( 1.0f ), glm::vec3( chunk->pos_gw ) );

	chunk->is_loaded = false;
	chunk->is_shutdown = false;

	chunk->cnt_adj = 0;
	chunk->ptr_adj = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	chunk->cnt_solid = 0;
	chunk->cnt_air = 0;

	glm::ivec3 pos_adj;
	FaceDirection face_adj;
	int hash_pos;
	Chunk * ptr_adj_chunk;
	auto iter = map_chunks.end( );

	for( int i = 0; i < FD_Size; i++ ) {
		face_adj = ( FaceDirection ) i;
		pos_adj = chunk->pos_lw + Directional::get_vec_dir_i( face_adj );

		hash_pos = Directional::get_hash( pos_adj );

		std::lock_guard< std::recursive_mutex > lock( mtx_chunks );
		iter = map_chunks.find( hash_pos );
		if( iter != map_chunks.end( ) ) {
			ptr_adj_chunk = &iter->second.get( );

			{
				std::lock_guard< std::mutex > lock( chunk->mtx_adj );

				chunk->ptr_adj[ face_adj ] = ptr_adj_chunk;
				chunk->cnt_adj += 1;
			}

			{
				std::lock_guard< std::mutex > lock( ptr_adj_chunk->mtx_adj );
				if( face_adj % 2 == 0 ) {
					ptr_adj_chunk->ptr_adj[ face_adj + 1 ] = chunk;
				}
				else {
					ptr_adj_chunk->ptr_adj[ face_adj - 1 ] = chunk;
				}
				ptr_adj_chunk->cnt_adj += 1;
			}
		}
	}
	
	sm_terrain.request_handle( chunk->handle_solid );
	sm_terrain.request_handle( chunk->handle_solid_temp );

	sm_terrain.request_handle( chunk->handle_trans );
	sm_terrain.request_handle( chunk->handle_trans_temp );

	chunk->handle_solid.release_buffer( );
	chunk->handle_solid_temp.release_buffer( );

	chunk->handle_trans.release_buffer( );
	chunk->handle_trans_temp.release_buffer( );

	chunk->ptr_noise = nullptr;
	chunk->ptr_file = nullptr;

	{
		std::unique_lock< std::recursive_mutex > lock( chunk->mtx_state );
		chunk->cnt_states = 0;
		for( int i = 0; i < ChunkState::CS_Size; i++ ) {
			chunk->states[ i ] = false;
		}
	}

	chunk_state( *chunk, ChunkState::CS_Init, true );

	chunk->is_working = false;
}

void ChunkMgr::chunk_shutdown( Chunk & chunk ) { 
	chunk_state( chunk, ChunkState::CS_Save, true );
	chunk_state( chunk, ChunkState::CS_Remove, true );
	chunk.is_shutdown = true;
}

void ChunkMgr::chunk_init( Chunk & chunk ) {
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 0;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Init, false );

		std::lock_guard< std::mutex > lock( mtx_noise );
		auto iter_noise = map_noise.find( chunk.hash_lw_2d );

		if( iter_noise == map_noise.end( ) ) {
			iter_noise = map_noise.insert( { chunk.hash_lw_2d, ChunkNoise( ) } ).first;

			chunk.ptr_noise = &iter_noise->second;

			//float noise_biome;
			float noise, noise_lerp, noise_total;

			auto & biome_base = list_biomes[ 0 ];

			for( int i = 0; i < WorldSize::Chunk::size_x; i++ ) {
				for( int j = 0; j < WorldSize::Chunk::size_z; j++ ) {
					noise_total = 0;

					for( auto & layer_noise : biome_base.list_noise ) { 
						noise = raw_noise_2d( 
							layer_noise.seed_x + ( chunk.pos_gw.x + i ) * layer_noise.scale_x,
							layer_noise.seed_y + ( chunk.pos_gw.z + j ) * layer_noise.scale_y ) *
							layer_noise.scale_height;

						for( auto & pair_bounds : layer_noise.list_bounds ) { 
							if( noise >= pair_bounds.first &&
								noise <= pair_bounds.second ) {

								noise += layer_noise.height_base;
								if( noise < layer_noise.height_min ) { noise = layer_noise.height_min; }
								if( noise > layer_noise.height_max ) { noise = layer_noise.height_max; }

								noise_total += noise;

								break;
							}
						}
					}

					chunk.ptr_noise->biome[ i ][ j ] = 0;
					chunk.ptr_noise->height[ i ][ j ] = noise_total;

					for( GLuint idx_biome = 1; idx_biome < list_biomes.size( ); ++idx_biome ) {
						auto & biome = list_biomes[ idx_biome ];
						auto & layer_biome = biome.noise_biome;

						noise = raw_noise_2d(
							layer_biome.seed_x + ( chunk.pos_gw.x + i ) * layer_biome.scale_x,
							layer_biome.seed_y + ( chunk.pos_gw.z + j ) * layer_biome.scale_y ) *
							layer_biome.scale_height;

						for( auto & pair_bounds : layer_biome.list_bounds ) { 
							if( noise >= pair_bounds.first &&
								noise <= pair_bounds.second ) {

								chunk.ptr_noise->biome[ i ][ j ] = idx_biome;
								break;
							}
						}

						if( chunk.ptr_noise->biome[ i ][ j ] == idx_biome ) { 
							noise_total = 0;


							for( auto & layer_height : biome.list_noise ) { 
								noise = raw_noise_2d(
									layer_height.seed_x + ( chunk.pos_gw.x + i ) * layer_height.scale_x,
									layer_height.seed_y + ( chunk.pos_gw.z + j ) * layer_height.scale_y ) *
									layer_height.scale_height;

								for( auto & pair_bounds : layer_height.list_bounds ) { 
									if( noise >= pair_bounds.first &&
										noise <= pair_bounds.second ) {
									
										noise += layer_height.height_base;
										if( noise < layer_height.height_min ) { noise = layer_height.height_min; }
										if( noise > layer_height.height_max ) { noise = layer_height.height_max; }

										noise_total += noise;
										break;
									}
								}
							}

							auto & layer_lerp = biome.noise_lerp;
							noise_lerp = 1.0f;

							noise = raw_noise_2d(
								layer_lerp.seed_x + ( chunk.pos_gw.x + i ) * layer_lerp.scale_x,
								layer_lerp.seed_y + ( chunk.pos_gw.z + j ) * layer_lerp.scale_y ) *
								layer_lerp.scale_height;

							for( auto & pair_bounds : layer_lerp.list_bounds ) { 
								if( noise >= pair_bounds.first &&
									noise <= pair_bounds.second ) {

									noise_lerp = ( noise - pair_bounds.first ) / ( pair_bounds.second - pair_bounds.first );

									break;
								}
							}

							chunk.ptr_noise->height[ i ][ j ] = chunk.ptr_noise->height[ i ][ j ] * ( 1.0f - noise_lerp ) + noise_lerp * noise_total;
						}
					}

					noise = raw_noise_2d(
						( 400.0f + chunk.pos_gw.x + i ) * 0.01f,
						( 400.0f + chunk.pos_gw.z + j ) * 0.01f ) * 5.0f;

					chunk.ptr_noise->envir[ i ][ j ] = noise;
				}
			}
		}
		else { 
			chunk.ptr_noise = &iter_noise->second;
		}

		chunk.ptr_noise->cnt_using += 1;

		chunk_state( chunk, ChunkState::CS_Read, true );
		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_read( Chunk & chunk ) { 
	using namespace std::tr2::sys;
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 0;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_io( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Read, false );

		glm::ivec3 pos_r, pos_lr;
		Directional::pos_lw_to_r( chunk.pos_lw, pos_r );
		Directional::pos_lw_to_lr( chunk.pos_lw, pos_lr );

		int pos_hash = Directional::get_hash( pos_r );
		{
			std::lock_guard< std::mutex > lock( mtx_file );
			auto iter_file = map_file.find( pos_hash );

			if( iter_file == map_file.end( ) ) {
				iter_file = map_file.insert( { pos_hash, ChunkFile( ) } ).first;
				auto & file = iter_file->second;

				std::ostringstream path_region;
				path_region << "./World/" << pos_r.x << "_" << pos_r.y << "_" << pos_r.z << ".chk";

				if( !exists( path_region.str( ) ) ) {
					file.file_header.idx_last = 0;
					for( int i = 0; i < FileHeader::num_regions; i++ ) {
						fill_array( file.file_header.array_index[ i ].index_section, -1 );
					}

					file.file_stream.open( path_region.str( ), std::fstream::out );
					file.file_stream.write( ( char* ) &file.file_header, sizeof( FileHeader ) );
					file.file_stream.close( );
					file.file_stream.open( path_region.str( ), std::fstream::out | std::fstream::in | std::fstream::binary );
				}
				else {
					file.file_stream.open( path_region.str( ), std::fstream::out | std::fstream::in | std::fstream::binary );
					file.file_stream.read( ( char * ) &file.file_header, sizeof( FileHeader ) );
				}

				file.cnt_using = 0;
			}

			chunk.ptr_file = &iter_file->second;

			chunk.ptr_file->cnt_using++;

			int index_region =
				WorldSize::Region::size_x * WorldSize::Region::size_z * pos_lr.y +
				WorldSize::Region::size_x * pos_lr.z +
				pos_lr.x;

			int index_section;
			int index_start;
			int num_block, id;
			int index_buffer;

			num_block = 0;
			index_buffer = 0;
			index_section = 0;

			index_start = chunk.ptr_file->file_header.array_index[ index_region ].index_section[ index_section ];

			if( index_start != -1 ) {
				chunk.ptr_file->file_stream.seekg( sizeof( FileHeader ) + index_start );
				chunk.ptr_file->file_stream.read( ( char * ) & buffer_section, SectionIndex::size_section_bytes );

				num_block = buffer_section[ index_buffer ];
				id = buffer_section[ index_buffer + 1 ];

				if( id != -1 ) {
					chunk.cnt_solid += num_block;
				}
				else {
					chunk.cnt_air += num_block;
				}

				for( int i = 0; i < WorldSize::Chunk::size_y; i++ ) {
					for( int j = 0; j < WorldSize::Chunk::size_z; j++ ) {
						for( int k = 0; k < WorldSize::Chunk::size_x; k++ ) {
							if( num_block == 0 ) {
								index_buffer += 2;
								num_block = buffer_section[ index_buffer ];
								id = buffer_section[ index_buffer + 1 ];

								if( id != -1 ) { 
									chunk.cnt_solid += num_block;
								}
								else { 
									chunk.cnt_air += num_block;
								}
							}

							if( index_buffer >= size_buffer ) {
								index_section++;

								index_start = chunk.ptr_file->file_header.array_index[ index_region ].index_section[ index_section ];

								chunk.ptr_file->file_stream.seekg( sizeof( FileHeader ) + index_start );
								chunk.ptr_file->file_stream.read( ( char * ) & buffer_section, SectionIndex::size_section_bytes );

								index_buffer = 0;

								num_block = buffer_section[ index_buffer ];
								id = buffer_section[ index_buffer + 1 ];
							}

							chunk.id_blocks[ k ][ i ][ j ] = id;
							num_block--;
						}
					}
				}

				chunk.is_loaded = true;
			}
		}

		if( chunk.is_loaded ) {
			{
				std::lock_guard< std::mutex > lock( chunk.mtx_adj );
				for( int i = 0; i < FD_Size; i++ ) {
					if( chunk.ptr_adj[ i ] != nullptr ) {
						chunk_state( *chunk.ptr_adj[ i ], ChunkState::CS_SMesh, true );
						chunk_state( *chunk.ptr_adj[ i ], ChunkState::CS_TMesh, true );
					}
				}
			}

			chunk_state( chunk, ChunkState::CS_SMesh, true );
			chunk_state( chunk, ChunkState::CS_TMesh, true );
		}
		else {
			chunk_state( chunk, ChunkState::CS_Load, true );
		}

		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_load( Chunk & chunk ) { 
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 0;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Load, false );

		Block & block_water = get_block_data( "Water" );
		Block & block_cobble = get_block_data( "Cobblestone" );

		int noise_height = 0;
		int noise_biome = 0;

		float noise_cobble;

		for( int i = 0; i < WorldSize::Chunk::size_x; i++ ) {
			for( int k = 0; k < WorldSize::Chunk::size_z; k++ ) {
				noise_height = chunk.ptr_noise->height[ i ][ k ];
				noise_biome = chunk.ptr_noise->biome[ i ][ k ];
				auto & biome = list_biomes[ noise_biome ];
				noise_cobble = 10 + raw_noise_2d( ( chunk.pos_gw.x + i ) * 0.01f, ( chunk.pos_gw.z + k ) * 0.01f ) * 5.0f;

				for( int j = 0; j < WorldSize::Chunk::size_y; j++ ) {
					if( chunk.pos_gw.y + j < noise_height ) {
						if( chunk.pos_gw.y + j < noise_height - noise_cobble ) {
							chunk.id_blocks[ i ][ j ][ k ] = block_cobble.id;
						}
						else {
							chunk.id_blocks[ i ][ j ][ k ] = biome.id_block_depth;
						}
					}
					else if( chunk.pos_gw.y + j == noise_height ) {
						chunk.id_blocks[ i ][ j ][ k ] = biome.id_block_surface;
					}
					else {
						if( chunk.pos_gw.y + j <= WorldSize::World::level_sea &&
							noise_biome != map_biome_name[ "Cobblestone Chasm" ] ) {
							chunk.id_blocks[ i ][ j ][ k ] = block_water.id;
						}
						else {
							chunk.id_blocks[ i ][ j ][ k ] = -1;
						}
					}

					if( chunk.id_blocks[ i ][ j ][ k ] != -1 ) { 
						chunk.cnt_solid += 1;
					}
					else { 
						chunk.cnt_air += 1;
					}
				}
			}
		}

		chunk.is_loaded = true;

		if( chunk.cnt_solid != 0 )
			chunk_state( chunk, ChunkState::CS_Gen, true );

		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_gen( Chunk & chunk ) { 
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 0;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Gen, false );

		SetState state;
		state.iter = map_chunks.end( );
		auto & block_pumpkin = get_block_data( "Pumpkin" );
		auto & block_gleaves = get_block_data( "Leaves Light Green" );
		auto & block_dgleaves = get_block_data( "Leaves Green" );
		auto & block_rleaves = get_block_data( "Leaves Red" );
		auto & block_bleaves = get_block_data( "Leaves Brown" );
		auto & block_grass_blade = get_block_data( "Grass Blade" );

		auto & biome_base = map_biome_name[ "Base" ];
		auto & biome_grass_hill = map_biome_name[ "Grass Hill" ];
		auto & biome_grass_plateau = map_biome_name[ "Grass Plateau" ];

		for( int i = 0; i < WorldSize::Chunk::size_x; i++ ) {
			for( int j = 0; j < WorldSize::Chunk::size_z; j++ ) {
				if( chunk.ptr_noise->height[ i ][ j ] >= WorldSize::World::level_sea &&
					chunk.ptr_noise->height[ i ][ j ] >= chunk.pos_gw.y &&
					chunk.ptr_noise->height[ i ][ j ] < chunk.pos_gw.y + WorldSize::Chunk::size_y ) {

					if( chunk.ptr_noise->biome[ i ][ j ] == biome_base ||
						chunk.ptr_noise->biome[ i ][ j ] == biome_grass_hill ||
						chunk.ptr_noise->biome[ i ][ j ] == biome_grass_plateau ) {
						if( rand( ) % 1000 < 10 ) {
							set_tree(
								glm::ivec3( chunk.pos_gw.x, 0, chunk.pos_gw.z ) +
								glm::ivec3( i, chunk.ptr_noise->height[ i ][ j ] + 1, j ),
								state, 
								rand( ) % 3 );
						}
						else if( rand( ) % 1000 < 10 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_gleaves.id );
						}
						else if( rand( ) % 1000 < 10 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_dgleaves.id );
						}
						else if( rand( ) % 1000 < 10 ) {
							set_block( 
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_rleaves.id );
						}
						else if( chunk.ptr_noise->envir[ i ][ j ] > 0 && rand( ) % 1000 < chunk.ptr_noise->envir[ i ][ j ] * 100 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_grass_blade.id );
						}
					}
					else if( chunk.ptr_noise->biome[ i ][ j ] == map_biome_name[ "Sand Plateau" ] ) {
						if( rand( ) % 1000 < 10 ) {
							set_tree(
								glm::ivec3( chunk.pos_gw.x, 0, chunk.pos_gw.z ) +
								glm::ivec3( i, chunk.ptr_noise->height[ i ][ j ] + 1, j ),
								state, 
								3 + rand( ) % 2 );
						}
						else if( rand( ) % 1000 < 10 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_pumpkin.id );
						}
						else if( rand( ) % 1000 < 10 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_bleaves.id );
						}
					}
				}
			}
		}

		proc_set_state( state );

		{
			std::lock_guard< std::mutex > lock( chunk.mtx_adj );
			for( int i = 0; i < FD_Size; i++ ) {
				if( chunk.ptr_adj[ i ] != nullptr ) {
					chunk_state( *chunk.ptr_adj[ i ], ChunkState::CS_SMesh, true );
					chunk_state( *chunk.ptr_adj[ i ], ChunkState::CS_TMesh, true );
				}
			}
		}

		chunk_state( chunk, ChunkState::CS_SMesh, true );
		chunk_state( chunk, ChunkState::CS_TMesh, true );
		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_mesh( Chunk & chunk ) {
	chunk.is_working = true;

	if( ( chunk.states[ ChunkState::CS_SMesh ] && !chunk.handle_solid_temp.request_buffer( ) ) &&
		( chunk.states[ ChunkState::CS_TMesh ] && !chunk.handle_trans_temp.request_buffer( ) ) ) {
		chunk.is_working = false;
		return;
	}
	else if( ( chunk.states[ ChunkState::CS_SMesh ] && !chunk.handle_solid_temp.request_buffer( ) ) &&
		!chunk.states[ ChunkState::CS_TMesh ] ) {
		chunk.is_working = false;
		return;
	}
	else if( ( chunk.states[ ChunkState::CS_TMesh ] && !chunk.handle_trans_temp.request_buffer( ) ) &&
		!chunk.states[ ChunkState::CS_SMesh ] ) {
		chunk.is_working = false;
		return;
	}

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 3;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		static FaceDirection list_face_z[ ] = { FD_Up, FD_Down, FD_Left, FD_Right };
		static FaceDirection list_face_x[ ] = { FD_Front, FD_Back };
		static glm::vec3 list_scale_verts[ ] = {
			{ 1, 0, 0 }, { 1, 0, 0 },	// Front / Back
			{ 0, 0, 1 }, { 0, 0, 1 },	// Left / Right
			{ 0, 0, 1 }, { 0, 0, 1 }	// Up / Down
		};
		static glm::vec2 list_scale_uvs[ ] = {
			{ 1, 0 }, { 1, 0 },		// Front / Back
			{ 1, 0 }, { 1, 0 },		// Left / Right
			{ 0, 1 }, { 0, 1 }		// Up / Down
		};

		glm::ivec3 pos_curr;
		glm::ivec3 pos_curr_x;
		glm::ivec3 pos_adj;
		glm::ivec3 pos_min( 0, 0, 0 );
		glm::ivec3 pos_max( WorldSize::Chunk::size_x - 1, WorldSize::Chunk::size_y - 1, WorldSize::Chunk::size_z - 1 );

		int id_curr, id_adj;

		std::array< glm::ivec3, FaceDirection::FD_Size > list_pos_last;
		std::array< std::pair< int, int >, FaceDirection::FD_Size > list_id_last;
		std::array< int, FaceDirection::FD_Size > list_cnt_last;
		std::array< Block *, FaceDirection::FD_Size > list_block_last;

		FaceDirection dir_face;

		Block * block_curr = nullptr;
		Block * block_adj = nullptr;

		Chunk * chunk_adj;

		if( chunk.states[ ChunkState::CS_SMesh ] && chunk.handle_solid_temp.ptr_buffer ) {
			chunk_state( chunk, ChunkState::CS_SMesh, false );

			chunk.list_incl_solid.clear( );

			chunk.handle_solid_temp.clear( );
			chunk.handle_solid_temp.ptr_buffer->list_verts.clear( );
			chunk.handle_solid_temp.ptr_buffer->list_inds.clear( );

			chunk.handle_solid_temp.push_set( SMTerrain::SMTGSet(
				SMTerrain::TypeGeometry::TG_Triangles,
				client.texture_mgr.get_program_id( "SMTerrain" ),
				client.texture_mgr.get_texture_id( "Blocks" )
			) );

			for( int iter_face = 0; iter_face < FaceDirection::FD_Size; iter_face++ ) {
				dir_face = ( FaceDirection ) iter_face;
				list_id_last[ dir_face ] = { -1, -1 };
				list_cnt_last[ dir_face ] = 0;
				list_block_last[ dir_face ] = nullptr;
			}

			for( pos_curr.x = 0; pos_curr.x < WorldSize::Chunk::size_x; pos_curr.x++ ) {
				for( pos_curr.y = 0; pos_curr.y < WorldSize::Chunk::size_y; pos_curr.y++ ) {
					for( pos_curr.z = 0; pos_curr.z < WorldSize::Chunk::size_z; pos_curr.z++ ) {
						id_curr = chunk.id_blocks[ pos_curr.x ][ pos_curr.y ][ pos_curr.z ];

						if( id_curr == -1 || id_curr == -2 ) {
							// Is air, skip
							for( FaceDirection iter_face : list_face_z ) {
								list_id_last[ iter_face ].first = -1;
							}

							goto list_x_solid;
						}

						block_curr = &get_block_data( id_curr );

						if( block_curr->is_trans ) {
							for( FaceDirection iter_face : list_face_z ) {
								list_id_last[ iter_face ].first = -1;
							}

							goto list_x_solid;
						}

						// Add the included faces
						if( block_curr->include_lookup.size( ) ) { 
							chunk.list_incl_solid.push_back( { 0, id_curr, pos_curr + chunk.pos_gw } );
						}
						
						// Start List z
						for( auto iter_face : list_face_z ) {
							dir_face = iter_face;

							pos_adj = pos_curr + Directional::get_vec_dir_i( dir_face );
							chunk_adj = &chunk;

							if( !Directional::is_point_in_region( pos_adj, pos_min, pos_max ) ) {
								pos_adj -= Directional::get_vec_dir_i( dir_face )  * WorldSize::Chunk::vec_size;

								std::lock_guard< std::mutex > lock( chunk.mtx_adj );
								chunk_adj = chunk.ptr_adj[ dir_face ];

								if( chunk_adj == nullptr || !chunk_adj->is_loaded ) {
									// Chunk Aint loaded
									list_id_last[ dir_face ].first = -1;

									continue;
								}
							}

							id_adj = chunk_adj->id_blocks[ pos_adj.x ][ pos_adj.y ][ pos_adj.z ];

							if( id_adj == -1 ) {
								if( id_curr == list_id_last[ dir_face ].first ) {
									// We have the same face in a row...
									list_cnt_last[ dir_face ]++;
								}
								else {
									// New face! Lets record some info...
									if( list_cnt_last[ dir_face ] && list_block_last[ dir_face ]->occlude_lookup[ dir_face ].size( ) != 0 ) {
										put_face(
											chunk.handle_solid_temp,
											list_pos_last[ dir_face ], list_block_last[ dir_face ]->color,
											dir_face,
											list_block_last[ dir_face ]->faces[ list_block_last[ dir_face ]->occlude_lookup[ dir_face ][ 0 ] ],
											glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
											glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
									}

									list_id_last[ dir_face ] = { id_curr, id_curr };
									list_pos_last[ dir_face ] = pos_curr;
									list_block_last[ dir_face ] = block_curr;
									list_cnt_last[ dir_face ] = 1;
								}

								continue;
							}
							else {
								block_adj = &get_block_data( id_adj );

								if( block_adj->occlude_lookup[ Directional::get_face_opposite( dir_face ) ].size( ) == 0 ||
									block_curr->is_visible( *block_adj ) ) {
									// A face is visible!

									if( id_curr == list_id_last[ dir_face ].first ) {
										// We have the same face in a row...
										list_cnt_last[ dir_face ]++;
									}
									else {
										// New face! Lets record some info...
										if( list_cnt_last[ dir_face ] && list_block_last[ dir_face ]->occlude_lookup[ dir_face ].size( ) != 0 ) {
											put_face(
												chunk.handle_solid_temp,
												list_pos_last[ dir_face ], list_block_last[ dir_face ]->color,
												dir_face,
												list_block_last[ dir_face ]->faces[ list_block_last[ dir_face ]->occlude_lookup[ dir_face ][ 0 ] ],
												glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
												glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
										}

										list_id_last[ dir_face ] = { id_curr, id_curr };
										list_pos_last[ dir_face ] = pos_curr;
										list_block_last[ dir_face ] = block_curr;
										list_cnt_last[ dir_face ] = 1;
									}

									continue;
								}
							}

							list_id_last[ dir_face ].first = -1;
						}

						list_x_solid: // Start list x

						pos_curr_x = { pos_curr.z, pos_curr.y, pos_curr.x };
						id_curr = chunk.id_blocks[ pos_curr_x.x ][ pos_curr_x.y ][ pos_curr_x.z ];

						if( id_curr == -1 || id_curr == -2 ) {
							// Is air, skip
							for( FaceDirection iter_face : list_face_x ) {
								list_id_last[ iter_face ].first = -1;
							}

							continue;
						}

						block_curr = &get_block_data( id_curr );

						if( block_curr->is_trans ) {
							for( FaceDirection iter_face : list_face_x ) {
								list_id_last[ iter_face ].first = -1;
							}

							continue;
						}

						for( auto iter_face : list_face_x ) {
							dir_face = iter_face;

							pos_adj = pos_curr_x + Directional::get_vec_dir_i( dir_face );
							chunk_adj = &chunk;

							if( !Directional::is_point_in_region( pos_adj, pos_min, pos_max ) ) {
								pos_adj -= Directional::get_vec_dir_i( dir_face )  * WorldSize::Chunk::vec_size;

								std::lock_guard< std::mutex > lock( chunk.mtx_adj );
								chunk_adj = chunk.ptr_adj[ dir_face ];

								if( chunk_adj == nullptr || !chunk_adj->is_loaded ) {
									// Chunk Aint loaded
									list_id_last[ dir_face ].first = -1;

									continue;
								}
							}

							id_adj = chunk_adj->id_blocks[ pos_adj.x ][ pos_adj.y ][ pos_adj.z ];

							if( id_adj == -1 ) {
								if( id_curr == list_id_last[ dir_face ].first ) {
									// We have the same face in a row...
									list_cnt_last[ dir_face ]++;
								}
								else {
									// New face! Lets record some info...
									if( list_cnt_last[ dir_face ] && list_block_last[ dir_face ]->occlude_lookup[ dir_face ].size( ) != 0 ) {
										put_face(
											chunk.handle_solid_temp,
											list_pos_last[ dir_face ], list_block_last[ dir_face ]->color,
											dir_face,
											list_block_last[ dir_face ]->faces[ list_block_last[ dir_face ]->occlude_lookup[ dir_face ][ 0 ] ],
											glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
											glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
									}

									list_id_last[ dir_face ] = { id_curr, id_curr };
									list_pos_last[ dir_face ] = pos_curr_x;
									list_block_last[ dir_face ] = block_curr;
									list_cnt_last[ dir_face ] = 1;
								}

								continue;
							}
							else {
								block_adj = &get_block_data( id_adj );

								if( block_adj->occlude_lookup[ Directional::get_face_opposite( dir_face ) ].size( ) == 0 ||
									block_curr->is_visible( *block_adj ) ) {
									// A face is visible!

									if( id_curr == list_id_last[ dir_face ].first ) {
										// We have the same face in a row...
										list_cnt_last[ dir_face ]++;
									}
									else {
										// New face! Lets record some info...
										if( list_cnt_last[ dir_face ] && list_block_last[ dir_face ]->occlude_lookup[ dir_face ].size( ) != 0 ) {
											put_face(
												chunk.handle_solid_temp,
												list_pos_last[ dir_face ], list_block_last[ dir_face ]->color,
												dir_face,
												list_block_last[ dir_face ]->faces[ list_block_last[ dir_face ]->occlude_lookup[ dir_face ][ 0 ] ],
												glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
												glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
										}

										list_id_last[ dir_face ] = { id_curr, id_curr };
										list_pos_last[ dir_face ] = pos_curr_x;
										list_block_last[ dir_face ] = block_curr;
										list_cnt_last[ dir_face ] = 1;
									}

									continue;
								}
							}

							list_id_last[ dir_face ].first = -1;
						}
					}
					// Woooo we made it through a row
					for( int iter_face = 0; iter_face < FaceDirection::FD_Size; iter_face++ ) {
						dir_face = ( FaceDirection ) iter_face;
						list_id_last[ dir_face ].first = -1;
					}
				}
			}

			for( int iter_face = 0; iter_face < FaceDirection::FD_Size; iter_face++ ) {
				dir_face = ( FaceDirection ) iter_face;
				list_id_last[ dir_face ].first = -1;
				if( list_cnt_last[ dir_face ] && list_block_last[ dir_face ]->occlude_lookup[ dir_face ].size( ) != 0 ) {
					put_face(
						chunk.handle_solid_temp,
						list_pos_last[ dir_face ], list_block_last[ dir_face ]->color,
						dir_face,
						list_block_last[ dir_face ]->faces[ list_block_last[ dir_face ]->occlude_lookup[ dir_face ][ 0 ] ],
						glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
						glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
				}
			}

			chunk.handle_solid_temp.finalize_set( );

			if( chunk.handle_solid_temp.get_size_ibo( ) ) {
				chunk_state( chunk, ChunkState::CS_SBuffer, true );

				std::unique_lock< std::recursive_mutex > lock( mtx_render );
				map_render.insert( { chunk.hash_lw, chunk } );
			}
			else { 
				chunk.handle_solid_temp.clear( );
				chunk.handle_solid_temp.release_buffer( );
			
				chunk.handle_solid.swap_handle( chunk.handle_solid_temp );

				chunk.handle_solid_temp.clear( );
			}
		}
		
		if( chunk.states[ ChunkState::CS_TMesh ] && chunk.handle_trans_temp.ptr_buffer ) {
			chunk_state( chunk, ChunkState::CS_TMesh, false );

			chunk.list_incl_trans.clear( );

			chunk.handle_trans_temp.clear( );

			auto & buffer = *chunk.handle_trans_temp.ptr_buffer;

			buffer.list_verts.clear( );
			buffer.list_inds.clear( );
			buffer.list_temp.clear( );
			buffer.list_sort.clear( );

			chunk.handle_trans_temp.push_set( SMTerrain::SMTGSet(
				SMTerrain::TypeGeometry::TG_Triangles,
				client.texture_mgr.get_program_id( "SMTerrain" ),
				client.texture_mgr.get_texture_id( "Blocks" )
			) );

			for( pos_curr.x = 0; pos_curr.x < WorldSize::Chunk::size_x; pos_curr.x++ ) {
				for( pos_curr.y = 0; pos_curr.y < WorldSize::Chunk::size_y; pos_curr.y++ ) {
					for( pos_curr.z = 0; pos_curr.z < WorldSize::Chunk::size_z; pos_curr.z++ ) {
						id_curr = chunk.id_blocks[ pos_curr.x ][ pos_curr.y ][ pos_curr.z ];

						if( id_curr == -1 ) {
							continue;
						}

						block_curr = &get_block_data( id_curr );

						if( !block_curr->is_trans ) {
							continue;
						}

						// Add the included faces
						if( block_curr->include_lookup.size( ) ) {
							chunk.list_incl_trans.push_back( { 0, id_curr, pos_curr + chunk.pos_gw } );
						}

						// Add the excluded faces
						for( GLuint i = 0; i < FaceDirection::FD_Size; ++i ) {
							dir_face = ( FaceDirection ) i;

							pos_adj = pos_curr + Directional::get_vec_dir_i( dir_face );
							chunk_adj = &chunk;

							if( !Directional::is_point_in_region( pos_adj, pos_min, pos_max ) ) {
								pos_adj -= Directional::get_vec_dir_i( dir_face )  * WorldSize::Chunk::vec_size;

								std::lock_guard< std::mutex > lock( chunk.mtx_adj );
								chunk_adj = chunk.ptr_adj[ dir_face ];

								if( chunk_adj == nullptr || !chunk_adj->is_loaded ) {
									continue;
								}
							}

							id_adj = chunk_adj->id_blocks[ pos_adj.x ][ pos_adj.y ][ pos_adj.z ];

							if( !block_curr->occlude_lookup[ dir_face ].size( ) ) { 
								continue;
							}

							if( id_adj == -1 ) {
								put_face(
									chunk.handle_trans_temp,
									pos_curr, block_curr->color,
									dir_face,
									block_curr->faces[ block_curr->occlude_lookup[ dir_face ][ 0 ] ] );

								put_sort( buffer.list_sort, glm::vec3( chunk.pos_gw + pos_curr ),
									*block_curr, ( FaceDirection ) dir_face );
							}
							else {
								block_adj = &get_block_data( id_adj );

								if( block_adj->occlude_lookup[ Directional::get_face_opposite( dir_face ) ].size( ) == 0 ||
									block_curr->is_visible( *block_adj ) ) {

									put_face(
										chunk.handle_trans_temp,
										pos_curr, block_curr->color,
										dir_face,
										block_curr->faces[ block_curr->occlude_lookup[ dir_face ][ 0 ] ] );

									put_sort( buffer.list_sort, glm::vec3( chunk.pos_gw + pos_curr ),
										*block_curr, ( FaceDirection ) dir_face );
								}
							}
						}
					}
				}
			}

			std::sort(
				buffer.list_sort.begin( ), buffer.list_sort.end( ),
				[ ] ( std::pair< float, GLuint > const & lho, std::pair< float, GLuint > const & rho ) {
					return lho.first > rho.first;
				}
			);


			buffer.list_temp.reserve( buffer.list_inds.size( ) );

			for( GLuint i = 0; i < buffer.list_sort.size( ); ++i ) {
				buffer.list_temp.insert( buffer.list_temp.end( ),
					std::make_move_iterator( buffer.list_inds.begin( ) + buffer.list_sort[ i ].second * 6 ),
					std::make_move_iterator( buffer.list_inds.begin( ) + buffer.list_sort[ i ].second * 6 + 6 )
				);
			}

			buffer.list_inds = std::move( buffer.list_temp );

			chunk.handle_trans_temp.finalize_set( );

			if( chunk.handle_trans_temp.get_size_ibo( ) ) {
				chunk_state( chunk, ChunkState::CS_TBuffer, true );

				std::unique_lock< std::recursive_mutex > lock( mtx_render );
				map_render.insert( { chunk.hash_lw, chunk } );
			}
			else {
				chunk.handle_trans_temp.clear( );
				chunk.handle_trans_temp.release_buffer( );

				chunk.handle_trans.swap_handle( chunk.handle_trans_temp );

				chunk.handle_trans_temp.clear( );
			}
		}

		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_buffer( Chunk & chunk ) {
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 3;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_main( priority, [ & ] ( ) {
		if( chunk.states[ ChunkState::CS_SBuffer ] ) {
			chunk_state( chunk, ChunkState::CS_SBuffer, false );

			chunk.handle_solid_temp.submit_buffer( );
			chunk.handle_solid_temp.release_buffer( );

			chunk.handle_solid.swap_handle( chunk.handle_solid_temp );

			chunk.handle_solid_temp.clear( );
		}

		if( chunk.states[ ChunkState::CS_TBuffer ] ) {
			chunk_state( chunk, ChunkState::CS_TBuffer, false );

			chunk.handle_trans_temp.submit_buffer( );
			chunk.handle_trans_temp.release_buffer( );

			chunk.handle_trans.swap_handle( chunk.handle_trans_temp );

			chunk.handle_trans_temp.clear( );
		}

		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_save( Chunk & chunk ) {
	using namespace std::tr2::sys;
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 2;
	priority = priority + ( float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_io( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Save, false );

		glm::ivec3 pos_lr;
		Directional::pos_lw_to_lr( chunk.pos_lw, pos_lr );

		{
			//client.time_mgr.begin_record( std::string( "SAVE" ) );
			std::lock_guard< std::mutex > lock( mtx_file );

			int index_region = WorldSize::Region::size_x * WorldSize::Region::size_z * pos_lr.y +
				WorldSize::Region::size_x * pos_lr.z + pos_lr.x;

			int index_section;
			int index_start;
			int num_block, id;
			int index_buffer;

			id = chunk.id_blocks[ 0 ][ 0 ][ 0 ];
			num_block = 0;
			index_buffer = 0;
			index_section = 0;

			for( int i = 0; i < WorldSize::Chunk::size_y; i++ ) {
				for( int j = 0; j < WorldSize::Chunk::size_z; j++ ) {
					for( int k = 0; k < WorldSize::Chunk::size_x; k++ ) {
						if( id == chunk.id_blocks[ k ][ i ][ j ] ) {
							num_block++;
						}
						else {
							buffer_section[ index_buffer ] = num_block;
							buffer_section[ index_buffer + 1 ] = id;
							index_buffer += 2;

							num_block = 1;
							id = chunk.id_blocks[ k ][ i ][ j ];

							if( index_buffer >= size_buffer ) {
								index_start = chunk.ptr_file->file_header.array_index[ index_region ].index_section[ index_section ];

								if( index_start == -1 ) {
									index_start = chunk.ptr_file->file_header.idx_last;
									chunk.ptr_file->file_header.array_index[ index_region ].index_section[ index_section ] = index_start;
									chunk.ptr_file->file_header.idx_last += SectionIndex::size_section_bytes;
								}

								chunk.ptr_file->file_stream.seekp( sizeof( FileHeader ) + index_start );
								chunk.ptr_file->file_stream.write( ( char * ) &buffer_section, SectionIndex::size_section_bytes );

								index_section++;
								index_buffer = 0;
							}
						}
					}
				}
			}

			if( num_block != 0 || index_buffer != 0 ) {
				buffer_section[ index_buffer ] = num_block;
				buffer_section[ index_buffer + 1 ] = id;
				index_buffer += 2;

				index_start = chunk.ptr_file->file_header.array_index[ index_region ].index_section[ index_section ];

				if( index_start == -1 ) {
					index_start = chunk.ptr_file->file_header.idx_last;
					chunk.ptr_file->file_header.array_index[ index_region ].index_section[ index_section ] = index_start;
					chunk.ptr_file->file_header.idx_last += SectionIndex::size_section_bytes;
				}

				chunk.ptr_file->file_stream.seekp( sizeof( FileHeader ) + index_start );
				chunk.ptr_file->file_stream.write( ( char * ) &buffer_section, SectionIndex::size_section_bytes );
			}

			chunk.ptr_file->file_stream.seekp( sizeof( FileHeader::array_index ) );
			chunk.ptr_file->file_stream.write( ( char * ) &chunk.ptr_file->file_header.idx_last, sizeof( int ) );

			chunk.ptr_file->file_stream.seekp( sizeof( SectionIndex ) * index_region );
			chunk.ptr_file->file_stream.write( ( char * ) &chunk.ptr_file->file_header.array_index[ index_region ], sizeof( SectionIndex ) );
		}

		chunk.is_working = false;

		//client.time_mgr.end_record( std::string( "SAVE" ) );
		//client.time_mgr.push_record( std::string( "SAVE" ) );
	} );
}

void ChunkMgr::chunk_remove( Chunk & chunk ) { 
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 2;
	priority = priority + ( float( max_dir ) / Directional::get_max( WorldSize::World::vec_size ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Remove, false );

		{
			std::lock_guard< std::mutex > lock( mtx_noise );

			auto iter = map_noise.find( chunk.hash_lw_2d );
			if( iter != map_noise.end( ) ) {
				iter->second.cnt_using -= 1;
				if( iter->second.cnt_using == 0 ) {
					map_noise.erase( iter );
				}
			}
		}

		{
			std::lock_guard< std::mutex > lock( mtx_file );

			if( chunk.ptr_file ) { 
				glm::ivec3 pos_r;
				Directional::pos_lw_to_r( chunk.pos_lw, pos_r );
				auto iter = map_file.find( Directional::get_hash( pos_r ) );
				if( iter != map_file.end( ) ) {
					iter->second.cnt_using -= 1;
					if( iter->second.cnt_using == 0 ) {
						iter->second.file_stream.close( );
						map_file.erase( iter );
					}
				}
			}
		}

		{ 
			std::lock_guard< std::recursive_mutex > lock( mtx_render );

			map_render.erase( chunk.hash_lw );
		}

		sm_terrain.release_handle( chunk.handle_solid );
		sm_terrain.release_handle( chunk.handle_solid_temp );

		sm_terrain.release_handle( chunk.handle_trans );
		sm_terrain.release_handle( chunk.handle_trans_temp );

		chunk_state_clear( chunk );

		{
			Chunk * ptr_adj_chunk = nullptr;
			std::lock_guard< std::recursive_mutex > lock( mtx_chunks );

			for( int i = 0; i < FD_Size; i++ ) {
				if( chunk.ptr_adj[ i ] != nullptr ) {
					ptr_adj_chunk = chunk.ptr_adj[ i ];

					{
						std::lock_guard< std::mutex > lock( ptr_adj_chunk->mtx_adj );
						if( i % 2 == 0 ) {
							ptr_adj_chunk->ptr_adj[ i + 1 ] = nullptr;
						}
						else {
							ptr_adj_chunk->ptr_adj[ i - 1 ] = nullptr;
						}
						ptr_adj_chunk->cnt_adj -= 1;
					}

					std::lock_guard< std::mutex > lock( chunk.mtx_adj );
					chunk.ptr_adj[ i ] = nullptr;
					chunk.cnt_adj -= 1;
				}
			}

			auto iter_chunk = map_chunks.find( chunk.hash_lw );
			if( iter_chunk != map_chunks.end( ) ) {
				iter_chunk->second.release( );
				map_chunks.erase( iter_chunk );
			}
		}

		chunk.is_working = false;
	} );
}

void ChunkMgr::load_block_data( ) {
	using namespace std::tr2::sys;
	std::ostringstream out;
	path path_base( "./Blocks" );
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement const * ele_block;
	tinyxml2::XMLElement const * ele_face;
	tinyxml2::XMLElement const * ele_data;
	tinyxml2::XMLElement const * ele_var;

	printTabbedLine( 1, "Loading blocks..." );

	for( directory_iterator iter_blocks( path_base ); iter_blocks != directory_iterator( ); ++iter_blocks ) { 
		if( iter_blocks->path( ).extension( ).string( ) != ".xml" ) { 
			continue;
		}

		Block block;

		doc.LoadFile( iter_blocks->path( ).string( ).c_str( ) );
		if( doc.Error( ) ) { 
			std::cout << "ERROR! Error reading: " << iter_blocks->path( ).string( ) << std::endl;
			doc.PrintError( );
			continue;
		}
		ele_block = doc.FirstChildElement( "Block" );

		ele_data = ele_block->FirstChildElement( "Name" );
		if( ele_data != nullptr ) {
			block.name = ele_data->GetText( );
		}

		ele_data = ele_block->FirstChildElement( "Texture" );
		if( ele_data != nullptr ) {
			block.texture = ele_data->GetText( );
			block.id_texture = client.texture_mgr.get_texture_id( block.texture );
		}

		ele_data = ele_block->FirstChildElement( "Transparent" );
		if( ele_data != nullptr ) {
			ele_data->QueryBoolText( &block.is_trans );
		}

		ele_data = ele_block->FirstChildElement( "Color" );
		if( ele_data != nullptr ) {
			ele_var = ele_data->FirstChildElement( "R" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &block.color.r );
			}

			ele_var = ele_data->FirstChildElement( "G" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &block.color.g );
			}

			ele_var = ele_data->FirstChildElement( "B" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &block.color.b );
			}

			ele_var = ele_data->FirstChildElement( "A" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &block.color.a );
			}
		}

		ele_data = ele_block->FirstChildElement( "Collider" );
		if( ele_data != nullptr ) {
			block.is_coll = true;
		}

		ele_face = ele_block->FirstChildElement( "Face" );
		while( ele_face != nullptr ) { 
			Face face;

			ele_data = ele_face->FirstChildElement( "Offset" );
			if( ele_data != nullptr ) { 
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) { 
					ele_var->QueryFloatText( &face.offset.x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.offset.y );
				}

				ele_var = ele_data->FirstChildElement( "Z" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.offset.z );
				}
			}

			ele_data = ele_face->FirstChildElement( "Dimension" );
			if( ele_data != nullptr ) {
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.dim.x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.dim.y );
				}

				ele_var = ele_data->FirstChildElement( "Z" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.dim.z );
				}
			}

			ele_data = ele_face->FirstChildElement( "Rotation" );
			if( ele_data != nullptr ) {
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.rot.x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.rot.y );
				}

				ele_var = ele_data->FirstChildElement( "Z" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.rot.z );
				}
			}

			ele_data = ele_face->FirstChildElement( "Color" );
			if( ele_data != nullptr ) {
				ele_var = ele_data->FirstChildElement( "R" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.r );
				}

				ele_var = ele_data->FirstChildElement( "G" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.g );
				}

				ele_var = ele_data->FirstChildElement( "B" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.b );
				}

				ele_var = ele_data->FirstChildElement( "A" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.a );
				}
			}

			ele_data = ele_face->FirstChildElement( "Occlude" );
			if( ele_data != nullptr ) {
				std::string text( ele_data->GetText( ) );

				if( text == "Front" ) {
					face.occlude = FaceDirection::FD_Front;
				}
				else if( text == "Back" ) {
					face.occlude = FaceDirection::FD_Back;
				}
				else if( text == "Left" ) {
					face.occlude = FaceDirection::FD_Left;
				}
				else if( text == "Right" ) {
					face.occlude = FaceDirection::FD_Right;
				}
				else if( text == "Up" ) {
					face.occlude = FaceDirection::FD_Up;
				}
				else if( text == "Down" ) {
					face.occlude = FaceDirection::FD_Down;
				}
				else {
					face.occlude = FaceDirection::FD_Size;
				}
			}

			ele_data = ele_face->FirstChildElement( "SubTexture" );
			if( ele_data != nullptr ) {
				face.subtex = ele_data->GetText( );
				face.id_subtex = client.texture_mgr.get_texture_layer( block.texture, face.subtex );
			}

			ele_data = ele_face->FirstChildElement( "UV" );
			int cnt = 0;
			while( ele_data != nullptr && cnt < 4 ) { 
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.uvs[ cnt ].x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.uvs[ cnt ].y );
				}

				cnt++;
				ele_data = ele_data->NextSiblingElement( "UV" );
			}

			block.faces.emplace_back( face );

			ele_face = ele_face->NextSiblingElement( "Face" );
		}

		for( int i = 0; i < block.faces.size( ); ++i ) {
			auto & face = block.faces[ i ];

			if( face.occlude != FaceDirection::FD_Size ) { 
				block.occlude_lookup[ face.occlude ].emplace_back( i );
			}
			else { 
				block.include_lookup.emplace_back( i );
			}

			glm::vec3 dim_h = face.dim / 2.0f;
			glm::vec3 pos_c = { 0.5f, 0.5f, 0.5f };
			glm::mat4 rotate_z = glm::rotate( glm::mat4( 1.0f ), glm::radians( face.rot.z ), glm::vec3( 0, 0, 1 ) );
			glm::mat4 rotate_y = glm::rotate( glm::mat4( 1.0f ), glm::radians( face.rot.y ), glm::vec3( 0, 1, 0 ) );
			glm::mat4 rotate_x = glm::rotate( glm::mat4( 1.0f ), glm::radians( face.rot.x ), glm::vec3( 1, 0, 0 ) );

			face.verts[ 0 ] = { dim_h.x, -dim_h.y, 0 };
			face.verts[ 1 ] = { -dim_h.x, -dim_h.y, 0 };
			face.verts[ 2 ] = { -dim_h.x, dim_h.y, 0 };
			face.verts[ 3 ] = { dim_h.x, dim_h.y, 0 };

			face.norms[ 0 ] = { 0, 0, -1 };
			face.norms[ 1 ] = { 0, 0, -1 };
			face.norms[ 2 ] = { 0, 0, -1 };
			face.norms[ 3 ] = { 0, 0, -1 };

			for( auto & vert : face.verts ) { 
				vert = glm::vec3( rotate_z * glm::vec4( vert, 0 ) );
				vert = glm::vec3( rotate_y * glm::vec4( vert, 0 ) );
				vert = glm::vec3( rotate_x * glm::vec4( vert, 0 ) );
				vert = vert + pos_c + face.offset;
			}

			for( auto & norm : face.norms ) { 
				norm = glm::vec3( rotate_z * glm::vec4( norm, 0 ) );
				norm = glm::vec3( rotate_y * glm::vec4( norm, 0 ) );
				norm = glm::vec3( rotate_x * glm::vec4( norm, 0 ) );
				norm = glm::normalize( norm );
			}

			/*glm::vec2 dim_dx = { 1.0f, 1.0f };
			glm::ivec2 const & dim = client.texture_mgr.get_texture( block.texture )->dim;
			dim_dx /= dim;
			int num_x, num_y;*/
			
			for( auto & uv : face.uvs ) {
				/*num_x = std::floor( uv.x / dim_dx.x );
				num_y = std::floor( uv.y / dim_dx.y );

				if( num_x == 0 ) num_x = 1;
				if( num_y == 0 ) num_y = 1;

				uv.x = num_x * dim_dx.x - dim_dx.x / 2.0f;
				uv.y = num_y * dim_dx.y - dim_dx.y / 2.0f;


				std::cout << "Uv: num_x: " << num_x << " num_y: " << num_y << std::endl;
				std::cout << "dim_dx: " << Directional::print_vec( dim_dx ) << " uv: " << Directional::print_vec( uv ) << std::endl;*/

				uv.z = face.id_subtex;
			} 
		}

		block.id = ( int ) list_block_data.size( );
		list_block_data.emplace_back( block );
		map_block_data.insert( { block.name, ( GLuint ) list_block_data.size( ) - 1 } );
	}
	
	printTabbedLine( 1, out.str( ) );
}

void ChunkMgr::load_block_mesh( ) { 
	for( auto & block : list_block_data ) { 
		SMChunkIncl::SMHandle handle;

		handle.clear( );

		sm_inclusive.request_handle( handle );

		handle.push_set( SMChunkIncl::SMGSet(
			SMChunkIncl::TypeGeometry::TG_Triangles,
			glm::mat4( 1.0f ),
			client.texture_mgr.get_program_id( "SMTerrain" ),
			client.texture_mgr.get_texture_id( "Blocks" ) ) );

		handle.request_buffer( );

		GLuint idx_inds = 0;

		for( auto idx_face : block.include_lookup ) { 
			auto & face = block.faces[ idx_face ];
			handle.push_verts( { 
				{
					{ face.verts[ 0 ].x, face.verts[ 0 ].y, face.verts[ 0 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 0 ].x, face.norms[ 0 ].y, face.norms[ 0 ].z },
					{ face.uvs[ 0 ].x, face.uvs[ 0 ].y, face.uvs[ 0 ].z }
				},
				{
					{ face.verts[ 1 ].x, face.verts[ 1 ].y, face.verts[ 1 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 1 ].x, face.norms[ 1 ].y, face.norms[ 1 ].z },
					{ face.uvs[ 1 ].x, face.uvs[ 1 ].y, face.uvs[ 1 ].z }
				},
				{
					{ face.verts[ 2 ].x, face.verts[ 2 ].y, face.verts[ 2 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 2 ].x, face.norms[ 2 ].y, face.norms[ 2 ].z },
					{ face.uvs[ 2 ].x, face.uvs[ 2 ].y, face.uvs[ 2 ].z }
				},
				{
					{ face.verts[ 3 ].x, face.verts[ 3 ].y, face.verts[ 3 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 3 ].x, face.norms[ 3 ].y, face.norms[ 3 ].z },
					{ face.uvs[ 3 ].x, face.uvs[ 3 ].y, face.uvs[ 3 ].z }
				}
			} );

			handle.push_inds( { 
				idx_inds + 0, idx_inds + 1, idx_inds + 2, idx_inds + 2, idx_inds + 3, idx_inds + 0,
			} );

			idx_inds += 4;
		}

		handle.finalize_set( );

		handle.submit_buffer( );

		handle.release_buffer( );

		list_handles_inclusive.push_back( handle );
	}
}

void ChunkMgr::toggle_chunk_debug( ) { 
	is_chunk_debug = !is_chunk_debug;
}

void ChunkMgr::toggle_shadow_debug( ) {
	is_shadow_debug = !is_shadow_debug;
}

void ChunkMgr::toggle_shadows( ) { 
	is_shadows = !is_shadows;
}

void ChunkMgr::toggle_flatshade( ) { 
	is_flatshade = !is_flatshade;
	if( is_flatshade ) { 
		is_shadows = false;
	}
	else { 
		is_shadows = true;
	}
}

int ChunkMgr::get_block( glm::vec3 const & pos_gw ) {
	glm::ivec3 pos_lw;
	Directional::pos_gw_to_lw( pos_gw, pos_lw );
	std::lock_guard< std::recursive_mutex > lock( mtx_chunks );
	auto iter = map_chunks.find( Directional::get_hash( pos_lw ) );
	if( iter != map_chunks.end( ) ) { 
		auto & chunk = iter->second.get( );
		if( chunk.is_loaded ) {
			glm::ivec3 pos_lc;
			Directional::pos_gw_to_lc( pos_gw, pos_lc );
			return iter->second.get( ).id_blocks[ pos_lc.x ][ pos_lc.y ][ pos_lc.z ];
		}
	}
	return -2;
}

void ChunkMgr::set_block( glm::ivec3 const & pos_gw, int const id ) {
	glm::ivec3 pos_lw;
	Directional::pos_gw_to_lw( pos_gw, pos_lw );
	
	std::lock_guard< std::recursive_mutex > lock( mtx_chunks );
	auto iter = map_chunks.find( Directional::get_hash( pos_lw ) );
	if( iter == map_chunks.end( ) ) {
		return;
	}

	auto & chunk = iter->second.get( );
	glm::ivec3 pos_lc;
	Directional::pos_gw_to_lc( pos_gw, pos_lc );
	if( chunk.id_blocks[ pos_lc.x ][ pos_lc.y ][ pos_lc.z ] == id ) {
		return;
	}

	chunk.id_blocks[ pos_lc.x ][ pos_lc.y ][ pos_lc.z ] = id;

	{
		std::lock_guard< std::mutex > lock( chunk.mtx_adj );
		if( pos_lc.x == 0 && chunk.ptr_adj[ FD_Right ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Right ], ChunkState::CS_SMesh, true );
			chunk_state( *chunk.ptr_adj[ FD_Right ], ChunkState::CS_TMesh, true );
		}
		else if( pos_lc.x == WorldSize::Chunk::size_x - 1 && chunk.ptr_adj[ FD_Left ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Left ], ChunkState::CS_SMesh, true );
			chunk_state( *chunk.ptr_adj[ FD_Left ], ChunkState::CS_TMesh, true );
		}

		if( pos_lc.y == 0 && chunk.ptr_adj[ FD_Down ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Down ], ChunkState::CS_SMesh, true );
			chunk_state( *chunk.ptr_adj[ FD_Down ], ChunkState::CS_TMesh, true );
		}
		else if( pos_lc.y == WorldSize::Chunk::size_y - 1 && chunk.ptr_adj[ FD_Up ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Up ], ChunkState::CS_SMesh, true );
			chunk_state( *chunk.ptr_adj[ FD_Up ], ChunkState::CS_TMesh, true );
		}

		if( pos_lc.z == 0 && chunk.ptr_adj[ FD_Back ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Back ], ChunkState::CS_SMesh, true );
			chunk_state( *chunk.ptr_adj[ FD_Back ], ChunkState::CS_TMesh, true );
		}
		else if( pos_lc.z == WorldSize::Chunk::size_z - 1 && chunk.ptr_adj[ FD_Front ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Front ], ChunkState::CS_SMesh, true );
			chunk_state( *chunk.ptr_adj[ FD_Front ], ChunkState::CS_TMesh, true );
		}
	}

	chunk_state( chunk, ChunkState::CS_SMesh, true );
	chunk_state( chunk, ChunkState::CS_TMesh, true );
}

void ChunkMgr::set_block( glm::vec3 const & pos_gw, int const id ) { 
	set_block( glm::ivec3( floor( pos_gw.x ), floor( pos_gw.y ), floor( pos_gw.z ) ), id );
}

void ChunkMgr::set_block( glm::ivec3 const & pos_gw, SetState & state,	int const id ) {
	Directional::pos_gw_to_lw( pos_gw, state.pos_lw );

	if( state.pos_lw == state.pos_lw_last && state.iter != map_chunks.end() ) {
		Directional::pos_gw_to_lc( pos_gw, state.pos_lc );
		auto & chunk = state.iter->second.get( );

		if( chunk.id_blocks[ state.pos_lc.x ][ state.pos_lc.y ][ state.pos_lc.z ] != id ) {
			chunk.id_blocks[ state.pos_lc.x ][ state.pos_lc.y ][ state.pos_lc.z ] = id;

			state.map_queue_dirty.insert( { chunk.hash_lw, chunk } );
			std::lock_guard< std::mutex > lock( chunk.mtx_adj );
			if( state.pos_lc.x == 0 && chunk.ptr_adj[ FD_Right ] != nullptr ) {
				state.map_queue_dirty.insert( {
					chunk.ptr_adj[ FD_Right ]->hash_lw,
					*chunk.ptr_adj[ FD_Right ] } );
			}
			else if( state.pos_lc.x == WorldSize::Chunk::size_x - 1 && chunk.ptr_adj[ FD_Left ] != nullptr ) {
				state.map_queue_dirty.insert( {
					chunk.ptr_adj[ FD_Left ]->hash_lw,
					*chunk.ptr_adj[ FD_Left ] } );
			}

			if( state.pos_lc.y == 0 && chunk.ptr_adj[ FD_Down ] != nullptr ) {
				state.map_queue_dirty.insert( {
					chunk.ptr_adj[ FD_Down ]->hash_lw,
					*chunk.ptr_adj[ FD_Down ] } );
			}
			else if( state.pos_lc.y == WorldSize::Chunk::size_y - 1 && chunk.ptr_adj[ FD_Up ] != nullptr ) {
				state.map_queue_dirty.insert( {
					chunk.ptr_adj[ FD_Up ]->hash_lw,
					*chunk.ptr_adj[ FD_Up ] } );
			}

			if( state.pos_lc.z == 0 && chunk.ptr_adj[ FD_Back ] != nullptr ) {
				state.map_queue_dirty.insert( {
					chunk.ptr_adj[ FD_Back ]->hash_lw,
					*chunk.ptr_adj[ FD_Back ] } );
			}
			else if( state.pos_lc.z == WorldSize::Chunk::size_z - 1 && chunk.ptr_adj[ FD_Front ] != nullptr ) {
				state.map_queue_dirty.insert( {
					chunk.ptr_adj[ FD_Front ]->hash_lw,
					*chunk.ptr_adj[ FD_Front ] } );
			}
		}
	}
	else {
		std::lock_guard< std::recursive_mutex > lock( mtx_chunks );
		state.iter = map_chunks.find( Directional::get_hash( state.pos_lw ) );
		if( state.iter != map_chunks.end( ) ) {
			Directional::pos_gw_to_lc( pos_gw, state.pos_lc );
			auto & chunk = state.iter->second.get( );

			if( chunk.id_blocks[ state.pos_lc.x ][ state.pos_lc.y ][ state.pos_lc.z ] != id ) {
				chunk.id_blocks[ state.pos_lc.x ][ state.pos_lc.y ][ state.pos_lc.z ] = id;

				state.map_queue_dirty.insert( { Directional::get_hash( state.pos_lw ), chunk } );
				std::lock_guard< std::mutex > lock( chunk.mtx_adj );
				if( state.pos_lc.x == 0 && chunk.ptr_adj[ FD_Right ] != nullptr ) {
					state.map_queue_dirty.insert( {
						chunk.ptr_adj[ FD_Right ]->hash_lw,
						*chunk.ptr_adj[ FD_Right ] } );
				}
				else if( state.pos_lc.x == WorldSize::Chunk::size_x - 1 && chunk.ptr_adj[ FD_Left ] != nullptr ) {
					state.map_queue_dirty.insert( {
						chunk.ptr_adj[ FD_Left ]->hash_lw,
						*chunk.ptr_adj[ FD_Left ] } );
				}

				if( state.pos_lc.y == 0 && chunk.ptr_adj[ FD_Down ] != nullptr ) {
					state.map_queue_dirty.insert( {
						chunk.ptr_adj[ FD_Down ]->hash_lw,
						*chunk.ptr_adj[ FD_Down ] } );
				}
				else if( state.pos_lc.y == WorldSize::Chunk::size_y - 1 && chunk.ptr_adj[ FD_Up ] != nullptr ) {
					state.map_queue_dirty.insert( {
						chunk.ptr_adj[ FD_Up ]->hash_lw,
						*chunk.ptr_adj[ FD_Up ] } );
				}

				if( state.pos_lc.z == 0 && chunk.ptr_adj[ FD_Back ] != nullptr ) {
					state.map_queue_dirty.insert( {
						chunk.ptr_adj[ FD_Back ]->hash_lw,
						*chunk.ptr_adj[ FD_Back ] } );
				}
				else if( state.pos_lc.z == WorldSize::Chunk::size_z - 1 && chunk.ptr_adj[ FD_Front ] != nullptr ) {
					state.map_queue_dirty.insert( {
						chunk.ptr_adj[ FD_Front ]->hash_lw,
						*chunk.ptr_adj[ FD_Front ] } );
				}
			}
		}
		state.pos_lw_last = state.pos_lw;
	}
}

void ChunkMgr::set_sphere( glm::ivec3 const & pos_gw, int const size, int const id ) {
	SetState state;
	state.iter = map_chunks.end( );

	set_sphere( pos_gw, state, size, id );

	proc_set_state( state );
}

void ChunkMgr::set_sphere( glm::vec3 const & pos_gw, int const size, int const id ) { 
	set_sphere( glm::ivec3( floor( pos_gw.x ), floor( pos_gw.y ), floor( pos_gw.z ) ), size, id );
}

void ChunkMgr::set_sphere( glm::ivec3 const & pos_gw, SetState & state, int const size, int const id ) {
	glm::ivec3 pos_check;

	for( pos_check.x = -size; pos_check.x <= size; pos_check.x++ ) {
		for( pos_check.y = -size; pos_check.y <= size; pos_check.y++ ) {
			for( pos_check.z = -size; pos_check.z <= size; pos_check.z++ ) {
				if( glm::length( glm::vec3( pos_check ) ) < size ) {
					set_block( pos_gw + pos_check, state, id );
				}
			}
		}
	}
}

void ChunkMgr::set_rect( glm::ivec3 const & pos_gw, glm::ivec3 const & dim, int const id ) {
	SetState state;
	state.iter = map_chunks.end( );

	set_rect( pos_gw, state, dim, id );

	proc_set_state( state );
}

void ChunkMgr::set_rect( glm::vec3 const & pos_gw, glm::ivec3 const & dim, int const id ) {
	set_rect( glm::ivec3( floor( pos_gw.x ), floor( pos_gw.y ), floor( pos_gw.z ) ), dim, id );
}

void ChunkMgr::set_rect( glm::ivec3 const & pos_gw, SetState & state, glm::ivec3 const & dim, int const id ) {
	glm::ivec3 pos_check;

	for( pos_check.x = 0; pos_check.x < dim.x; pos_check.x++ ) {
		for( pos_check.y = 0; pos_check.y < dim.y; pos_check.y++ ) {
			for( pos_check.z = 0; pos_check.z < dim.z; pos_check.z++ ) {
				set_block( pos_gw + pos_check, state, id );
			}
		}
	}
}

void ChunkMgr::set_ellipsoid( glm::ivec3 const & pos_gw, glm::ivec3 const & dim , int const id ) {
	SetState state;
	state.iter = map_chunks.end( );

	set_ellipsoid( pos_gw, state, dim, id );

	proc_set_state( state );
}

void ChunkMgr::set_ellipsoid( glm::vec3 const & pos_gw, glm::ivec3 const & dim, int const id ) {
	set_ellipsoid( glm::ivec3( floor( pos_gw.x ), floor( pos_gw.y ), floor( pos_gw.z ) ), dim, id );
}

void ChunkMgr::set_ellipsoid( glm::ivec3 const & pos_gw, SetState & state, glm::ivec3 const & dim, int const id ) {
	glm::ivec3 pos_check;

	for( pos_check.x = -dim.x; pos_check.x <= dim.x; pos_check.x++ ) {
		for( pos_check.y = -dim.y; pos_check.y <= dim.y; pos_check.y++ ) {
			for( pos_check.z = -dim.z; pos_check.z <= dim.z; pos_check.z++ ) {
				if( pow( pos_check.x / float( dim.x ), 2 ) +
					pow( pos_check.y / float( dim.y ), 2 ) +
					pow( pos_check.z / float( dim.z ), 2 ) < 1 ) {
					set_block( pos_gw + pos_check, state, id );
				}
			}
		}
	}
}

void ChunkMgr::set_tree( glm::ivec3 const & pos_gw, int const id ) {
	SetState state;
	state.iter = map_chunks.end( );

	set_tree( pos_gw, state, id );

	proc_set_state( state );
}

void ChunkMgr::set_tree( glm::vec3 const & pos_gw, int const id ) {
	set_tree( glm::ivec3( floor( pos_gw.x ), floor( pos_gw.y ), floor( pos_gw.z ) ), id );
}

void ChunkMgr::set_tree( glm::ivec3 const & pos_gw, SetState & state, int const id ) {
	if( id == 0 ) {
		float noise_scale = 0.006f;
		int const height_min = 6, height_max = 10;
		int const radius_min = 3, radius_max = 6;

		auto & block_leaves = get_block_data( "Leaves Light Green" );
		auto & block_log = get_block_data( "Log" );
		int tree_height;
		int tree_radius;

		tree_height = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( height_max - height_min );
		tree_height = height_min + abs( tree_height );

		tree_radius = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( radius_max - radius_min );
		tree_radius = radius_min + abs( tree_radius );

		set_sphere( 
			pos_gw + glm::ivec3( 0, tree_height, 0 ), state, 
			tree_radius, 
			block_leaves.id );

		set_rect( 
			pos_gw, state, 
			glm::ivec3( 1, tree_height, 1 ), 
			block_log.id );
	}
	else if( id == 1 ) {
		float noise_scale = 0.006f;
		int const height_min = 10, height_max = 16;
		int const radius_min = 5, radius_max = 9;

		auto & block_leaves = get_block_data( "Leaves Green" );
		auto & block_log = get_block_data( "Birch Log" );

		float scale_radius;
		int tree_height;
		int tree_radius;

		tree_height = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( height_max - height_min );
		tree_height = height_min + abs( tree_height );

		tree_radius = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( radius_max - radius_min );
		tree_radius = radius_min + abs( tree_radius );

		for( int i = 0; i < tree_height * 2; i++ ) {
			scale_radius = 1 - i / float( tree_height * 2 - 2 );
			set_ellipsoid( 
				pos_gw + glm::ivec3( 0, 2 + i, 0 ), state, 
				glm::ivec3( tree_radius * scale_radius, 1, tree_radius * scale_radius ), 
				block_leaves.id );
		}

		set_rect( 
			pos_gw, state, 
			glm::ivec3( 1, tree_height, 1 ), 
			block_log.id );
	}
	else if( id == 2 ) {
		float noise_scale = 0.006f;
		int const height_min = 12, height_max = 18;
		int const radius_min = 4, radius_max = 8;

		auto & block_leaves = get_block_data( "Leaves Red" );
		auto & block_log = get_block_data( "Birch Log" );

		int tree_height;
		int tree_radius;

		tree_height = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( height_max - height_min );
		tree_height = height_min + abs( tree_height );

		for( int i = tree_height; i >= 4; i-= 6 ) {
			tree_radius = raw_noise_2d(
				( pos_gw.x + i * 100 ) * noise_scale,
				( pos_gw.z + i * 100 ) * noise_scale ) *
				( radius_max - radius_min );
			tree_radius = radius_min + abs( tree_radius );

			set_ellipsoid(
				pos_gw + glm::ivec3( 0, i, 0 ), state,
				glm::ivec3( tree_radius, 4, tree_radius ),
				block_leaves.id );
		}

		set_rect( 
			pos_gw, state, 
			glm::ivec3( 1, tree_height, 1 ), 
			block_log.id );
	}
	else if( id == 3 ) { 
		float noise_scale = 0.006f;
		int const height_min = 3, height_max = 6;

		auto & block_log = get_block_data( "Cactus" );

		int tree_height;

		tree_height = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( height_max - height_min );
		tree_height = height_min + abs( tree_height );

		set_rect(
			pos_gw, state,
			glm::ivec3( 1, tree_height, 1 ),
			block_log.id );
	}
	else if( id == 4 ) { 
		float noise_scale = 0.006f;
		int const height_min = 10, height_max = 16;
		int const radius_min = 5, radius_max = 9;

		auto & block_leaves = get_block_data( "Leaves Brown" );
		auto & block_log = get_block_data( "Birch Log" );

		float scale_radius;
		int tree_height;
		int tree_radius;

		tree_height = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( height_max - height_min );
		tree_height = height_min + abs( tree_height );

		tree_radius = raw_noise_2d(
			pos_gw.x * noise_scale,
			pos_gw.z * noise_scale ) *
			( radius_max - radius_min );
		tree_radius = radius_min + abs( tree_radius );

		for( int i = 0; i < tree_height - 2; i++ ) {
			if( i % 2 == 0 ) {
				scale_radius = 1 - i / float( tree_height * 2 - 2 );
				set_ellipsoid(
					pos_gw + glm::ivec3( 0, 2 + i, 0 ), state,
					glm::ivec3( tree_radius * scale_radius, 1, tree_radius * scale_radius ),
					block_leaves.id );
			}
		}

		set_rect(
			pos_gw, state,
			glm::ivec3( 1, tree_height, 1 ),
			block_log.id );
	}
}

void ChunkMgr::explode_sphere( glm::vec3 const & pos_gw, int const size ) {
	explode_sphere_recur( pos_gw, size, 0 );
}

void ChunkMgr::explode_sphere_recur( glm::vec3 const & pos_gw, int const size, int depth ) { 
	glm::ivec3 pos_check;
	glm::vec3 vec_fwd;
	int id_curr = 0;
	auto & block_water = get_block_data( "Water" );
	auto & block_tnt = get_block_data( "Tnt" );

	depth++;

	for( pos_check.x = -size; pos_check.x <= size; pos_check.x++ ) {
		for( pos_check.y = -size; pos_check.y <= size; pos_check.y++ ) {
			for( pos_check.z = -size; pos_check.z <= size; pos_check.z++ ) {
				if( glm::length( glm::vec3( pos_check ) ) >= size ) {
					continue;
				}

				vec_fwd = glm::vec3( rand( ) % 200 - 100, rand( ) % 200 - 100, rand( ) % 200 - 100 );
				vec_fwd = glm::normalize( vec_fwd );
				id_curr = get_block( pos_gw + glm::vec3( pos_check ) );

				if( id_curr == block_tnt.id ) {
					client.entity_mgr.entity_add(
						"Tnt",
						[ id_curr, pos_gw, pos_check ] ( Client & client, Entity & entity ) {
							entity.id = id_curr;
							entity.color = client.chunk_mgr.get_block_data( id_curr ).color;

							auto & ec_state = entity.h_state.get( );
							ec_state.pos = glm::floor( pos_gw ) + glm::vec3( pos_check ) + glm::vec3( 0.5f, 0.5f, 0.5f );
							//ec_state.dim = glm::vec3( 0.5f, 0.5f, 0.5f );

							return ErrorEntity::EE_Ok;
						}
					);
				}
				else if( id_curr != -1 && id_curr != -2 ) {
					if( rand( ) % 1000 <= 666 ) { 
						continue;
					}

					client.entity_mgr.entity_add(
						"Grav Block",
						[ id_curr, pos_gw, pos_check ] ( Client & client, Entity & entity ) {
							entity.id = id_curr;
							entity.color = client.chunk_mgr.get_block_data( id_curr ).color;

							auto & ec_state = entity.h_state.get( );
							ec_state.pos = glm::floor( pos_gw ) + glm::vec3( pos_check ) + glm::vec3( 0.5f, 0.5f, 0.5f );
							ec_state.veloc = glm::vec3( rand( ) % 200 - 100, rand( ) % 200 - 100, rand( ) % 200 - 100 );
							ec_state.veloc = glm::normalize( ec_state.veloc );
							ec_state.veloc *= 10.0f + ( rand( ) % 25 );
							//ec_state.dim = glm::vec3( 0.5f, 0.5f, 0.5f );

							return ErrorEntity::EE_Ok;
						}
					);
				}
			}
		}
	}

	set_sphere( pos_gw, size, -1 );
}

Block & ChunkMgr::get_block_data( int const id ) {
	return list_block_data[ id ];
}

Block & ChunkMgr::get_block_data( std::string const & name ) {
	return list_block_data[ map_block_data[ name ] ];
}

Block * ChunkMgr::get_block_data_safe( std::string const & name ) {
	auto iter = map_block_data.find( name );
	if( iter == map_block_data.end( ) ) {
		return nullptr;
	}

	return &list_block_data[ iter->second ];
}

std::string const & ChunkMgr::get_block_string( int const id ) { 
	static std::string str_air( "Air" );
	static std::string str_null( "Null" );

	if( id == -1 ) {
		return str_air;
	}
	else if( id >= 0 && id < list_block_data.size( ) ) {
		return list_block_data[ id ].name;
	}
	else { 
		return str_null;
	}
}

int ChunkMgr::get_num_blocks( ) {
	return ( int ) list_block_data.size( );
}

void ChunkMgr::print_dirty( ) {
	if( map_dirty.size( ) > 0 ) {
		auto & out = client.display_mgr.out;
		out.str( "" );
		out << "Printing dirty list:";
		client.gui_mgr.print_to_console( out.str( ) );

		auto iter = map_dirty.begin( );
		while( iter != map_dirty.end( ) ) {
			out.str( "" );
			out << "Chunk: " << Directional::print_vec( iter->second.pos_lw );
			out << " working: " << iter->second.is_working;
			out << " loaded: " << iter->second.is_loaded;
			out << " shutdown: " << iter->second.is_shutdown;
			out << " SCount: " << iter->second.cnt_states << " States: ";
			for( int i = 0; i < ChunkState::CS_Size; i++ ) {
				if( iter->second.states[ i ] ) out << i << " ";
			}
			client.gui_mgr.print_to_console( out.str( ) );
			iter++;
		}
	}
}

void ChunkMgr::print_prio( int const range, int const base_prio ) {
	auto & out = client.display_mgr.out;
	glm::ivec3 pos_chunk;
	int max_world = Directional::get_max( WorldSize::World::vec_size );
	int max_dir;
	int priority;
	int num_print = 0;

	for( int i = pos_center_chunk_lw.x - range; i <= pos_center_chunk_lw.x + range; i++ ) {
		for( int j = pos_center_chunk_lw.y - range; j <= pos_center_chunk_lw.y + range; j++ ) {
			for( int k = pos_center_chunk_lw.z - range; k <= pos_center_chunk_lw.z + range; k++ ) {
				pos_chunk = glm::ivec3( i, j, k );
				max_dir = Directional::get_max( pos_center_chunk_lw - pos_chunk );
				if( max_dir == range ) {
					priority = base_prio + ( 1.0f - float( max_dir ) / max_world ) * ( client.thread_mgr.get_max_prio() / 2 );
					out.str( "" );
					out << Directional::print_vec( pos_chunk ) << " prio: " << priority;
					client.gui_mgr.print_to_console( out.str( ) );
					num_print++;
				}
			}
		}
	}

	out.str( "" );
	out << "Printed " << num_print << " chunk priority.";
	client.gui_mgr.print_to_console( out.str( ) );
}

void ChunkMgr::add_emitter( Emitter & emitter ) { 
	if( light_data.num_emitters.x < light_data.max_emitters ) {
		light_data.list_pos[ light_data.num_emitters.x ] = emitter.pos;
		light_data.list_color[ light_data.num_emitters.x ] = emitter.color;
		light_data.list_radius[ light_data.num_emitters.x ].x = emitter.radius;
		light_data.num_emitters.x++;

		auto & out = client.display_mgr.out;
		out.str( "" );
		out << "Emitter Added: " << Directional::print_vec( emitter.pos );
		client.gui_mgr.print_to_console( out.str( ) );
		out.str( "" );
		out << "Num emitters: " << light_data.num_emitters.x;
		client.gui_mgr.print_to_console( out.str( ) );
	}
}

void ChunkMgr::clear_emitters( ) { 
	light_data.num_emitters.x = 0;
}

void ChunkMgr::print_center_chunk_mesh( ) {
	auto iter_chunks = map_chunks.find( Directional::get_hash( pos_center_chunk_lw ) );
	if( iter_chunks == map_chunks.end( ) ) { 
		return;
	}

	auto & chunk = iter_chunks->second.get( );

	auto & out = client.display_mgr.out;
	out.str( "" );
	out << "[Chunk Data]:" << "\n" <<
		"pos_lw: " << Directional::print_vec( chunk.pos_lw ) << "\n" <<
		"pos_gw: " << Directional::print_vec( chunk.pos_gw ) << "\n\n" <<
		"num_vbo_blcks: " << chunk.handle_solid.get_size_vbo( ) << "\n" <<
		chunk.handle_solid.print_vbo( ) << "\n" << 
		"num_ibo_blocks: " << chunk.handle_solid.get_size_ibo( ) << "\n" <<
		chunk.handle_solid.print_ibo( ) << "\n" <<
		"commands: " << "\n" <<
		chunk.handle_solid.print_commands( ) << "\n";

	client.gui_mgr.print_to_console( out.str( ) );
}

inline void put_face(
	SMTerrain::SMTHandle & handle, glm::ivec3 const & pos,
	glm::vec4 const & color, FaceDirection dir, Face const & face ) {

	GLuint idx_inds = ( GLuint ) handle.ptr_buffer->list_verts.size( );

	VertTerrain vert {
		( GLuint ) ( pos.x ), ( GLuint ) ( pos.y ), ( GLuint ) ( pos.z ),
		( GLuint ) ( color.r * 31 ), ( GLuint ) ( color.g * 31 ),
		( GLuint ) ( color.b * 31 ), ( GLuint ) ( color.a * 31 ),
		( GLuint ) face.id_subtex, ( GLuint ) 0,
		( GLuint ) dir, ( GLuint ) 0
	};

	handle.push_verts( {
		vert, vert, vert, vert
	}  );
	
	handle.push_inds( {
		idx_inds + 0, idx_inds + 1, idx_inds + 2,
		idx_inds + 2, idx_inds + 3, idx_inds + 0
	} );
}

inline void put_face(
	SMTerrain::SMTHandle & handle, glm::ivec3 const & pos,
	glm::vec4 const & color, FaceDirection dir, Face const & face,
	glm::vec3 const & scale_verts, glm::vec2 const & scale_uvs ) {

	GLuint idx_inds = ( GLuint ) handle.ptr_buffer->list_verts.size( );

	GLuint scale = Directional::get_max( scale_verts );

	VertTerrain vert{ 
		( GLuint ) ( pos.x ), ( GLuint ) ( pos.y ), ( GLuint ) ( pos.z ),
		( GLuint ) ( color.r * 31 ), ( GLuint ) ( color.g * 31 ), 
		( GLuint ) ( color.b * 31 ), ( GLuint ) ( color.a * 31 ),
		( GLuint ) face.id_subtex, ( GLuint ) 0,
		( GLuint ) dir, ( GLuint ) scale - 1 
	};

	handle.push_verts( {
		vert, vert, vert, vert
	} );
	
	handle.push_inds( {
		idx_inds + 0, idx_inds + 1, idx_inds + 2,
		idx_inds + 2, idx_inds + 3, idx_inds + 0
	} );
}

inline void ChunkMgr::put_sort(
	std::vector< std::pair< float, GLuint > > & list_sort,
	glm::vec3 & pos_gw,
	Block & block, FaceDirection face ) {


	list_sort.push_back( {
		glm::length2( pos_gw + block.faces[ face ].offset - client.display_mgr.camera.pos_camera ),
		( GLuint ) list_sort.size( )
	} );
}

inline void ChunkMgr::put_sort(
	std::vector< std::pair< float, GLuint > > & list_sort,
	glm::vec3 & pos_gw, glm::vec3 & scale, 
	Block & block, FaceDirection face ) {


	list_sort.push_back( {
		glm::length2( pos_gw + block.faces[ face ].offset * scale - client.display_mgr.camera.pos_camera ),
		( GLuint ) list_sort.size( )
	} );
}