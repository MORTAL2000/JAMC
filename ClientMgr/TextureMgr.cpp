#include "TextureMgr.h"

#include "Client.h"

#include "glm/gtc/type_ptr.hpp"
#include "tinyxml2-master/tinyxml2.h"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

TextureMgr::TextureMgr( Client & client ) :
	path_shaders( "./Shaders/" ),
	Manager( client ) { }

TextureMgr::~TextureMgr( ) { }

void TextureMgr::init( ) {
	printf( "\n*** TextureMgr ***\n" );

	printf( "\nCreating shaders...\n\n" );

	for( auto & shader : {
		"BasicOrtho", "BasicPersp", "SMBasic", "SMBasicProj", 
		"Terrain", "SMTerrainInstance", "Selector", "Entity", 
		"ShadowMap" } ) {

		loader_add( shader, false );
	}

	for( auto & shader : {
		"SMTerrain", "SMShadowMapSolid", "SMShadowMapTrans", "SMTerrainBasic" } ) {

		loader_add( shader, true );
	}

	printf( "\nLoading Textures...\n" );

	GL_CHECK( load_textures( ) );

	printf( "\nLoading Fonts...\n" );

	GL_CHECK( load_fonts( ) );

	printf( "\nCreating uniform buffer objects...\n" );

	GL_CHECK( glGenBuffers( 1, &id_ubo_mvp ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_mvp ) );
	GL_CHECK( glBufferData( GL_UNIFORM_BUFFER, sizeof( MVPMatrices ), &client.display_mgr.camera.mvp_matrices, GL_DYNAMIC_DRAW ) );
	GL_CHECK( glBindBufferBase( GL_UNIFORM_BUFFER, 0, id_ubo_mvp ) );

	GL_CHECK( glGenBuffers( 1, &id_ubo_lights ) );
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_lights ) );
	GL_CHECK( glBufferData( GL_UNIFORM_BUFFER, sizeof( LightData ), &client.chunk_mgr.get_light_data( ), GL_DYNAMIC_DRAW ) );
	GL_CHECK( glBindBufferBase( GL_UNIFORM_BUFFER, 1, id_ubo_lights ) );

	printf( "\nLinking Uniforms...\n" );
	GLuint id_program;
	GLuint idx_block;
	GLuint idx_sampler;

	// Link MVP Matrix UBO
	for( auto const & shader : { 
		"BasicOrtho", "BasicPersp", "SMBasic", "SMBasicProj", "Terrain",
		"SMTerrain", "SMTerrainBasic", "SMTerrainInstance", "Entity", "Selector" } ) {

		id_program = get_program_id( shader );
		GL_CHECK( idx_block = glGetUniformBlockIndex( id_program, "mvp_matrices" ) );
		GL_CHECK( glUniformBlockBinding( id_program, idx_block, 0 ) );
	}

	// Link Light Data UBO
	for( auto const & shader : { 
		"BasicPersp", "Terrain", "SMTerrain", "SMTerrainBasic",
		"SMTerrainInstance", "Entity", "Selector" } ) {

		id_program = get_program_id( shader );
		GL_CHECK( idx_block = glGetUniformBlockIndex( id_program, "light_data" ) );
		GL_CHECK( glUniformBlockBinding( id_program, idx_block, 1 ) );
	}

	// Uniform frag_sampler
	for( auto const & shader : { 
		"BasicOrtho", "BasicPersp", "SMBasic", "SMBasicProj", "Terrain",
		"SMTerrain", "SMTerrainBasic", "SMTerrainInstance", "Selector", "Entity" } ) {

		id_program = get_program_id( shader );
		bind_program( id_program );
		GL_CHECK( idx_sampler = glGetUniformLocation( id_program, "frag_sampler" ) );
		GL_CHECK( glUniform1i( idx_sampler, 0 ) );
	}
}

void TextureMgr::loader_add( std::string const & name, bool is_geom ) { 
	ShaderLoader loader;
	loader.name = name;

	printf( "Loading shader: %s\n", name.c_str( ) );
	if( is_geom ) {
		load_shader( path_shaders + name + ".geom", path_shaders + name + ".vert", path_shaders + name + ".frag", loader.id_program );
	}
	else { 
		load_shader( path_shaders + name + ".vert", path_shaders + name + ".frag", loader.id_program );
	}

	list_shaders.emplace_back( loader );
	map_shaders.insert( { name, ( GLuint ) list_shaders.size( ) - 1 } );
}

