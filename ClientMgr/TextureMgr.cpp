#include "TextureMgr.h"

#include "Client.h"

#include "glm/gtc/type_ptr.hpp"
#include "tinyxml2-master/tinyxml2.h"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

int const TextureMgr::size_terrain_mip_map = 4;
int const TextureMgr::size_terrain_texture_padding = pow( 2, 4 );
int const TextureMgr::size_terrain_texture = 64;
int const TextureMgr::size_terrain_atlas = ( TextureMgr::size_terrain_texture + 2 * TextureMgr::size_terrain_texture_padding ) * 16 ;

TextureMgr::TextureMgr( Client & client ) :
	path_shaders( "./Shaders/" ),
	Manager( client ) { }

TextureMgr::~TextureMgr( ) { }

void TextureMgr::init( ) {
	printTabbedLine( 0, "Init TextureMgr..." );

	printTabbedLine( 1, "Creating shaders and loading data..." );

	loader_add( "Basic" );
	loader_add( "SMBasic" );
	loader_add( "Terrain" );
	loader_add( "SMTerrain" );
	loader_add( "Selector" );
	loader_add( "Entity" );
	loader_add( "ShadowMap" );
	loader_add( "SMShadowMapSolid" );
	loader_add( "SMShadowMapTrans" );

	std::cout << std::endl;
	printTabbedLine( 1, "Creating Textures..." );

	load_textures( );

	load_skybox( );
	load_sun( );
	load_fonts( );
	load_materials( );

	std::cout << std::endl;
	printTabbedLine( 1, "Creating uniform buffer objects..." );

	glGenBuffers( 1, &id_ubo_mvp );
	glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_mvp );
	glBufferData( GL_UNIFORM_BUFFER, sizeof( MVPMatrices ), &client.display_mgr.camera.mvp_matrices, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, 0, id_ubo_mvp );

	glGenBuffers( 1, &id_ubo_lights );
	glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_lights );
	glBufferData( GL_UNIFORM_BUFFER, sizeof( LightData ), &client.chunk_mgr.get_light_data( ), GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, 1, id_ubo_lights );

	std::cout << std::endl;
	printTabbedLine( 1, "Linking Uniform Buffer objects..." );
	for( auto & shader : { "Basic", "SMBasic", "Terrain", "SMTerrain", "Entity", "Selector" } ) {
		bind_program( shader );
		GLuint uniform_block_index = glGetUniformBlockIndex( id_prog, "mvp_matrices" );
		glUniformBlockBinding( id_prog, uniform_block_index, 0 );
	}

	for( auto & shader : { "Terrain", "SMTerrain", "Entity", "Selector" } ) {
		bind_program( shader );
		GLuint uniform_block_index = glGetUniformBlockIndex( id_prog, "light_data" );
		glUniformBlockBinding( id_prog, uniform_block_index, 1 );
	}

	glActiveTexture( GL_TEXTURE0 );

	for( auto & shader : { "Basic", "SMBasic", "Terrain", "SMTerrain", "Selector", "Entity" } ) { 
		bind_program( shader );
		GLuint prog_sampler = glGetUniformLocation( id_prog, "frag_sampler" );
		glUniform1i( prog_sampler, 0 );
	}
	
	std::cout << std::endl;
	printTabbedLine( 1, checkGlErrors( ) );
	std::cout << std::endl;
}

void TextureMgr::loader_add( std::string const & name ) { 
	ShaderLoader loader;
	loader.name = name;

	printTabbedLine( 1, "Loading shader: " + name );
	load_shader( path_shaders + name + ".vert", path_shaders + name + ".frag", loader.id_prog );

	list_shaders.emplace_back( loader );
	map_shaders.insert( { name, list_shaders.size( ) - 1 } );
}

void TextureMgr::bind_program( std::string const & name ) { 
	auto iter_map = map_shaders.find( name );
	if( iter_map == map_shaders.end( ) ) { 
		std::cout << "Error binding program " << name << std::endl;
		return;
	}

	auto & loader = list_shaders[ iter_map->second ];
	if( loader.id_prog == id_prog ) { 
		return;
	}

	id_prog = loader.id_prog;
	glUseProgram( id_prog );
}

void TextureMgr::bind_program( GLuint id_prog ) { 
	if( id_prog == this->id_prog ) {
		return;
	}

	this->id_prog = id_prog;
	glUseProgram( id_prog );
}

void TextureMgr::unbind_program( ) {
	id_prog = 0;
	glUseProgram( 0 );
}

MultiTex * TextureMgr::get_texture( std::string const & name_tex ) {
	auto iter = map_multitex.find( name_tex );
	if( iter == map_multitex.end( ) ) {
		std::cout << "Cant find multitex: " << name_tex << "!" << std::endl;
		return nullptr;
	}
	return iter->second;
	return nullptr;
}

