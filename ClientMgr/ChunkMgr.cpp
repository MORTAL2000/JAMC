#include "ChunkMgr.h"

#include "Client.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>

#include "simplexnoise.h"

glm::ivec3 const World::size_vect( World::size_x, World::size_y, World::size_z );

ChunkMgr::ChunkMgr( Client & client ) :
	Manager( client ) { }

ChunkMgr::~ChunkMgr( ) { }

void ChunkMgr::init( ) {
	printTabbedLine( 0, "Init ChunkMgr..." );

	load_block_data( );

	std::cout << std::endl;
	printTabbedLine( 1, "Init Pools..." );

	client.resource_mgr.reg_pool< Chunk >( World::num_chunks );

	map_chunks.reserve( World::num_chunks );
	map_dirty.reserve( World::num_chunks );
	map_noise.reserve( ( World::size_x * 2 + 1 ) * ( World::size_z * 2 + 1 ) + 32 );

	client.resource_mgr.pool< Chunk >( ).apply_func_all( [ & ] ( Chunk & chunk ) { 
		chunk_vbo( chunk );
	} );

	for( int i = 0; i < ChunkMgr::size_pool_buff; i++ ) { 
		list_avail_buff.push_back( &list_pool_buff[ i ] );
	}

	pos_center_chunk_lw = glm::ivec3( 0, 0, 0 );

	{
		float padding = 0.1f;
		glGenBuffers( 1, &id_vbo_chunk_outline );
		glBindBuffer( GL_ARRAY_BUFFER, id_vbo_chunk_outline );
		std::vector< GLfloat > temp_verts;
		temp_verts.push_back( padding );					temp_verts.push_back( padding );					temp_verts.push_back( padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( padding );					temp_verts.push_back( padding );

		temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( padding );

		temp_verts.push_back( padding );					temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_z - padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_z - padding );

		temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( Chunk::size_z - padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( Chunk::size_z - padding );

		temp_verts.push_back( padding );					temp_verts.push_back( padding );					temp_verts.push_back( padding );
		temp_verts.push_back( padding );					temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_z - padding );

		temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( padding );
		temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( Chunk::size_z - padding );

		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( padding );					temp_verts.push_back( padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_z - padding );
	
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( Chunk::size_z - padding );

		temp_verts.push_back( padding );					temp_verts.push_back( padding );					temp_verts.push_back( padding );
		temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( padding );

		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( padding );					temp_verts.push_back( padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( padding );

		temp_verts.push_back( padding );					temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_z - padding );
		temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( Chunk::size_z - padding );

		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( padding );					temp_verts.push_back( Chunk::size_z - padding );
		temp_verts.push_back( Chunk::size_x - padding );	temp_verts.push_back( Chunk::size_y - padding );	temp_verts.push_back( Chunk::size_z - padding );

		size_chunk_outline = temp_verts.size( );

		glBufferData( GL_ARRAY_BUFFER, size_chunk_outline * sizeof( GLfloat ), temp_verts.data( ), GL_STATIC_DRAW );
	}

	std::cout << std::endl;
	printTabbedLine( 1, "Init Light..." );
	init_light( );

	std::cout << std::endl;
	printTabbedLine( 1, "Creating FBO..." );
	glGenFramebuffers( 1, &id_depth_fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, id_depth_fbo );

	std::cout << std::endl;
	printTabbedLine( 1, "Creating shadowmap texture..." );
	glGenTextures( 1, &id_tex_depth );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, id_tex_depth );

	std::cout << std::endl;
	printTabbedLine( 1, "Binding shadowmap data..." );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
	GLfloat borderColor[ ] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id_tex_depth, 0 );
	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	client.texture_mgr.bind_program( "ShadowMap" );
	int idx_sampler = glGetUniformLocation( client.texture_mgr.id_prog, "frag_sampler" );
	glUniform1i( idx_sampler, 0 );

	client.texture_mgr.bind_program( "Terrain" );
	idx_sampler = glGetUniformLocation( client.texture_mgr.id_prog, "frag_shadow" );
	glUniform1i( idx_sampler, 1 );

	int idx_bias = glGetUniformLocation( client.texture_mgr.id_prog, "bias_l" );
	glUniform1f( idx_bias, 0.000005f );

	idx_bias = glGetUniformLocation( client.texture_mgr.id_prog, "bias_h" );
	glUniform1f( idx_bias, 0.000005f );

	glActiveTexture( GL_TEXTURE0 );

	using namespace std::tr2::sys;
	path path_world( "./World" );

	if( !exists( path_world ) ) {
		create_directory( path_world );
	}

	is_chunk_debug = false;
	is_shadow_debug = false;
	is_shadows = true;

	std::cout << std::endl;
	printTabbedLine( 1, checkGlErrors( ) );
	std::cout << std::endl;
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

					if( !Directional::is_within_range( chunk.pos_lw, World::size_vect, pos_center_chunk_lw ) ) {
						chunk_state( chunk, ChunkState::CS_Save, true );
						chunk_state( chunk, ChunkState::CS_Remove, true );
						chunk.is_shutdown = true;
					}
					else {
						glm::ivec3 pos_adj;

						for( int i = 0; i < FaceDirection::FD_Size; i++ ) {
							if( chunk.ptr_adj[ i ] == nullptr ) {
								pos_adj = chunk.pos_lw + Directional::get_vec_dir_i( ( FaceDirection ) i );

								if( Directional::is_within_range( pos_adj, World::size_vect, pos_center_chunk_lw ) ) {
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
								chunk_state( chunk, ChunkState::CS_SMesh, true );
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
}

void ChunkMgr::render( ) {
	client.texture_mgr.bind_terrain( );
	render_skybox( );
	render_exlude( );
	render_sort( );
	render_pass_shadow( );
	render_pass_norm( );
	render_debug( );
}

void ChunkMgr::render_skybox( ) { 
	client.texture_mgr.unbind_program( );
	client.texture_mgr.bind_skybox( );

	client.display_mgr.draw_skybox( client.display_mgr.camera.pos_camera + glm::vec3( 0, -200, 0 ), 2500 );
	client.display_mgr.draw_sun( glm::vec3( light_data.sun_data.pos_sun ), 50 );
}

void ChunkMgr::render_exlude( ) {
	auto & camera = client.display_mgr.camera.pos_camera;

	client.time_mgr.begin_record( RecordStrings::RENDER_EXLUSION );

	list_render.clear( );

	for( auto & pair_render : map_render ) {
		auto & chunk = pair_render.second;
		if( Directional::is_point_in_cone(
			chunk.pos_gw,
			client.display_mgr.camera.pos_camera,
			client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
			client.display_mgr.fov ) ||
			Directional::is_point_in_cone(
				chunk.pos_gw + glm::ivec3( Chunk::vec_size.x, 0, 0 ),
				client.display_mgr.camera.pos_camera,
				client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
				client.display_mgr.fov ) ||
			Directional::is_point_in_cone(
				chunk.pos_gw + glm::ivec3( Chunk::vec_size.x, Chunk::vec_size.y, 0 ),
				client.display_mgr.camera.pos_camera,
				client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
				client.display_mgr.fov ) ||
			Directional::is_point_in_cone(
				chunk.pos_gw + glm::ivec3( 0, Chunk::vec_size.y, 0 ),
				client.display_mgr.camera.pos_camera,
				client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
				client.display_mgr.fov ) ||

			Directional::is_point_in_cone(
				chunk.pos_gw + glm::ivec3( 0, 0, Chunk::vec_size.z ),
				client.display_mgr.camera.pos_camera,
				client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
				client.display_mgr.fov ) ||
			Directional::is_point_in_cone(
				chunk.pos_gw + glm::ivec3( Chunk::vec_size.x, 0, Chunk::vec_size.z ),
				client.display_mgr.camera.pos_camera,
				client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
				client.display_mgr.fov ) ||
			Directional::is_point_in_cone(
				chunk.pos_gw + glm::ivec3( Chunk::vec_size.x, Chunk::vec_size.y, Chunk::vec_size.z ),
				client.display_mgr.camera.pos_camera,
				client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
				client.display_mgr.fov ) ||
			Directional::is_point_in_cone(
				chunk.pos_gw + glm::ivec3( 0, Chunk::vec_size.y, Chunk::vec_size.z ),
				client.display_mgr.camera.pos_camera,
				client.display_mgr.camera.pos_camera + glm::vec3( client.display_mgr.camera.vec_front ),
				client.display_mgr.fov ) ||
			glm::distance( glm::vec3( chunk.pos_lw ), glm::vec3( pos_center_chunk_lw ) ) <= 3.0
			) {
			list_render.push_back( &chunk );
		}
	}

	client.time_mgr.end_record( RecordStrings::RENDER_EXLUSION );
	client.time_mgr.push_record( RecordStrings::RENDER_EXLUSION );
}

void ChunkMgr::render_sort( ) {
	client.time_mgr.begin_record( RecordStrings::RENDER_SORT );

	std::sort( list_render.begin( ), list_render.end( ), [ & ] ( Chunk * lro, Chunk * rho ) {
		return	glm::length( client.display_mgr.camera.pos_camera - glm::vec3( lro->pos_gw + Chunk::vec_size / 2 ) ) >
			glm::length( client.display_mgr.camera.pos_camera - glm::vec3( rho->pos_gw + Chunk::vec_size / 2 ) );
	} );

	client.time_mgr.end_record( RecordStrings::RENDER_SORT );
	client.time_mgr.push_record( RecordStrings::RENDER_SORT );
}

static glm::mat4 mat_model;
static glm::mat3 mat_norm;
static glm::mat4 mat_ortho_light;
static glm::mat4 mat_view_light;
static glm::mat4 mat_proj_light;

void ChunkMgr::render_pass_shadow( ) {
	if( is_shadow_debug ) {
		client.texture_mgr.unbind_program( );
		client.display_mgr.set_ortho( );

		glBindTexture( GL_TEXTURE_2D, id_tex_depth );

		glDisable( GL_LIGHTING );

		glBegin( GL_QUADS );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( 200.0f, 200.0f );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( 500.0f, 200.0f );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( 500.0f, 500.0f );

		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( 200.0f, 500.0f );

		glEnd( );

		glEnable( GL_LIGHTING );

		client.display_mgr.set_proj( );
	}

	if( is_shadows ) {
		client.texture_mgr.bind_terrain( );
		client.texture_mgr.bind_program( "ShadowMap" );
		static GLuint idx_mat_light = glGetUniformLocation( client.texture_mgr.id_prog, "mat_light" );
		static GLuint idx_mat_model = glGetUniformLocation( client.texture_mgr.id_prog, "mat_model" );

		glViewport( 0, 0, SHADOW_WIDTH, SHADOW_HEIGHT );
		glBindFramebuffer( GL_FRAMEBUFFER, id_depth_fbo );
		glClear( GL_DEPTH_BUFFER_BIT );

		mat_ortho_light = glm::ortho( -dim_ortho, dim_ortho, -dim_ortho, dim_ortho, near_plane, far_plane );

		mat_view_light = glm::lookAt(
			client.display_mgr.camera.pos_camera +
			glm::normalize(
				glm::vec3( client.chunk_mgr.light_data.sun_data.pos_sun ) - client.display_mgr.camera.pos_camera
			) * ( far_plane / 2.0f ),
			client.display_mgr.camera.pos_camera,
			glm::vec3( 0.0f, 1.0f, 0.0f ) );

		mat_view_light =
			glm::rotate( glm::mat4( 1.0f ), glm::radians( client.display_mgr.camera.rot_camera.y ), glm::vec3( 0.0f, 0.0f, 1.0f ) ) *
			mat_view_light;

		mat_proj_light = mat_ortho_light * mat_view_light * glm::mat4( 1.0f );

		glCullFace( GL_FRONT );

		glUniformMatrix4fv( idx_mat_light, 1, GL_FALSE, glm::value_ptr( mat_proj_light ) );

		for( auto chunk : list_render ) {
			if( Directional::is_within_range( chunk->pos_lw, glm::ivec3( 4, 3, 4 ), pos_center_chunk_lw ) ) {
				mat_model = glm::translate( glm::mat4( 1.0f ), glm::vec3( chunk->pos_gw ) );
				glUniformMatrix4fv( idx_mat_model, 1, GL_FALSE, glm::value_ptr( mat_model ) );

				chunk_render_shadowmap( *chunk );
			}
		}

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		glCullFace( GL_BACK );
	}
	else { 
		glBindFramebuffer( GL_FRAMEBUFFER, id_depth_fbo );
		glClear( GL_DEPTH_BUFFER_BIT );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
}

void ChunkMgr::render_pass_norm( ) {
	client.texture_mgr.bind_terrain( );
	client.texture_mgr.bind_program( "Terrain" );
	static GLuint idx_mat_model = glGetUniformLocation( client.texture_mgr.id_prog, "mat_model" );
	static GLuint idx_mat_norm = glGetUniformLocation( client.texture_mgr.id_prog, "mat_norm" );
	static GLuint idx_mat_light = glGetUniformLocation( client.texture_mgr.id_prog, "mat_light" );

	glUniformMatrix4fv( idx_mat_light, 1, GL_FALSE, glm::value_ptr( mat_proj_light ) );

	client.display_mgr.resize_window( client.display_mgr.get_window ( ) );

	for( auto chunk : list_render ) {
		mat_model = glm::translate( glm::mat4( 1.0f ), glm::vec3( chunk->pos_gw ) );
		glUniformMatrix4fv( idx_mat_model, 1, GL_FALSE, glm::value_ptr( mat_model ) );

		mat_norm = glm::inverseTranspose( glm::mat3( mat_model ) );
		glUniformMatrix3fv( idx_mat_norm, 1, GL_FALSE, glm::value_ptr( mat_norm ) );

		chunk_render( *chunk );
	}
}

void ChunkMgr::render_debug( ) {
	//vbo.render( client );

	if( is_chunk_debug ) {
		/*
		client.display_mgr.set_camera( );

		auto & camera = client.display_mgr.camera.pos_camera;

		client.texture_mgr.unbind_program( );

		glDisable( GL_TEXTURE_2D );

		glBindBuffer( GL_ARRAY_BUFFER, id_vbo_chunk_outline );
		glVertexPointer( 3, GL_FLOAT, sizeof( GLfloat ) * 3, BUFFER_OFFSET( 0 ) );

		Chunk * chunk;
		glm::ivec3 * pos_chunk;
		std::lock_guard< std::recursive_mutex > lock( mtx_render );
		for( auto & pair_render : map_render ) {
			chunk = &pair_render.second;
			pos_chunk = &chunk->pos_gw;

			if( chunk->cnt_adj != 6 ) {
				glPushMatrix( );

				glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
				glTranslatef( pos_chunk->x, pos_chunk->y, pos_chunk->z );
				glDrawArrays( GL_LINES, 0, size_chunk_outline / 3 );

				glPopMatrix( );
			}
			else {
				glPushMatrix( );

				glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
				glTranslatef( pos_chunk->x, pos_chunk->y, pos_chunk->z );
				glDrawArrays( GL_LINES, 0, size_chunk_outline / 3 );

				glPopMatrix( );
			}
		}

		glBegin( GL_LINES );
		glVertex3f( camera.x, camera.y - 1, camera.z );
		glVertex3f(
			camera.x + light_data.sun_data.pos_sun.x,
			camera.y + light_data.sun_data.pos_sun.y,
			camera.z + light_data.sun_data.pos_sun.z );
		glEnd( );

		glEnable( GL_TEXTURE_2D );
		*/
	}
}

void ChunkMgr::end( ) { 
}

void ChunkMgr::sec( ) {
	//auto & out = client.display_mgr.out;
	//out.str( "" );
	//out << "Time average read: " << client.time_mgr.get_average( std::string( "FILE" ) ) << "ms | save:" << client.time_mgr.get_average( std::string( "SAVE" ) ) << "ms";
	//client.gui_mgr.print_to_console( out.str( ) );
}

int const level_sea = Chunk::size_y / 2;

GLfloat amb_light[ ] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat diff_light[ ] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat spec_light[ ] = { 0.5f, 0.5f, 0.3f, 1.0f };

GLfloat amb_mat[ ] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat diff_mat[ ] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat spec_mat[ ] = { 0.5f, 0.5f, 0.3f, 1.0f };

void ChunkMgr::init_light( ) {
	light_data.num_emitters.x = 0;
	pos_deg_light = 75;
	is_sun_pause = true;

	glEnable( GL_LIGHT0 );
	glMaterialf( GL_FRONT, GL_SHININESS, 20.0f );

	glMaterialfv( GL_FRONT, GL_SPECULAR, spec_mat );
	glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diff_light );

	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
	glLightfv( GL_LIGHT0, GL_AMBIENT, diff_light );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diff_light );
	glLightfv( GL_LIGHT0, GL_SPECULAR, spec_light );
}

float const max_amb = 0.5f;
float const min_amb = 0.1f;

void ChunkMgr::calc_light( ) {
	float time_game = client.time_mgr.get_time( TimeStrings::GAME );
	if( !is_sun_pause ) pos_deg_light += DELTA_CORRECT;
	while( pos_deg_light >= 360 ) pos_deg_light -= 360;
	float rad_time = pos_deg_light  * PI / 180;
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

	light_data.sun_data.pos_sun = glm::vec4( client.display_mgr.camera.pos_camera, 0.0 ) + glm::vec4(
		cos( rad_time ) * dist_sun,
		sin( rad_time ) * dist_sun,
		sin( rad_time + 90 ) * ( dist_sun / 2 ),
		1.0 );

	light_data.sun_data.ambient = glm::vec4( light_amb, light_amb, light_amb, 1.0f );
	light_data.sun_data.diffuse = glm::vec4( light_amb, light_amb, light_amb, 0.0f );

	GLfloat pos[ ] = { light_data.sun_data.pos_sun.x, light_data.sun_data.pos_sun.y, light_data.sun_data.pos_sun.z, 0.0 };
	glLightfv( GL_LIGHT0, GL_POSITION, pos );
	glLightfv( GL_LIGHT0, GL_AMBIENT, glm::value_ptr( light_data.sun_data.ambient ) );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, glm::value_ptr( light_data.sun_data.diffuse ) );
}

void ChunkMgr::proc_set_state( SetState & state ) {
	for( auto & pair_chunk : state.map_queue_dirty ) {
		chunk_state( pair_chunk.second, ChunkState::CS_SMesh, true );
	}
}

ChunkBuffer * ChunkMgr::get_buffer() {
	ChunkBuffer * ptr_buffer = nullptr;
	std::lock_guard< std::mutex > lock( mtx_pool_buff );

	if( !list_avail_buff.empty() ) {
		ptr_buffer = list_avail_buff.back( );
		list_avail_buff.pop_back( );
	}

	return ptr_buffer;
}

void ChunkMgr::put_buffer( ChunkBuffer *& ptr_buffer ) {
	std::lock_guard< std::mutex > lock( mtx_pool_buff );

	if( ptr_buffer != nullptr ) {
		list_avail_buff.push_back( ptr_buffer );
	}

	ptr_buffer = nullptr;
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
		else if( chunk.states[ CS_Buffer ] ) {
			chunk_buffer( chunk );
		}
		else if( chunk.states[ CS_SMesh ] ) {
			chunk_smesh( chunk );
		}
		else if( chunk.states[ CS_TMesh ] ) {
			chunk_tmesh( chunk );
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

void ChunkMgr::chunk_render( Chunk & chunk ) {
	glBindVertexArray( chunk.id_vao );

	if( chunk.idx_solid > 0 ) {
		glDrawElements( GL_TRIANGLES, chunk.idx_solid * 6, GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );
	}

	if( chunk.idx_trans > chunk.idx_solid ) {
		glDisable( GL_CULL_FACE );
		glDrawElements( GL_TRIANGLES, ( chunk.idx_trans - chunk.idx_solid ) * 6, GL_UNSIGNED_INT, BUFFER_OFFSET( chunk.idx_solid * sizeof( ChunkFaceIndices ) ) );
		glEnable( GL_CULL_FACE );
	}

	glBindVertexArray( 0 );
}

void ChunkMgr::chunk_render_shadowmap( Chunk & chunk ) {
	glBindVertexArray( chunk.id_vao );

	if( chunk.idx_solid > 0 ) {
		glDrawElements( GL_TRIANGLES, chunk.idx_solid * 6, GL_UNSIGNED_INT, BUFFER_OFFSET( 0 ) );
	}

	if( chunk.idx_trans > chunk.idx_solid ) {
		glDisable( GL_CULL_FACE );
		glDrawElements( GL_TRIANGLES, ( chunk.idx_trans - chunk.idx_solid ) * 6, GL_UNSIGNED_INT, BUFFER_OFFSET( chunk.idx_solid * sizeof( ChunkFaceIndices ) ) );
		glEnable( GL_CULL_FACE );
	}

	glBindVertexArray( 0 );
}

void ChunkMgr::chunk_vbo( Chunk & chunk ) {
	glGenVertexArrays( 1, &chunk.id_vao );
	glGenBuffers( 1, &chunk.id_vbo );
	glGenBuffers( 1, &chunk.id_ibo );

	glBindVertexArray( chunk.id_vao );
	glBindBuffer( GL_ARRAY_BUFFER, chunk.id_vbo );

	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );

	// Vert
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( ChunkVert ), BUFFER_OFFSET( 0 ) );
	// Color
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( ChunkVert ), BUFFER_OFFSET( 12 ) );
	// Norm
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_TRUE, sizeof( ChunkVert ), BUFFER_OFFSET( 28 ) );
	// UV
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( ChunkVert ), BUFFER_OFFSET( 40 ) );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, chunk.id_ibo );

	//glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
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
			}
		}
	}

	if( chunk ) {
		chunk->pos_lw = pos_lw;
		chunk->hash_lw = hash;
		chunk->hash_lw_2d = Directional::get_hash( glm::ivec2( pos_lw.x, pos_lw.z ) );
		chunk->pos_gw = pos_lw * Chunk::vec_size;

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

		chunk->idx_solid = 0;
		chunk->idx_trans = 0;

		if( chunk->ptr_buffer != nullptr ) {
			put_buffer( chunk->ptr_buffer );
		}

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
}