void TextureMgr::bind_program( std::string const & name ) { 
	auto iter_map = map_shaders.find( name );
	if( iter_map == map_shaders.end( ) ) { 
		printf( "Error binding program %s\n", name.c_str( ) );
		return;
	}

	auto & loader = list_shaders[ iter_map->second ];
	if( loader.id_program == id_bound_program ) { 
		return;
	}

	id_bound_program = loader.id_program;
	glUseProgram( id_bound_program );
}

void TextureMgr::bind_program( GLuint id_prog ) { 
	if( id_prog == this->id_bound_program ) {
		return;
	}

	this->id_bound_program = id_prog;
	glUseProgram( id_prog );
}

void TextureMgr::unbind_program( ) {
	id_bound_program = 0;
	glUseProgram( 0 );
}

MultiTexture * TextureMgr::get_texture( std::string const & name_texture ) {
	auto iter = map_multitex.find( name_texture );
	if( iter == map_multitex.end( ) ) {
		printf( "Cant find multitex: %s!\n", name_texture.c_str( ) );
		return nullptr;
	}
	return iter->second;
}

GLuint TextureMgr::get_texture_id( std::string const & name_texture ) {
	auto iter = map_multitex.find( name_texture );
	if( iter == map_multitex.end( ) ) {
		printf( "Cant find multitex: %s!\n", name_texture.c_str( ) );
		return 0;
	}
	return iter->second->id_texture;
}

GLuint TextureMgr::get_texture_layer( std::string const & name_texture, std::string const & name_subtexture ) {
	auto iter_multitex = map_multitex.find( name_texture );
	if( iter_multitex == map_multitex.end( ) ) {
		printf( "Cant find multitex: %s %s!", name_texture.c_str( ), name_subtexture.c_str( ) );
		return 0;
	}

	auto iter_subtex = iter_multitex->second->map_subtexture_lookup.find( name_subtexture );
	if( iter_subtex == iter_multitex->second->map_subtexture_lookup.end( ) ) {
		std::cout << "Cant find subtex: " << name_texture << " " << name_subtexture << "!" << std::endl;
		return 0;
	}
	return iter_subtex->second;
}

void TextureMgr::bind_texture( GLuint const id_active, GLuint const id_texture ) {
	if( this->id_bound_active != id_active ) {
		this->id_bound_active = id_active;
		glActiveTexture( GL_TEXTURE0 + id_active );
	}

	if( this->id_bound_texture != id_texture ) {
		this->id_bound_texture = id_texture;
		glBindTexture( GL_TEXTURE_2D, id_texture );
	}
}

void TextureMgr::bind_texture_array( GLuint const id_active, GLuint const id_texture ) {
	if( this->id_bound_active != id_active ) {
		this->id_bound_active = id_active;
		glActiveTexture( GL_TEXTURE0 + id_active );
	}

	if( this->id_bound_texture != id_texture ) {
		this->id_bound_texture = id_texture;
		glBindTexture( GL_TEXTURE_2D_ARRAY, id_texture );
	}
}

