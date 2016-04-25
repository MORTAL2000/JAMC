#include "TextureMgr.h"

#include "Client.h"

#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

int const TextureMgr::size_terrain_mip_map = 4;
int const TextureMgr::size_terrain_texture_padding = pow( 2, 4 );
int const TextureMgr::size_terrain_texture = 64;
int const TextureMgr::size_terrain_atlas = ( TextureMgr::size_terrain_texture + 2 * TextureMgr::size_terrain_texture_padding ) * 16 ;

TextureMgr::TextureMgr( Client & client ) :
	Manager( client ) { }

TextureMgr::~TextureMgr( ) { }

void TextureMgr::load_terrain( ) {
	using namespace std::tr2::sys;
	auto & out = client.display_mgr.out;

	// Lets allocate some texture space on the gfx card for atlas and temp space for copying
	glGenTextures( 1, &id_temp );
	glBindTexture( GL_TEXTURE_2D, id_temp );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, size_terrain_texture, size_terrain_texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

	glGenTextures( 1, &id_terrain );
	glBindTexture( GL_TEXTURE_2D, id_terrain );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, size_terrain_atlas, size_terrain_atlas, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexStorage2D( GL_TEXTURE_2D, size_terrain_mip_map, GL_RGBA8, size_terrain_atlas, size_terrain_atlas );

	// Lets start with our block directory
	path path_blocks( "./Blocks" );

	int index = 0;
	int succ = 0;
	int tex_succ = 0;
	int x, y;
	int padding = 0;
	float pixel_correct = 0.5f;

	// Lets iterate all the blocks in the block directory
	if( !exists( path_blocks ) ) {
		create_directory( path_blocks );
	}

	for( directory_iterator dir_iter_blocks( path_blocks ); dir_iter_blocks != directory_iterator( ); dir_iter_blocks++ ) {
		if( is_directory( dir_iter_blocks->status( ) ) ) {
			uvs_terrain.push_back( std::vector< face_uvs >( ) );
			tex_succ = 0;

			// Lets iterate all the png files in each block directory
			for( directory_iterator dir_iter_texture( dir_iter_blocks->path( ) ); dir_iter_texture != directory_iterator( ); dir_iter_texture++ ) {
				if( is_regular_file( dir_iter_texture->path( ) ) && dir_iter_texture->path( ).extension( ).string( ) == ".png" ) {

					// Load temp texture from each png
					id_temp = SOIL_load_OGL_texture(
						dir_iter_texture->path( ).string( ).c_str( ),
						SOIL_LOAD_RGBA,
						id_temp,
						SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_INVERT_Y );

					// If temp texutre was loaded
					if( id_temp != 0 ) {
						x = ( index * ( size_terrain_texture + 2 * size_terrain_texture_padding ) ) % size_terrain_atlas;
						y = ( ( index * ( size_terrain_texture + 2 * size_terrain_texture_padding ) ) / size_terrain_atlas )
							* ( size_terrain_texture + 2 * size_terrain_texture_padding );

						// Copy the data to texture atlas
						glCopyImageSubData(
							id_temp, GL_TEXTURE_2D, 0,
							0, 0, 0,
							id_terrain, GL_TEXTURE_2D, 0,
							x + size_terrain_texture_padding,
							y + size_terrain_texture_padding,
							0,
							size_terrain_texture, size_terrain_texture, 1 );

						for( int i = 0; i < size_terrain_texture_padding; i++ ) {
							glCopyImageSubData(
								id_temp, GL_TEXTURE_2D, 0,
								0, 0, 0,
								id_terrain, GL_TEXTURE_2D, 0,
								x + i,
								y + size_terrain_texture_padding,
								0,
								1, size_terrain_texture, 1 );

							glCopyImageSubData(
								id_temp, GL_TEXTURE_2D, 0,
								size_terrain_texture - 1, 0, 0,
								id_terrain, GL_TEXTURE_2D, 0,
								x + size_terrain_texture_padding + size_terrain_texture + i,
								y + size_terrain_texture_padding,
								0,
								1, size_terrain_texture, 1 );

							glCopyImageSubData(
								id_temp, GL_TEXTURE_2D, 0,
								0, 0, 0,
								id_terrain, GL_TEXTURE_2D, 0,
								x + size_terrain_texture_padding,
								y + i,
								0,
								size_terrain_texture, 1, 1 );

							glCopyImageSubData(
								id_temp, GL_TEXTURE_2D, 0,
								0, size_terrain_texture - 1, 0,
								id_terrain, GL_TEXTURE_2D, 0,
								x + size_terrain_texture_padding,
								y + size_terrain_texture_padding + size_terrain_texture + i,
								0,
								size_terrain_texture, 1, 1 );

							for( int j = 0; j < size_terrain_texture_padding; j++ ) {
								glCopyImageSubData(
									id_temp, GL_TEXTURE_2D, 0,
									0, 0, 0,
									id_terrain, GL_TEXTURE_2D, 0,
									x + i,
									y + j,
									0,
									1, 1, 1 );

								glCopyImageSubData(
									id_temp, GL_TEXTURE_2D, 0,
									size_terrain_texture - 1, 0, 0,
									id_terrain, GL_TEXTURE_2D, 0,
									x + size_terrain_texture_padding + size_terrain_texture + i,
									y + j,
									0,
									1, 1, 1 );

								glCopyImageSubData(
									id_temp, GL_TEXTURE_2D, 0,
									0, size_terrain_texture - 1, 0,
									id_terrain, GL_TEXTURE_2D, 0,
									x + i,
									y + size_terrain_texture_padding + size_terrain_texture + j,
									0,
									1, 1, 1 );

								glCopyImageSubData(
									id_temp, GL_TEXTURE_2D, 0,
									size_terrain_texture - 1, size_terrain_texture - 1, 0,
									id_terrain, GL_TEXTURE_2D, 0,
									x + size_terrain_texture_padding + size_terrain_texture + i,
									y + size_terrain_texture_padding + size_terrain_texture + j,
									0,
									1, 1, 1 );
							}
						}

						// Lets build the UVS per texture in the atlas map
						auto & uvs_current = uvs_terrain.back( );
						uvs_current.push_back(
							face_uvs { {
								{ ( float( x + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas,
								( float( y + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas },
								{ ( float( x + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas,
								( float( y + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas },
								{ ( float( x + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas,
								( float( y + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas },
								{ ( float( x + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas,
								( float( y + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas }
							} } 
						);

						succ++; tex_succ++;
						index++;

						out.str( "" );
						out << "SUCCESS: " << dir_iter_texture->path( ).string( );
						client.gui_mgr.print_to_console( out.str( ) );
						printTabbedLine( 2, out.str( ) );
					}
					else {
						out.str( "" );
						out << "ERROR: " << dir_iter_texture->path( ).string( );
						client.gui_mgr.print_to_console( out.str( ) );
						printTabbedLine( 2, out.str( ) );
					}
				}
			}

			if( tex_succ == 0 ) {
				x = ( index * ( size_terrain_texture + 2 * size_terrain_texture_padding ) ) % size_terrain_atlas;
				y = ( ( index * ( size_terrain_texture + 2 * size_terrain_texture_padding ) ) / size_terrain_atlas )
					* ( size_terrain_texture + 2 * size_terrain_texture_padding );

				auto & uvs_current = uvs_terrain.back( );
				uvs_current.push_back(
					face_uvs { {
						{ ( float( x + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas,
						( float( y + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas },
						{ ( float( x + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas,
						( float( y + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas },
						{ ( float( x + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas,
						( float( y + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas },
						{ ( float( x + size_terrain_texture_padding + padding ) + pixel_correct ) / size_terrain_atlas,
						( float( y + size_terrain_texture_padding + size_terrain_texture - padding ) - pixel_correct ) / size_terrain_atlas }
						} } );

				out.str( "" );
				out << "ERROR: There is no texture for " << dir_iter_blocks->path( ).filename( ).string( );
				client.gui_mgr.print_to_console( out.str( ) );
				printTabbedLine( 2, out.str( ) );

				index++;
			}
		}
	}

	glBindTexture( GL_TEXTURE_2D, id_terrain );

	glGenerateMipmap( GL_TEXTURE_2D );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	out.str( "" );
	out << "Loaded: " << succ << " Total: " << index;
	client.gui_mgr.print_to_console( out.str( ) );
	client.gui_mgr.print_to_console( std::string( "" ) );
	printTabbedLine( 2, out.str( ) );
}

void TextureMgr::load_skybox( ) { 
	int index = 0;
	int succ = 0;
	int tex_succ = 0;
	int x, y;
	int padding = 0;
	float pixel_correct = 0.5f;

	glm::ivec2 dim_skybox( 3600, 2700 );
	glm::ivec2 dim_skyface = dim_skybox / glm::ivec2( 4, 3 );

	id_skybox = SOIL_load_OGL_texture(
		".\\Skybox\\Skybox1.png",
		SOIL_LOAD_RGBA,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y );

	x = dim_skyface.x * 1;
	y = dim_skyface.y * 1;

	uvs_skybox.push_back( { {
		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y }
		} } );

	x = dim_skyface.x * 3;
	y = dim_skyface.y * 1;

	uvs_skybox.push_back( { {
		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y }
		} } );

	x = dim_skyface.x * 2;
	y = dim_skyface.y * 1;

	uvs_skybox.push_back( { {
		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y }
		} } );

	x = dim_skyface.x * 0;
	y = dim_skyface.y * 1;

	uvs_skybox.push_back( { {
		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y }
		} } );

	x = dim_skyface.x * 1;
	y = dim_skyface.y * 2;

	uvs_skybox.push_back( { {
		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y }
		} } );

	x = dim_skyface.x * 1;
	y = dim_skyface.y * 0;

	uvs_skybox.push_back( { {
		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y }
		} } );
}

void TextureMgr::load_sun( ) { 
	int index = 0;
	int succ = 0;
	int tex_succ = 0;
	int x, y;
	int padding = 0;
	float pixel_correct = 0.5f;

	glm::ivec2 dim_skybox( 3600, 2700 );
	glm::ivec2 dim_skyface = dim_skybox / glm::ivec2( 4, 3 );

	x = 0; y = 0;

	uvs_sun = { {
		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + padding ) + pixel_correct ) / dim_skybox.y },

		{ ( float( x + dim_skyface.x - padding ) - pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y },

		{ ( float( x + padding ) + pixel_correct ) / dim_skybox.x,
		( float( y + dim_skyface.y - padding ) - pixel_correct ) / dim_skybox.y }
		} };
}

void TextureMgr::load_fonts( ) { 
	int index = 0;
	int succ = 0;
	int tex_succ = 0;
	int x, y;
	float offset_x = 0;
	float offset_y = -19.5f;
	int padding_x = 27;
	int padding_y = 46;
	float pixel_correct = 0.5f;

	glm::ivec2 dim_fonts( 860, 1400 );
	glm::ivec2 dim_char = dim_fonts / 10;

	glGenTextures( 1, &id_fonts );
	glBindTexture( GL_TEXTURE_2D, id_fonts );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, dim_fonts.x, dim_fonts.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

	id_fonts = SOIL_load_OGL_texture(
		".\\Fonts\\Font1.png",
		SOIL_LOAD_RGBA,
		id_fonts,
		SOIL_FLAG_INVERT_Y );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	for( int i = 0; i < ( 176 - 32 ); i++ ) { 
		x = ( i * dim_char.x ) % dim_fonts.x;
		y = dim_fonts.y - dim_char.y - ( ( i * dim_char.x ) / dim_fonts.x ) * ( dim_char.y );

		uvs_fonts.push_back(
			face_uvs { {
				{ ( float( x ) + offset_x + padding_x + pixel_correct ) / dim_fonts.x,
				( float( y ) + offset_y + padding_y + pixel_correct ) / dim_fonts.y },

				{ ( float( x ) + offset_x + dim_char.x - padding_x - pixel_correct ) / dim_fonts.x,
				( float( y ) + offset_y + padding_y + pixel_correct ) / dim_fonts.y },

				{ ( float( x ) + offset_x + dim_char.x - padding_x - pixel_correct ) / dim_fonts.x,
				( float( y ) + offset_y + dim_char.y - padding_y - pixel_correct ) / dim_fonts.y },

				{ ( float( x ) + offset_x + padding_x + pixel_correct ) / dim_fonts.x,
				( float( y ) + offset_y + dim_char.y - padding_y - pixel_correct ) / dim_fonts.y }
			} } );
	}
}

void TextureMgr::load_materials( ) { 
	glGenTextures( 1, &id_materials );
	glBindTexture( GL_TEXTURE_2D, id_materials );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

	id_materials = SOIL_load_OGL_texture(
		".\\Materials\\Solid.png",
		SOIL_LOAD_RGBA,
		id_materials,
		SOIL_FLAG_INVERT_Y );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	uvs_materials = { {
		{ 0, 0 },
		{ 1, 0 },
		{ 1, 1 },
		{ 0, 1 }
	} };
}

void TextureMgr::init( ) {
	printTabbedLine( 0, "Init TextureMgr..." );

	printTabbedLine( 1, "Loading textures..." );

	client.gui_mgr.print_to_console( "Loading Raw Textures and UVs:" );

	//client.gui_mgr.print_to_console( std::string( "Loading Raw Textures and UVs:" ) );

	load_terrain( );
	load_skybox( );
	load_sun( );
	load_fonts( );
	load_materials( );

	printTabbedLine( 2, checkGlErrors( ) );

	printTabbedLine( 1, "...Loading textures" );

	printTabbedLine( 0, "...Init TextureMgr" );
	std::cout << std::endl;

	load_shader( "./Shaders/Test.vs", "./Shaders/Test.fs", id_prog );
	glUseProgram( id_prog );
	int prog_sampler = glGetUniformLocation( id_prog, "frag_sampler" );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, id_terrain );
	glUniform1i( prog_sampler, 0 );

	glActiveTexture( GL_TEXTURE0 );

	int prog_fade_max = glGetUniformLocation( id_prog, "fade_max" );
	glUniform1f( prog_fade_max, Chunk::size_x * World::size_x - Chunk::size_x / 2.0f );

	int prog_fade_min = glGetUniformLocation( id_prog, "fade_min" );
	glUniform1f( prog_fade_min, Chunk::size_x * World::size_x - Chunk::size_x * 1.5f );
}

void TextureMgr::update( ) {
	use_prog( );

	GLuint prog_camera = glGetUniformLocation( id_prog, "pos_camera" );
	GLuint prog_mat_view = glGetUniformLocation( id_prog, "mat_view" );
	GLuint prog_mat_perspective = glGetUniformLocation( id_prog, "mat_perspective" );
	GLuint prog_time = glGetUniformLocation( id_prog, "time" );

	glUniform3f( prog_camera, client.display_mgr.camera.pos_camera.x, client.display_mgr.camera.pos_camera.y, client.display_mgr.camera.pos_camera.z );
	glUniformMatrix4fv( prog_mat_view, 1, GL_FALSE, glm::value_ptr( client.display_mgr.camera.mat_view ) );
	glUniformMatrix4fv( prog_mat_perspective, 1, GL_FALSE, glm::value_ptr( client.display_mgr.camera.mat_perspective ) );
	glUniform1f( prog_time, client.time_mgr.get_time( TimeStrings::GAME ) );
}

void TextureMgr::read_file( std::string const & path_file, std::string & data ) {
	std::ifstream fileStream( path_file, std::ios::in );

	if( !fileStream.is_open( ) ) {
		std::cout << "Could not read file " << path_file << ". File does not exist." << std::endl;
		return;
	}

	std::string line = "";
	while( !fileStream.eof( ) ) {
		std::getline( fileStream, line );
		data.append( line + "\n" );
	}

	fileStream.close( );
}

void TextureMgr::load_vert_shader( std::string const & path_file, GLuint & id_vert ) {
	std::string data_vert;
	read_file( path_file, data_vert );

	char const * ptr_data_vert = data_vert.c_str( );
	GLint result = GL_FALSE;
	int length;

	// Compile vertex shader
	std::cout << "Compiling vertex shader." << std::endl;
	id_vert = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( id_vert, 1, &ptr_data_vert, NULL );
	glCompileShader( id_vert );

	// Check vertex shader
	glGetShaderiv( id_vert, GL_COMPILE_STATUS, &result );
	glGetShaderiv( id_vert, GL_INFO_LOG_LENGTH, &length );

	if( length ) {
		std::vector< char > error_vert( ( length > 1 ) ? length : 1 );
		glGetShaderInfoLog( id_vert, length, NULL, &error_vert[ 0 ] );
		std::cout << &error_vert[ 0 ] << std::endl;
	}
}

void TextureMgr::load_frag_shader( std::string const & path_file, GLuint & id_frag ) {
	std::string data_frag;
	read_file( path_file, data_frag );

	char const * ptr_data_frag = data_frag.c_str( );
	GLint result = GL_FALSE;
	int length;

	// Compile fragment shader
	std::cout << "Compiling fragment shader." << std::endl;
	id_frag = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( id_frag, 1, &ptr_data_frag, NULL );
	glCompileShader( id_frag );

	// Check fragment shader
	glGetShaderiv( id_frag, GL_COMPILE_STATUS, &result );
	glGetShaderiv( id_frag, GL_INFO_LOG_LENGTH, &length );

	if( length ) {
		std::vector< char > error_frag( ( length > 1 ) ? length : 1 );
		glGetShaderInfoLog( id_frag, length, NULL, &error_frag[ 0 ] );
		std::cout << &error_frag[ 0 ] << std::endl;
	}
}

void TextureMgr::load_shader( std::string const & path_vert, std::string const & path_frag, GLuint & id_prog ) {
	GLuint id_vert, id_frag;
	GLint result = GL_FALSE;
	int length;

	load_vert_shader( path_vert, id_vert );
	load_frag_shader( path_frag, id_frag );

	std::cout << "Linking program" << std::endl;
	id_prog = glCreateProgram( );
	glAttachShader( id_prog, id_vert );
	glAttachShader( id_prog, id_frag );
	glLinkProgram( id_prog );

	glGetProgramiv( id_prog, GL_LINK_STATUS, &result );
	glGetProgramiv( id_prog, GL_INFO_LOG_LENGTH, &length );

	if( length ) {
		std::vector< char > error_prog( ( length > 1 ) ? length : 1 );
		glGetProgramInfoLog( id_prog, length, NULL, &error_prog[ 0 ] );
		std::cout << &error_prog[ 0 ] << std::endl;
	}

	//glDeleteShader( id_vert );
	//glDeleteShader( id_frag );
}

int TextureMgr::get_num_blocks( ) {
	return uvs_terrain.size( );
}

int TextureMgr::get_num_block_textures( int const id_block ) {
	return uvs_terrain[ id_block ].size( );
}

void TextureMgr::bind_skybox( ) {
	if( id_bound != id_skybox ) { 
		id_bound = id_skybox;
		glBindTexture( GL_TEXTURE_2D, id_bound );
	}
}

void TextureMgr::bind_terrain( ) {
	if( id_bound != id_terrain ) {
		id_bound = id_terrain;
		glBindTexture( GL_TEXTURE_2D, id_bound );
	}
}

void TextureMgr::bind_fonts( ) { 
	if( id_bound != id_fonts ) { 
		id_bound = id_fonts;
		glBindTexture( GL_TEXTURE_2D, id_bound );
	}
}

void TextureMgr::bind_materials( ) {
	if( id_bound != id_materials ) {
		id_bound = id_materials;
		glBindTexture( GL_TEXTURE_2D, id_bound );
	}
}

face_uvs & TextureMgr::get_uvs_block( int const id_block, int const id_texture ) {
	return uvs_terrain[ id_block ][ id_texture ];
}

face_uvs & TextureMgr::get_uvs_skybox( FaceDirection dir_face ) {
	return uvs_skybox[ dir_face ];
}

face_uvs & TextureMgr::get_uvs_sun( ) { 
	return uvs_sun;
}

face_uvs & TextureMgr::get_uvs_fonts( int const id_font ) { 
	return uvs_fonts[ id_font ];
}

face_uvs & TextureMgr::get_uvs_materials( ) { 
	return uvs_materials;
}