GLuint TextureMgr::get_texture_id( std::string const & name_tex ) {
	auto iter = map_multitex.find( name_tex );
	if( iter == map_multitex.end( ) ) {
		std::cout << "Cant find multitex: " << name_tex << "!" << std::endl;
		return 0;
	}
	return iter->second->id_tex;
}

GLuint TextureMgr::get_texture_layer( std::string const & name_tex, std::string const & name_subtex ) {
	auto iter_multitex = map_multitex.find( name_tex );
	if( iter_multitex == map_multitex.end( ) ) {
		std::cout << "Cant find multitex: " << name_tex << " " << name_subtex << "!" << std::endl;
		return 0;
	}

	auto iter_subtex = iter_multitex->second->map_tex_lookup.find( name_subtex );
	if( iter_subtex == iter_multitex->second->map_tex_lookup.end( ) ) {
		std::cout << "Cant find subtex: " << name_tex << " " << name_subtex << "!" << std::endl;
		return 0;
	}
	return iter_subtex->second;
}

void TextureMgr::bind_texture( GLuint const id_active, GLuint const id_texture ) {
	if( this->id_active != id_active ) {
		this->id_active = id_active;
		glActiveTexture( GL_TEXTURE0 + id_active );
	}

	if( this->id_texture != id_texture ) {
		this->id_texture = id_texture;
		glBindTexture( GL_TEXTURE_2D, id_texture );
	}
}

void TextureMgr::bind_texture_array( GLuint const id_active, GLuint const id_texture ) {
	if( this->id_active != id_active ) {
		this->id_active = id_active;
		glActiveTexture( GL_TEXTURE0 + id_active );
	}

	if( this->id_texture != id_texture ) {
		this->id_texture = id_texture;
		glBindTexture( GL_TEXTURE_2D_ARRAY, id_texture );
	}
}