void TextureMgr::load_textures( ) { 
	using namespace std::experimental::filesystem;
	path path_base( "./Textures" );
	path path_multitex;
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement const * elem;
	GLuint num_tex = 0;

	// Load xml file
	doc.LoadFile( "./Textures/MultiTexDesc.xml" );

	// Any errors?
	if( doc.Error( ) ) { 
		std::cout << "ERROR: Error while loading MultiTex descriptor!!" << std::endl;
		doc.PrintError( );
		return;
	}

	// Lets find out how many multitex elements we have in our xml file!
	elem = doc.FirstChildElement( "MultiTex" );
	while( elem ) {
		MultiTexture entry;
		entry.name_texture = elem->GetText( );
		entry.dim_texture.x = elem->FindAttribute( "x" )->IntValue( );
		entry.dim_texture.y = elem->FindAttribute( "y" )->IntValue( );
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
		path_multitex.append( "/" + entry.name_texture );

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
		entry.map_subtexture_lookup.reserve( num_tex );

		printf( "\nLoading MultiTex[ %s ] dim: %s size: %i\n", entry.name_texture.c_str( ), Directional::print_vec( entry.dim_texture ).c_str( ), entry.num_subtexture );

		// Request space fo the copy and final multitexture
		glGenTextures( 1, &id_copy );
		glBindTexture( GL_TEXTURE_2D, id_copy );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, entry.dim_texture.x, entry.dim_texture.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

		glGenTextures( 1, &entry.id_texture );
		glBindTexture( GL_TEXTURE_2D_ARRAY, entry.id_texture );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexStorage3D( GL_TEXTURE_2D_ARRAY, entry.num_mipmap, GL_RGBA8, entry.dim_texture.x, entry.dim_texture.y, num_tex );

		// Lets iterate down all the sub textures in their folders!
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
					SOIL_FLAG_INVERT_Y );

				// Copy it from there to our multitexture
				glCopyImageSubData(
					id_copy, GL_TEXTURE_2D, 0,
					0, 0, 0,
					entry.id_texture, GL_TEXTURE_2D_ARRAY, 0,
					0, 0, entry.num_subtexture,
					entry.dim_texture.x, entry.dim_texture.y, 1 );

				// Lets find the path relative to the multitexture name!
				std::string path_relative;

				auto iter_path = iter_multitex_sub->path( ).end( );
				iter_path--;
				iter_path--;

				path_relative += iter_path->string( ) + "/";
				iter_path++;
				path_relative += iter_path->filename( ).stem( ).string( );

				entry.map_subtexture_lookup.insert( { path_relative, entry.num_subtexture } );

				entry.num_subtexture++;
				printf( "Loading subtex: %s\n", path_relative.c_str( ) );
			}
		}

		map_multitex.insert( { entry.name_texture, &entry } );

		// Generate mipmaps!
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		glGenerateMipmap( GL_TEXTURE_2D_ARRAY );

		glDeleteTextures( 1, &id_copy );
		printf( "Loaded MultiTex[ %s ] dim: %s size: %i\n", entry.name_texture.c_str( ), Directional::print_vec( entry.dim_texture ).c_str( ), entry.num_subtexture );
	}
}