void ChunkMgr::chunk_shutdown( Chunk & chunk ) { 
	chunk_state( chunk, ChunkState::CS_Save, true );
	chunk_state( chunk, ChunkState::CS_Remove, true );
	chunk.is_shutdown = true;
}

void ChunkMgr::chunk_init( Chunk & chunk ) {
	static float scale_world = 0.006f;
	static float height_world = 50.0f;
	static int seed_world = 1234;

	static float scale_basic = 0.006f;
	static float height_basic = 4.0f;
	static int seed_basic = 101010;

	static float scale_cliffs = 0.0006f;
	static float height_cliffs = 50.0f;
	static int seed_cliffs = 2234;

	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 0;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Init, false );

		std::lock_guard< std::mutex > lock( mtx_noise );
		auto iter_noise = map_noise.find( chunk.hash_lw_2d );

		if( iter_noise == map_noise.end( ) ) {
			iter_noise = map_noise.insert( { chunk.hash_lw_2d, ChunkNoise( ) } ).first;

			chunk.ptr_noise = &iter_noise->second;

			float noise, noise_total;

			for( int i = 0; i < Chunk::size_x; i++ ) {
				for( int j = 0; j < Chunk::size_z; j++ ) {
					noise = 0;
					noise_total = 0;

					noise = raw_noise_2d(
						( seed_world + chunk.pos_gw.x + i ) * scale_world / 5,
						( seed_world + chunk.pos_gw.z + j ) * scale_world / 5 ) * height_world;

					if( noise > 0 ) {
						noise_total += noise;
					}

					noise = raw_noise_2d(
						( seed_basic + chunk.pos_gw.x + i ) * scale_basic,
						( seed_basic + chunk.pos_gw.z + j ) * scale_basic ) * height_basic;

					noise_total += noise;

					noise = raw_noise_2d(
						( seed_basic + chunk.pos_gw.x + i ) * scale_world,
						( seed_basic + chunk.pos_gw.z + j ) * scale_world ) * 2.0f;

					noise_total *= noise;

					chunk.ptr_noise->height[ i ][ j ] = World::level_sea + noise_total;

					noise_total = noise = raw_noise_2d(
						( seed_basic + chunk.pos_gw.x + i ) * scale_basic / 5,
						( seed_basic + chunk.pos_gw.z + j ) * scale_basic / 5 ) * 20.0f;

					if( noise_total > 10.0f ) {
						chunk.ptr_noise->biome[ i ][ j ] = 1;
					}
					else {
						chunk.ptr_noise->biome[ i ][ j ] = 0;
					}
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
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_io( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Read, false );

		glm::ivec3 pos_r, pos_lr;
		Directional::pos_lw_to_r( chunk.pos_lw, pos_r );
		Directional::pos_lw_to_lr( chunk.pos_lw, pos_lr );

		int pos_hash = Directional::get_hash( pos_r );
		{
			//client.time_mgr.begin_record( std::string( "FILE" ) );
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
				Region::size_x * Region::size_z * pos_lr.y +
				Region::size_x * pos_lr.z +
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

				for( int i = 0; i < Chunk::size_y; i++ ) {
					for( int j = 0; j < Chunk::size_z; j++ ) {
						for( int k = 0; k < Chunk::size_x; k++ ) {
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

			//client.time_mgr.end_record( std::string( "FILE" ) );
			//client.time_mgr.push_record( std::string( "FILE" ) );
		}

		if( chunk.is_loaded ) {
			{
				std::lock_guard< std::mutex > lock( chunk.mtx_adj );
				for( int i = 0; i < FD_Size; i++ ) {
					if( chunk.ptr_adj[ i ] != nullptr ) {
						chunk_state( *chunk.ptr_adj[ i ], ChunkState::CS_SMesh, true );
					}
				}
			}

			//chunk_state( chunk, ChunkState::CS_Wait, true );
			chunk_state( chunk, ChunkState::CS_SMesh, true );
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
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Load, false );

		Block & block_grass = get_block_data( "Grass" );
		Block & block_sand = get_block_data( "Sand" );
		Block & block_dirt = get_block_data( "Dirt" );
		Block & block_water = get_block_data( "Water" );

		int noise = 0;
		int biome = 0;

		for( int i = 0; i < Chunk::size_x; i++ ) {
			for( int k = 0; k < Chunk::size_z; k++ ) {
				noise = chunk.ptr_noise->height[ i ][ k ];
				biome = chunk.ptr_noise->biome[ i ][ k ];
				for( int j = 0; j < Chunk::size_y; j++ ) {
					if( biome == 0 ) {
						if( chunk.pos_gw.y + j < noise ) {
							chunk.id_blocks[ i ][ j ][ k ] = block_dirt.id;
						}
						else if( chunk.pos_gw.y + j == noise ) {
							chunk.id_blocks[ i ][ j ][ k ] = block_grass.id;
						}
						else {
							if( chunk.pos_gw.y + j < World::level_sea ) {
								chunk.id_blocks[ i ][ j ][ k ] = block_water.id;
							}
							else {
								chunk.id_blocks[ i ][ j ][ k ] = -1;
							}
						}
					}
					else if( biome == 1 ) {
						if( chunk.pos_gw.y + j < noise ) {
							chunk.id_blocks[ i ][ j ][ k ] = block_sand.id;
						}
						else if( chunk.pos_gw.y + j == noise ) {
							chunk.id_blocks[ i ][ j ][ k ] = block_sand.id;
						}
						else {
							if( chunk.pos_gw.y + j < World::level_sea ) {
								chunk.id_blocks[ i ][ j ][ k ] = block_water.id;
							}
							else {
								chunk.id_blocks[ i ][ j ][ k ] = -1;
							}
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

		//chunk_state( chunk, ChunkState::CS_Wait, true );
		if( chunk.cnt_solid != 0 )
			chunk_state( chunk, ChunkState::CS_Gen, true );

		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_gen( Chunk & chunk ) { 
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 0;
	priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Gen, false );

		SetState state;
		state.iter = map_chunks.end( );
		auto & block_pumpkin = get_block_data( std::string( "Pumpkin" ) );
		auto & block_gleaves = get_block_data( std::string( "Leaves" ) );
		auto & block_dgleaves = get_block_data( std::string( "Pine Leaves" ) );
		auto & block_rleaves = get_block_data( std::string( "Red Leaves" ) );
		auto & block_bleaves = get_block_data( std::string( "Brown Leaves" ) );

		for( int i = 0; i < Chunk::size_x; i++ ) {
			for( int j = 0; j < Chunk::size_z; j++ ) {
				if( chunk.ptr_noise->height[ i ][ j ] > World::level_sea &&
					chunk.ptr_noise->height[ i ][ j ] >= chunk.pos_gw.y &&
					chunk.ptr_noise->height[ i ][ j ] < chunk.pos_gw.y + Chunk::size_y ) {
					if( chunk.ptr_noise->biome[ i ][ j ] == 0 ) {
						if( rand( ) % 1000 < 3 ) {
							set_tree(
								glm::ivec3( chunk.pos_gw.x, 0, chunk.pos_gw.z ) +
								glm::ivec3( i, chunk.ptr_noise->height[ i ][ j ] + 1, j ),
								state, 
								rand( ) % 3 );
						}
						else if( rand( ) % 1000 < 3 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_gleaves.id );
						}
						else if( rand( ) % 1000 < 3 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_dgleaves.id );
						}
						else if( rand( ) % 1000 < 3 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_rleaves.id );
						}
					}
					else if( chunk.ptr_noise->biome[ i ][ j ] == 1 ) { 
						if( rand( ) % 1000 < 2 ) {
							set_tree(
								glm::ivec3( chunk.pos_gw.x, 0, chunk.pos_gw.z ) +
								glm::ivec3( i, chunk.ptr_noise->height[ i ][ j ] + 1, j ),
								state, 
								3 + rand( ) % 2 );
						}
						else if( rand( ) % 1000 < 2 ) {
							set_block(
								glm::ivec3( chunk.pos_gw.x + i, chunk.ptr_noise->height[ i ][ j ] + 1, chunk.pos_gw.z + j ),
								state,
								block_pumpkin.id );
						}
						else if( rand( ) % 1000 < 3 ) {
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
				}
			}
		}

		chunk_state( chunk, ChunkState::CS_SMesh, true );
		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_smesh( Chunk & chunk ) { 
	chunk.is_working = true;

	if( !chunk.ptr_buffer ) { 
		chunk.ptr_buffer = get_buffer( );
	}

	if( chunk.ptr_buffer ) {
		int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
		int priority = 3;
		priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

		client.thread_mgr.task_async( priority, [ & ] ( ) {
			chunk_state( chunk, ChunkState::CS_SMesh, false );

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
			glm::ivec3 pos_adj;
			glm::ivec3 pos_min( 0, 0, 0 );
			glm::ivec3 pos_max( Chunk::size_x - 1, Chunk::size_y - 1, Chunk::size_z - 1 );

			int id_curr, id_adj;

			std::array< glm::ivec3, FaceDirection::FD_Size > list_pos_last;
			std::array< std::pair< int, int >, FaceDirection::FD_Size > list_id_last;
			std::array< int, FaceDirection::FD_Size > list_cnt_last;
			std::array< Block *, FaceDirection::FD_Size > list_block_last;

			FaceDirection dir_face;

			Block * block_curr = nullptr;
			Block * block_adj = nullptr;

			Chunk * chunk_adj;

			chunk.ptr_buffer->list_indices.clear( );
			chunk.ptr_buffer->list_vertices_solid.clear( );
			chunk.ptr_buffer->list_vertices_trans.clear( );
			chunk.ptr_buffer->list_sort.clear( );
			chunk.ptr_buffer->size_solid = 0;
			chunk.ptr_buffer->size_trans = 0;

			for( int iter_face = 0; iter_face < FaceDirection::FD_Size; iter_face++ ) {
				dir_face = ( FaceDirection ) iter_face;
				list_id_last[ dir_face ] = { -1, -1 };
				list_cnt_last[ dir_face ] = 0;
				list_block_last[ dir_face ] = nullptr;
			}

			for( pos_curr.x = 0; pos_curr.x < Chunk::size_x; pos_curr.x++ ) {
				for( pos_curr.y = 0; pos_curr.y < Chunk::size_y; pos_curr.y++ ) {
					for( pos_curr.z = 0; pos_curr.z < Chunk::size_z; pos_curr.z++ ) {
						for( auto iter_face : list_face_z ) {
							dir_face = iter_face;
							id_curr = chunk.id_blocks[ pos_curr.x ][ pos_curr.y ][ pos_curr.z ];
						
							if( id_curr == -1 ) {
								// Is air, skip
								list_id_last[ dir_face ].first = -1;

								continue;
							}

							block_curr = &get_block_data( id_curr );

							if( block_curr->is_trans ) {
								// Is trans, skip
								list_id_last[ dir_face ].first = -1;

								continue;
							}

							pos_adj = pos_curr + Directional::get_vec_dir_i( dir_face );
							chunk_adj = &chunk;

							if( !Directional::is_point_in_region( pos_adj, pos_min, pos_max ) ) {
								pos_adj -= Directional::get_vec_dir_i( dir_face )  * Chunk::vec_size;

								std::lock_guard< std::mutex > lock( chunk.mtx_adj );
								chunk_adj = chunk.ptr_adj[ dir_face ];

								if( chunk_adj == nullptr || !chunk_adj->is_loaded ) {
									// Chunk Aint loaded
									list_id_last[ dir_face ].first = -1;

									continue;
								}
							}

							id_adj = chunk_adj->id_blocks[ pos_adj.x ][ pos_adj.y ][ pos_adj.z ];

							if( id_adj == -1 ||
								( block_adj = &get_block_data( id_adj ) )->is_trans ) {
								// A face is visible!

								if( id_curr == list_id_last[ dir_face ].first ) {
									// We have the same face in a row...
									list_cnt_last[ dir_face ]++;
								}
								else {
									// New face! Lets record some info...
									if( list_cnt_last[ dir_face ] ) {
										put_face(
											chunk.ptr_buffer->list_vertices_solid,
											list_pos_last[ dir_face ],
											list_block_last[ dir_face ]->get_verts( dir_face ),
											glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
											list_block_last[ dir_face ]->color,
											Directional::get_vec_dir_f( dir_face ),
											list_block_last[ dir_face ]->get_uvs( dir_face ),
											glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
									}

									list_id_last[ dir_face ] = { id_curr, id_curr };
									list_pos_last[ dir_face ] = pos_curr;
									list_block_last[ dir_face ] = block_curr;
									list_cnt_last[ dir_face ] = 1;
								}

								continue;
							}

							list_id_last[ dir_face ].first = -1;
						}

						for( auto iter_face : list_face_x ) {
							glm::ivec3 pos_curr_x = { pos_curr.z, pos_curr.y, pos_curr.x };
							dir_face = iter_face;
							id_curr = chunk.id_blocks[ pos_curr_x.x ][ pos_curr_x.y ][ pos_curr_x.z ];


							if( id_curr == -1 ) {
								// Is air, skip
								list_id_last[ dir_face ].first = -1;

								continue;
							}

							block_curr = &get_block_data( id_curr );

							if( block_curr->is_trans ) {
								// Is trans, skip
								list_id_last[ dir_face ].first = -1;

								continue;
							}

							pos_adj = pos_curr_x + Directional::get_vec_dir_i( dir_face );
							chunk_adj = &chunk;


							if( !Directional::is_point_in_region( pos_adj, pos_min, pos_max ) ) {
								pos_adj -= Directional::get_vec_dir_i( dir_face )  * Chunk::vec_size;

								std::lock_guard< std::mutex > lock( chunk.mtx_adj );
								chunk_adj = chunk.ptr_adj[ dir_face ];

								if( chunk_adj == nullptr || !chunk_adj->is_loaded ) {
									// Chunk Aint loaded
									list_id_last[ dir_face ].first = -1;

									continue;
								}
							}

							id_adj = chunk_adj->id_blocks[ pos_adj.x ][ pos_adj.y ][ pos_adj.z ];

							if( id_adj == -1 ||
								( block_adj = &get_block_data( id_adj ) )->is_trans ) {
								// A face is visible!

								if( id_curr == list_id_last[ dir_face ].first ) {
									// We have the same face in a row...
									list_cnt_last[ dir_face ]++;
								}
								else {
									// New face! Lets record some info...

									if( list_cnt_last[ dir_face ] ) {
										put_face(
											chunk.ptr_buffer->list_vertices_solid,
											list_pos_last[ dir_face ],
											list_block_last[ dir_face ]->get_verts( dir_face ),
											glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
											list_block_last[ dir_face ]->color,
											Directional::get_vec_dir_f( dir_face ),
											list_block_last[ dir_face ]->get_uvs( dir_face ),
											glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
									}

									list_id_last[ dir_face ] = { id_curr, id_curr };
									list_pos_last[ dir_face ] = pos_curr_x;
									list_block_last[ dir_face ] = block_curr;
									list_cnt_last[ dir_face ] = 1;
								}

								continue;
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
				if( list_cnt_last[ dir_face ] ) {
					put_face(
						chunk.ptr_buffer->list_vertices_solid,
						list_pos_last[ dir_face ],
						list_block_last[ dir_face ]->get_verts( dir_face ), 
						glm::max( glm::vec3( 1, 1, 1 ), list_scale_verts[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ),
						list_block_last[ dir_face ]->color,
						Directional::get_vec_dir_f( dir_face ),
						list_block_last[ dir_face ]->get_uvs( dir_face ), 
						glm::max( glm::vec2( 1, 1 ), list_scale_uvs[ dir_face ] * ( float ) list_cnt_last[ dir_face ] ) );
				}
			}

			// TRANS
			for( pos_curr.x = 0; pos_curr.x < Chunk::size_x; pos_curr.x++ ) {
				for( pos_curr.y = 0; pos_curr.y < Chunk::size_y; pos_curr.y++ ) {
					for( pos_curr.z = 0; pos_curr.z < Chunk::size_z; pos_curr.z++ ) {
						for( int i = 0; i < FaceDirection::FD_Size; i++ ) {
							dir_face = ( FaceDirection ) i;
							id_curr = chunk.id_blocks[ pos_curr.x ][ pos_curr.y ][ pos_curr.z ];

							if( id_curr == -1 ) {
								continue;
							}

							block_curr = &get_block_data( id_curr );

							if( !block_curr->is_trans ) {
								continue;
							}

							pos_adj = pos_curr + Directional::get_vec_dir_i( dir_face );
							chunk_adj = &chunk;

							if( !Directional::is_point_in_region( pos_adj, pos_min, pos_max ) ) {
								pos_adj -= Directional::get_vec_dir_i( dir_face )  * Chunk::vec_size;
								std::lock_guard< std::mutex > lock( chunk.mtx_adj );
								chunk_adj = chunk.ptr_adj[ dir_face ];
								if( chunk_adj == nullptr || !chunk_adj->is_loaded ) {
									continue;
								}
							}

							id_adj = chunk_adj->id_blocks[ pos_adj.x ][ pos_adj.y ][ pos_adj.z ];

							if( id_adj == -1 ||
								( ( block_adj = &get_block_data( id_adj ) )->is_trans && id_adj != id_curr ) ) {

								put_face(
									chunk.ptr_buffer->list_vertices_trans,
									pos_curr,
									block_curr->get_verts( dir_face ),
									block_curr->color,
									Directional::get_vec_dir_f( dir_face ),
									block_curr->get_uvs( dir_face ) );

								chunk.ptr_buffer->list_sort.emplace_back( 
									std::pair< float, int > { 
										glm::distance( glm::vec3( chunk.pos_gw + pos_curr ) + Block::get_center( dir_face ), client.display_mgr.camera.pos_camera ), 
										chunk.ptr_buffer->list_vertices_trans.size( ) - 1
									} 
								);

								continue;
							}
						}
					}
				}
			}

			for( int unsigned i = 0; i < chunk.ptr_buffer->list_vertices_solid.size( ); i++ ) { 
				chunk.ptr_buffer->list_indices.emplace_back(
					ChunkFaceIndices {
						i * 4 + 0, i * 4 + 1, i * 4 + 2,
						i * 4 + 2, i * 4 + 3, i * 4 + 0
					}
				);
			}

			chunk.ptr_buffer->size_solid = chunk.ptr_buffer->list_vertices_solid.size( );

			std::sort( chunk.ptr_buffer->list_sort.begin( ), chunk.ptr_buffer->list_sort.end( ),
				[ ] ( std::pair< float, int > const & lho, std::pair< float, int > const & rho ) {
				return lho.first > rho.first;
			} );

			for( auto & pair_sort : chunk.ptr_buffer->list_sort ) {
				chunk.ptr_buffer->list_vertices_solid.emplace_back( chunk.ptr_buffer->list_vertices_trans[ pair_sort.second ] );
			}

			for( int unsigned i = chunk.ptr_buffer->size_solid; i < chunk.ptr_buffer->list_vertices_solid.size( ); i++ ) {
				chunk.ptr_buffer->list_indices.emplace_back(
					ChunkFaceIndices {
						i * 4 + 0, i * 4 + 1, i * 4 + 2,
						i * 4 + 2, i * 4 + 3, i * 4 + 0
					}
				);
			}

			chunk.ptr_buffer->size_trans = chunk.ptr_buffer->list_vertices_solid.size( );

			if( chunk.ptr_buffer->size_trans > 0 ) { 
				chunk_state( chunk, ChunkState::CS_Buffer, true );
			}
			else {
				auto iter = map_render.end( );
				put_buffer( chunk.ptr_buffer );
				chunk.idx_solid = 0;
				chunk.idx_trans = 0;
				std::unique_lock< std::recursive_mutex > lock( mtx_render );
				map_render.erase( chunk.hash_lw );
			}

			chunk.is_working = false;
		} );
	}
	else { 
		chunk.is_working = false;
	}
}

void ChunkMgr::chunk_tmesh( Chunk & chunk ) { 
	chunk.is_working = true;

	client.thread_mgr.task_async( 5, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_TMesh, false );

		chunk_state( chunk, ChunkState::CS_Buffer, true );
		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_buffer( Chunk & chunk ) { 
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 10;
	//priority = priority + ( 1.0f - float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_main( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Buffer, false );

		if( chunk.ptr_buffer ) {
			glBindBuffer( GL_ARRAY_BUFFER, chunk.id_vbo );
			glBufferData( GL_ARRAY_BUFFER, chunk.idx_trans * sizeof( ChunkFaceVertices ), nullptr, GL_STATIC_DRAW );
			glBufferData( GL_ARRAY_BUFFER, chunk.ptr_buffer->size_trans * sizeof( ChunkFaceVertices ), chunk.ptr_buffer->list_vertices_solid.data( ), GL_STATIC_DRAW );

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, chunk.id_ibo );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, chunk.idx_trans * sizeof( ChunkFaceIndices ), nullptr, GL_STATIC_DRAW );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, chunk.ptr_buffer->size_trans * sizeof( ChunkFaceIndices ), chunk.ptr_buffer->list_indices.data( ), GL_STATIC_DRAW );

			chunk.idx_solid = chunk.ptr_buffer->size_solid;
			chunk.idx_trans = chunk.ptr_buffer->size_trans;

			put_buffer( chunk.ptr_buffer );
		}

		if( chunk.idx_trans != 0 ) {
			std::lock_guard< std::recursive_mutex > lock( mtx_render );
			map_render.insert( { chunk.hash_lw, chunk } );
		}

		chunk.is_working = false;
	} );
}

void ChunkMgr::chunk_save( Chunk & chunk ) {
	using namespace std::tr2::sys;
	chunk.is_working = true;

	int max_dir = Directional::get_max( pos_center_chunk_lw - chunk.pos_lw );
	int priority = 2;
	priority = priority + ( float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_io( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Save, false );

		glm::ivec3 pos_lr;
		Directional::pos_lw_to_lr( chunk.pos_lw, pos_lr );

		{
			//client.time_mgr.begin_record( std::string( "SAVE" ) );
			std::lock_guard< std::mutex > lock( mtx_file );

			int index_region = Region::size_x * Region::size_z * pos_lr.y +
				Region::size_x * pos_lr.z + pos_lr.x;

			int index_section;
			int index_start;
			int num_block, id;
			int index_buffer;

			id = chunk.id_blocks[ 0 ][ 0 ][ 0 ];
			num_block = 0;
			index_buffer = 0;
			index_section = 0;

			for( int i = 0; i < Chunk::size_y; i++ ) {
				for( int j = 0; j < Chunk::size_z; j++ ) {
					for( int k = 0; k < Chunk::size_x; k++ ) {
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
	priority = priority + ( float( max_dir ) / Directional::get_max( World::size_vect ) ) * ( client.thread_mgr.get_max_prio( ) / 2 );

	client.thread_mgr.task_async( priority, [ & ] ( ) {
		chunk_state( chunk, ChunkState::CS_Remove, false );

		if( chunk.ptr_buffer ) { 
			put_buffer( chunk.ptr_buffer );
		}

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

	printTabbedLine( 1, "Loading blocks..." );

	client.gui_mgr.print_to_console( std::string( "Loading block data:" ) );

	path path_blocks( "./Blocks" );
	int index = 0;
	int succ = 0;

	if( !exists( path_blocks ) ) {
		create_directory( path_blocks );
	}

	for( directory_iterator dir_iter( path_blocks ); dir_iter != directory_iterator( ); dir_iter++ ) {
		if( is_directory( dir_iter->status( ) ) ) {
			std::string name_block = dir_iter->path( ).filename( ).string( );
			std::string path_desc = dir_iter->path( ).string( ) + "\\" + name_block + ".desc";
			std::ifstream in_file( path_desc );
			int id = list_block_data.size();

			if( in_file.is_open( ) ) {
				std::string line; line.resize( 128 );
				std::string token;
				int pos_start, pos_end;

				Block block( id, name_block );

				for( int i = 0; i < FD_Size; i++ ) {
					block.uvs[ i ] = client.texture_mgr.get_uvs_block( id, 0 );
					block.orientation[ i ] = 0;
				}

				while( std::getline( in_file, line ) ) {
					pos_start = 0; pos_end = 0;
					pos_end = line.find( ":" );

					if( pos_end != std::string::npos ) {
						token = line.substr( pos_start, pos_end - pos_start );

						if( token == "Color" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.color.r = atof( token.data( ) );

							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.color.g = atof( token.data( ) );

							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.color.b = atof( token.data( ) );

							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.color.a = atof( token.data( ) );
						}
						else if( token == "Texture Front" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.uvs[ FD_Front ] = client.texture_mgr.get_uvs_block( id, atoi( token.data( ) ) );
						}
						else if( token == "Texture Back" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.uvs[ FD_Back ] = client.texture_mgr.get_uvs_block( id, atoi( token.data( ) ) );
						}
						else if( token == "Texture Left" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.uvs[ FD_Left ] = client.texture_mgr.get_uvs_block( id, atoi( token.data( ) ) );
						}
						else if( token == "Texture Right" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.uvs[ FD_Right ] = client.texture_mgr.get_uvs_block( id, atoi( token.data( ) ) );
						}
						else if( token == "Texture Up" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.uvs[ FD_Up ] = client.texture_mgr.get_uvs_block( id, atoi( token.data( ) ) );
						}
						else if( token == "Texture Down" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.uvs[ FD_Down ] = client.texture_mgr.get_uvs_block( id, atoi( token.data( ) ) );
						}
						else if( token == "Orientation Front" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.orientation[ FD_Front ] = atoi( token.data( ) );
						}
						else if( token == "Orientation Back" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.orientation[ FD_Back ] = atoi( token.data( ) );
						}
						else if( token == "Orientation Left" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.orientation[ FD_Left ] = atoi( token.data( ) );
						}
						else if( token == "Orientation Right" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.orientation[ FD_Right ] = atoi( token.data( ) );
						}
						else if( token == "Orientation Up" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.orientation[ FD_Up ] = atoi( token.data( ) );
						}
						else if( token == "Orientation Down" ) {
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							block.orientation[ FD_Down ] = atoi( token.data( ) );
						}
						else if( token == "Visibility" ) { 
							pos_start = line.find( " ", pos_end );
							pos_end = line.find( ",", pos_start );
							pos_start += 1;
							token = line.substr( pos_start, pos_end - pos_start );
							if( token == "Transparent" ) { 
								block.is_trans = true;
							}
						}
					}
				}

				list_block_data.push_back( block );
				map_block_data.insert( std::make_pair( name_block, id ) );

				out.str( "" );
				out << "SUCCESS: " << path_desc;
				client.gui_mgr.print_to_console( out.str( ) );
				printTabbedLine( 1, out.str( ) );

				succ++;

				in_file.close( );
			}
			else {
				Block block( id, name_block );

				for( int i = 0; i < FD_Size; i++ ) {
					block.uvs[ i ] = client.texture_mgr.get_uvs_block( id, 0 );
				}

				list_block_data.push_back( block );

				out.str( "" );
				out << "ERROR: " << path_desc;
				client.gui_mgr.print_to_console( out.str( ) );
				printTabbedLine( 1, out.str( ) );
			}
		}
		index++;
	}

	out.str( "" );
	out << "Loaded: " << succ << " Total: " << index;
	client.gui_mgr.print_to_console( out.str( ) );

	printTabbedLine( 1, out.str( ) );
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
		}
		else if( pos_lc.x == Chunk::size_x - 1 && chunk.ptr_adj[ FD_Left ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Left ], ChunkState::CS_SMesh, true );
		}

		if( pos_lc.y == 0 && chunk.ptr_adj[ FD_Down ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Down ], ChunkState::CS_SMesh, true );
		}
		else if( pos_lc.y == Chunk::size_y - 1 && chunk.ptr_adj[ FD_Up ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Up ], ChunkState::CS_SMesh, true );
		}

		if( pos_lc.z == 0 && chunk.ptr_adj[ FD_Back ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Back ], ChunkState::CS_SMesh, true );
		}
		else if( pos_lc.z == Chunk::size_z - 1 && chunk.ptr_adj[ FD_Front ] != nullptr ) {
			chunk_state( *chunk.ptr_adj[ FD_Front ], ChunkState::CS_SMesh, true );
		}
	}

	chunk_state( chunk, ChunkState::CS_SMesh, true );
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

			state.map_queue_dirty.insert( { Directional::get_hash( state.pos_lw ), chunk } );
			std::lock_guard< std::mutex > lock( chunk.mtx_adj );
			if( state.pos_lc.x == 0 && chunk.ptr_adj[ FD_Right ] != nullptr ) {
				state.map_queue_dirty.insert( {
					Directional::get_hash( chunk.ptr_adj[ FD_Right ]->pos_lw ),
					*chunk.ptr_adj[ FD_Right ] } );
			}
			else if( state.pos_lc.x == Chunk::size_x - 1 && chunk.ptr_adj[ FD_Left ] != nullptr ) {
				state.map_queue_dirty.insert( {
					Directional::get_hash( chunk.ptr_adj[ FD_Left ]->pos_lw ),
					*chunk.ptr_adj[ FD_Left ] } );
			}

			if( state.pos_lc.y == 0 && chunk.ptr_adj[ FD_Down ] != nullptr ) {
				state.map_queue_dirty.insert( {
					Directional::get_hash( chunk.ptr_adj[ FD_Down ]->pos_lw ),
					*chunk.ptr_adj[ FD_Down ] } );
			}
			else if( state.pos_lc.y == Chunk::size_y - 1 && chunk.ptr_adj[ FD_Up ] != nullptr ) {
				state.map_queue_dirty.insert( {
					Directional::get_hash( chunk.ptr_adj[ FD_Up ]->pos_lw ),
					*chunk.ptr_adj[ FD_Up ] } );
			}

			if( state.pos_lc.z == 0 && chunk.ptr_adj[ FD_Back ] != nullptr ) {
				state.map_queue_dirty.insert( {
					Directional::get_hash( chunk.ptr_adj[ FD_Back ]->pos_lw ),
					*chunk.ptr_adj[ FD_Back ] } );
			}
			else if( state.pos_lc.z == Chunk::size_z - 1 && chunk.ptr_adj[ FD_Front ] != nullptr ) {
				state.map_queue_dirty.insert( {
					Directional::get_hash( chunk.ptr_adj[ FD_Front ]->pos_lw ),
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
						Directional::get_hash( chunk.ptr_adj[ FD_Right ]->pos_lw ),
						*chunk.ptr_adj[ FD_Right ] } );
				}
				else if( state.pos_lc.x == Chunk::size_x - 1 && chunk.ptr_adj[ FD_Left ] != nullptr ) {
					state.map_queue_dirty.insert( {
						Directional::get_hash( chunk.ptr_adj[ FD_Left ]->pos_lw ),
						*chunk.ptr_adj[ FD_Left ] } );
				}

				if( state.pos_lc.y == 0 && chunk.ptr_adj[ FD_Down ] != nullptr ) {
					state.map_queue_dirty.insert( {
						Directional::get_hash( chunk.ptr_adj[ FD_Down ]->pos_lw ),
						*chunk.ptr_adj[ FD_Down ] } );
				}
				else if( state.pos_lc.y == Chunk::size_y - 1 && chunk.ptr_adj[ FD_Up ] != nullptr ) {
					state.map_queue_dirty.insert( {
						Directional::get_hash( chunk.ptr_adj[ FD_Up ]->pos_lw ),
						*chunk.ptr_adj[ FD_Up ] } );
				}

				if( state.pos_lc.z == 0 && chunk.ptr_adj[ FD_Back ] != nullptr ) {
					state.map_queue_dirty.insert( {
						Directional::get_hash( chunk.ptr_adj[ FD_Back ]->pos_lw ),
						*chunk.ptr_adj[ FD_Back ] } );
				}
				else if( state.pos_lc.z == Chunk::size_z - 1 && chunk.ptr_adj[ FD_Front ] != nullptr ) {
					state.map_queue_dirty.insert( {
						Directional::get_hash( chunk.ptr_adj[ FD_Front ]->pos_lw ),
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

		auto & block_leaves = get_block_data( std::string( "Leaves" ) );
		auto & block_log = get_block_data( std::string( "Log" ) );
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

		auto & block_leaves = get_block_data( std::string( "Pine Leaves" ) );
		auto & block_log = get_block_data( std::string( "Birch Log" ) );

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

		auto & block_leaves = get_block_data( std::string( "Red Leaves" ) );
		auto & block_log = get_block_data( std::string( "Birch Log" ) );

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

		auto & block_log = get_block_data( std::string( "Cactus" ) );

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

		auto & block_leaves = get_block_data( std::string( "Brown Leaves" ) );
		auto & block_log = get_block_data( std::string( "Birch Log" ) );

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
	auto & block_water = get_block_data( std::string( "Water" ) );
	auto & block_tnt = get_block_data( std::string( "Tnt" ) );

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
	return list_block_data.size( );
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
	int max_world = Directional::get_max( World::size_vect );
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

void put_face( 
	std::vector< ChunkFaceVertices > & buffer_verts, glm::ivec3 const & pos, 
	FaceVerts const & verts, Color4 const & color, 
	glm::vec3 const & normal, FaceUvs const & uvs ) {

	buffer_verts.emplace_back( 
		ChunkFaceVertices { {
			{ { pos.x + verts[ 0 ][ 0 ], pos.y + verts[ 0 ][ 1 ], pos.z + verts[ 0 ][ 2 ] },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 0 ][ 0 ], uvs[ 0 ][ 1 ], uvs[ 0 ][ 2 ] } },

			{ { pos.x + verts[ 1 ][ 0 ], pos.y + verts[ 1 ][ 1 ], pos.z + verts[ 1 ][ 2 ] },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 1 ][ 0 ], uvs[ 1 ][ 1 ], uvs[ 1 ][ 2 ] } },

			{ { pos.x + verts[ 2 ][ 0 ], pos.y + verts[ 2 ][ 1 ], pos.z + verts[ 2 ][ 2 ] },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 2 ][ 0 ], uvs[ 2 ][ 1 ], uvs[ 2 ][ 2 ] } },

			{ { pos.x + verts[ 3 ][ 0 ], pos.y + verts[ 3 ][ 1 ], pos.z + verts[ 3 ][ 2 ] },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 3 ][ 0 ], uvs[ 3 ][ 1 ], uvs[ 3 ][ 2 ] } }
		} } 
	);
}

void put_face( 
	std::vector< ChunkFaceVertices > & buffer_verts, glm::ivec3 const & pos,
	FaceVerts const & verts, glm::ivec3 const & scale_verts,
	Color4 const & color, glm::vec3 const & normal, 
	FaceUvs const & uvs, glm::ivec2 const & scale_uvs ) {

	buffer_verts.emplace_back(
		ChunkFaceVertices { {
			{ { pos.x + verts[ 0 ][ 0 ] * scale_verts.x, pos.y + verts[ 0 ][ 1 ] * scale_verts.y, pos.z + verts[ 0 ][ 2 ] * scale_verts.z },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 0 ][ 0 ] * scale_uvs.x, uvs[ 0 ][ 1 ] * scale_uvs.y, uvs[ 0 ][ 2 ] } },

			{ { pos.x + verts[ 1 ][ 0 ] * scale_verts.x, pos.y + verts[ 1 ][ 1 ] * scale_verts.y, pos.z + verts[ 1 ][ 2 ] * scale_verts.z },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 1 ][ 0 ] * scale_uvs.x, uvs[ 1 ][ 1 ] * scale_uvs.y, uvs[ 1 ][ 2 ] } },

			{ { pos.x + verts[ 2][ 0 ] * scale_verts.x, pos.y + verts[ 2 ][ 1 ] * scale_verts.y, pos.z + verts[ 2 ][ 2 ] * scale_verts.z },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 2 ][ 0 ] * scale_uvs.x, uvs[ 2 ][ 1 ] * scale_uvs.y, uvs[ 2 ][ 2 ] } },

			{ { pos.x + verts[ 3 ][ 0 ] * scale_verts.x, pos.y + verts[ 3 ][ 1 ] * scale_verts.y, pos.z + verts[ 3 ][ 2 ] * scale_verts.z },
			{ color.r, color.g, color.b, color.a },
			{ normal.x, normal.y, normal.z },
			{ uvs[ 3 ][ 0 ] * scale_uvs.x, uvs[ 3 ][ 1 ] * scale_uvs.y, uvs[ 3 ][ 2 ] } }
			} }
	);
}