void TextureMgr::load_textures( ) { 
	using namespace std::tr2::sys;
	path path_base( "./Textures" );
	path path_multitex;
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement const * elem;
	GLuint num_tex = 0;

	// Load xml file
	doc.LoadFile( "./Textures/MultiTexDesc.xml" );

	std::cout << std::endl;
	std::cout << checkGlErrors( ) << std::endl;
	std::cout << "Loading MultiTexs..." << std::endl;

	// Any errors?
	if( doc.Error( ) ) { 
		std::cout << "ERROR: Error while loading MultiTex descriptor!!" << std::endl;
		doc.PrintError( );
		return;
	}

	// Lets find out how many multitex elements we have in our xml file!
	elem = doc.FirstChildElement( "MultiTex" );
	while( elem ) {
		MultiTex entry;
		entry.name = elem->GetText( );
		entry.dim.x = elem->FindAttribute( "x" )->IntValue( );
		entry.dim.y = elem->FindAttribute( "y" )->IntValue( );
		entry.num_mipmap = elem->FindAttribute( "mipmap" )->IntValue( );
		if( entry.num_mipmap <= 0 ) entry.num_mipmap = 1;
		if( entry.num_mipmap > 10 ) entry.num_mipmap = 10;

		list_multitex.emplace_back( entry );

		elem = elem->NextSiblingElement( "MultiTex" );
	}

	// Reserve bucket space in the lookup map
	map_multitex.reserve( list_multitex.size( ) );

	// Lets load the subtextures for each folder into the multitex!
	for( int i = 0; i < list_multitex.size( ); ++i ) {
		auto & entry = list_multitex[ i ];

		// Grab our new path!
		path_multitex = path_base;
		path_multitex.append( "/" + entry.name );

		// Count how many sub textures there are!
		num_tex = 0;
		for( directory_iterator iter_multitex( path_multitex ); iter_multitex != directory_iterator( ); ++iter_multitex ) {
			if( !is_directory( iter_multitex->status( ) ) ) {
				continue;
			}

			for( directory_iterator iter_multitex_sub( iter_multitex->path( ) ); iter_multitex_sub != directory_iterator( ); ++iter_multitex_sub ) {
				if( iter_multitex_sub->path( ).extension( ).string( ) == ".png" ) {
					++num_tex;
				}
			}
		}

		// Reserve bucket space fot the sub texture lookup
		entry.map_tex_lookup.reserve( num_tex );

		std::cout << "Loading MultiTex: [" << entry.name << "] dim:" << Directional::print_vec( entry.dim ) 
			<< " size:" << entry.size << " mipmap:" << entry.num_mipmap << std::endl;

		// Request space fot he copy and final multitexture
		glGenTextures( 1, &id_copy );
		glBindTexture( GL_TEXTURE_2D, id_copy );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, entry.dim.x, entry.dim.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

		glGenTextures( 1, &entry.id_tex );
		glBindTexture( GL_TEXTURE_2D_ARRAY, entry.id_tex );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexStorage3D( GL_TEXTURE_2D_ARRAY, entry.num_mipmap, GL_RGBA8, entry.dim.x, entry.dim.y, 1024 );

		std::cout << "Error after allocation? " << checkGlErrors( ) << std::endl;

		// Lets iterate over all the sub textures in their folders!
		for( directory_iterator iter_multitex( path_multitex ); iter_multitex != directory_iterator( ); ++iter_multitex ) {
			if( !is_directory( iter_multitex->status( ) ) ) {
				continue;
			}

			for( directory_iterator iter_multitex_sub( iter_multitex->path( ) ); iter_multitex_sub != directory_iterator( ); ++iter_multitex_sub ) {
				if( iter_multitex_sub->path( ).extension( ).string( ) != ".png" ) {
					continue;
				}

				// Load the sub texture into copy
				id_copy = SOIL_load_OGL_texture(
					iter_multitex_sub->path( ).string( ).c_str( ),
					SOIL_LOAD_RGBA,
					id_copy,
					SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_INVERT_Y );

				// Copy it from there to our multitexture
				glCopyImageSubData(
					id_copy, GL_TEXTURE_2D, 0,
					0, 0, 0,
					entry.id_tex, GL_TEXTURE_2D_ARRAY, 0,
					0, 0, entry.size,
					entry.dim.x, entry.dim.y, 1 );

				// Lets find the path relative to the multitexture name!
				std::string path_relative;

				auto iter_path = iter_multitex_sub->path( ).end( );
				iter_path--;
				iter_path--;

				path_relative += iter_path->string( ) + "/";
				iter_path++;
				path_relative += iter_path->filename( ).stem( ).string( );

				entry.map_tex_lookup.insert( { path_relative, entry.size } );

				entry.size++;

				std::cout << "Loading subtex: " << path_relative << std::endl;
			}
		}

		map_multitex.insert( { entry.name, &entry } );

		// Generate mipmaps!
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		glGenerateMipmap( GL_TEXTURE_2D_ARRAY );

		glDeleteTextures( 1, &id_copy );

		std::cout << "Loaded MultiTex[" << entry.name << "] dim:" << Directional::print_vec( entry.dim ) << " size:" << entry.size << std::endl;
	}

	std::cout << checkGlErrors( ) << std::endl;
	std::cout << std::endl;
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

void TextureMgr::update( ) {
	std::cout << "Tex in: " << checkGlErrors( ) << std::endl;
	glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_mvp );
	auto & matrices = client.display_mgr.camera.mvp_matrices;
	matrices.time_game = client.time_mgr.get_time( TimeStrings::GAME );
	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( MVPMatrices ), &matrices );

	glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_lights );
	auto & light_data = client.chunk_mgr.get_light_data( );

	glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( LightData ), &light_data );
	std::cout << "Tex out: " << checkGlErrors( ) << std::endl;
	/*
	int index = 0;
	glBufferSubData( GL_UNIFORM_BUFFER, index, sizeof( LightData::SunData ), &light_data.sun_data );
	index += sizeof( LightData::SunData );
	glBufferSubData( GL_UNIFORM_BUFFER, index, sizeof( GLint ) + light_data.num_emitter * sizeof( glm::vec4 ), &light_data.num_emitter );
	index += sizeof( GLint ) + light_data.max_emitters * sizeof( glm::vec4 );
	glBufferSubData( GL_UNIFORM_BUFFER, index, light_data.num_emitter * sizeof( glm::vec4 ), &light_data.list_color );
	index += light_data.max_emitters * sizeof( glm::vec4 );
	glBufferSubData( GL_UNIFORM_BUFFER, index, light_data.num_emitter * sizeof( GLfloat ), &light_data.list_radius );
	*/
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

	glDeleteShader( id_vert );
	glDeleteShader( id_frag );
}
 
void TextureMgr::bind_skybox( ) {
	if( id_texture != id_skybox ) { 
		id_texture = id_skybox;
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, id_texture );
	}
}

void TextureMgr::bind_fonts( ) { 
	if( id_texture != id_fonts ) { 
		id_texture = id_fonts;
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, id_texture );
	}
}

void TextureMgr::bind_materials( ) {
	if( id_texture != id_materials ) {
		id_texture = id_materials;
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, id_texture );
	}
}

ShaderLoader const * TextureMgr::get_program( std::string const & name ) {
	auto & iter_program = map_shaders.find( name );
	if( iter_program != map_shaders.end( ) ) { 
		return &list_shaders[ iter_program->second ];
	}

	return nullptr;
}

GLuint const TextureMgr::get_program_id( std::string const & name ) {
	auto & iter_program = map_shaders.find( name );
	if( iter_program != map_shaders.end( ) ) {
		return list_shaders[ iter_program->second ].id_prog;
	}
	else { 
		return 0;
	}
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