void TextureMgr::load_fonts( ) { 
	int x, y;
	float offset_x = 0;
	float offset_y = -19.5f;
	int padding_x = 27;
	int padding_y = 46;
	float pixel_correct = 0.5f;

	glm::ivec2 dim_fonts( 860, 1400 );
	glm::ivec2 dim_char = dim_fonts / 10;

	for( int i = 0; i < ( 176 - 32 ); i++ ) { 
		x = ( i * dim_char.x ) % dim_fonts.x;
		y = dim_fonts.y - dim_char.y - ( ( i * dim_char.x ) / dim_fonts.x ) * ( dim_char.y );

		uvs_fonts.push_back( face_uvs { {
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

void TextureMgr::update( ) {
	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_mvp ) );
	auto & matrices = client.display_mgr.camera.mvp_matrices;
	matrices.time_game = client.time_mgr.get_time( TimeStrings::GAME );
	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( MVPMatrices ), &matrices ) );

	GL_CHECK( glBindBuffer( GL_UNIFORM_BUFFER, id_ubo_lights ) );
	auto & light_data = client.chunk_mgr.get_light_data( );

	GL_CHECK( glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( LightData ), &light_data ) );
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

void TextureMgr::load_geom_shader( std::string const & path_file, GLuint & id_geom ) { 
	std::string data_geom;
	read_file( path_file, data_geom );

	char const * ptr_data_frag = data_geom.c_str( );
	GLint result = GL_FALSE;
	int length;

	// Compile geometry shader
	id_geom = glCreateShader( GL_GEOMETRY_SHADER );
	glShaderSource( id_geom, 1, &ptr_data_frag, NULL );
	glCompileShader( id_geom );

	// Check geometry shader
	glGetShaderiv( id_geom, GL_COMPILE_STATUS, &result );
	glGetShaderiv( id_geom, GL_INFO_LOG_LENGTH, &length );

	if( length ) {
		std::vector< char > error_frag( ( length > 1 ) ? length : 1 );
		glGetShaderInfoLog( id_geom, length, NULL, &error_frag[ 0 ] );
		std::cout << &error_frag[ 0 ] << std::endl;
	}
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

void TextureMgr::load_shader( std::string const & path_vert, std::string const & path_frag, GLuint & id_program ) {
	GLuint id_vert, id_frag;
	GLint result = GL_FALSE;
	int length;

	load_vert_shader( path_vert, id_vert );
	load_frag_shader( path_frag, id_frag );

	id_program = glCreateProgram( );
	glAttachShader( id_program, id_vert );
	glAttachShader( id_program, id_frag );
	glLinkProgram( id_program );

	glGetProgramiv( id_program, GL_LINK_STATUS, &result );
	glGetProgramiv( id_program, GL_INFO_LOG_LENGTH, &length );

	if( length ) {
		std::vector< char > error_prog( ( length > 1 ) ? length : 1 );
		glGetProgramInfoLog( id_program, length, NULL, &error_prog[ 0 ] );
		printf( "Error loading shader: %s", error_prog.data( ) );
	}

	glDeleteShader( id_vert );
	glDeleteShader( id_frag );
}

void TextureMgr::load_shader( std::string const & path_geom, std::string const & path_vert, std::string const & path_frag, GLuint & id_program ) { 
	GLuint id_geom, id_vert, id_frag;
	GLint result = GL_FALSE;
	int length;

	load_geom_shader( path_geom, id_geom );
	load_vert_shader( path_vert, id_vert );
	load_frag_shader( path_frag, id_frag );

	id_program = glCreateProgram( );
	glAttachShader( id_program, id_geom );
	glAttachShader( id_program, id_vert );
	glAttachShader( id_program, id_frag );
	glLinkProgram( id_program );

	glGetProgramiv( id_program, GL_LINK_STATUS, &result );
	glGetProgramiv( id_program, GL_INFO_LOG_LENGTH, &length );

	if( length ) {
		std::vector< char > error_prog( ( length > 1 ) ? length : 1 );
		glGetProgramInfoLog( id_program, length, NULL, &error_prog[ 0 ] );
		printf( "Error loading shader: %s", error_prog.data( ) );
	}

	glDeleteShader( id_geom );
	glDeleteShader( id_vert );
	glDeleteShader( id_frag );
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
		return list_shaders[ iter_program->second ].id_program;
	}
	else { 
		return 0;
	}
}

face_uvs & TextureMgr::get_uvs_fonts( int const id_font ) { 
	return uvs_fonts[ id_font ];
}

void TextureMgr::update_uniform( GLuint id_program, std::string const & name_uniform, GLint const value ) { 
	bind_program( id_program );
	glUniform1i( glGetUniformLocation( id_program, name_uniform.c_str( ) ), value );
}

void TextureMgr::update_uniform( GLuint id_program, std::string const & name_uniform, GLuint const value ) { 
	bind_program( id_program );
	glUniform1ui( glGetUniformLocation( id_program, name_uniform.c_str( ) ), value );
}

void TextureMgr::update_uniform( GLuint id_program, std::string const & name_uniform, GLfloat const value ) { 
	bind_program( id_program );
	glUniform1f( glGetUniformLocation( id_program, name_uniform.c_str( ) ), value );
}

void TextureMgr::update_uniform( GLuint id_program, std::string const & name_uniform, glm::mat4 const & value ) { 
	bind_program( id_program );
	glUniformMatrix4fv( glGetUniformLocation( id_program, name_uniform.c_str( ) ), 1, GL_FALSE, glm::value_ptr( value ) );
}

/*
void TextureMgr::update_uniform( std::string const & name_program, std::string const & name_uniform, GLint value ) { 
	GLuint id_program;
	id_program = get_program_id( name_program.c_str( ) );
	update_uniform( id_program, name_uniform, value );
}

void TextureMgr::update_uniform( std::string const & name_program, std::string const & name_uniform, GLuint value ) { 
	GLuint id_program;
	id_program = get_program_id( name_program.c_str( ) );
	update_uniform( id_program, name_uniform, value );
}

void TextureMgr::update_uniform( std::string const & name_program, std::string const & name_uniform, GLfloat value ) { 
	GLuint id_program;
	id_program = get_program_id( name_program.c_str( ) );
	update_uniform( id_program, name_uniform, value );
}

void TextureMgr::update_uniform( std::string const & name_program, std::string const & name_uniform, glm::mat4 & value ) { 
	GLuint id_program;
	id_program = get_program_id( name_program.c_str( ) );
	update_uniform( id_program, name_uniform, value );
}*/
