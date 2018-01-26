#pragma once
#include "Globals.h"

#include "Manager.h"

#include "glew\include\GL\glew.h"
#include "glm\glm.hpp"

#include <vector>
#include <array>
#include <unordered_map>

typedef std::array< std::array< float, 2 >, 4 > face_uvs;

struct ShaderLoader { 
	GLuint id_program = 0;
	std::string name;
};

struct MultiTexture { 
	GLuint id_texture;
	GLuint num_mipmap;
	GLuint num_subtexture;
	glm::ivec2 dim_texture;
	std::string name_texture;
	std::unordered_map< std::string, int > map_subtexture_lookup;

	MultiTexture( ) :
		id_texture( 0 ),
		num_mipmap( 0 ),
		num_subtexture( 0 ),
		dim_texture( 0, 0 ),
		name_texture( "" ),
		map_subtexture_lookup( ) { }
};

class TextureMgr :
	public Manager {

private:
	std::string const path_shaders;
	std::vector< ShaderLoader > list_shaders;
	std::unordered_map< std::string, GLuint > map_shaders;

	GLuint id_copy;

	GLuint id_ubo_mvp;
	GLuint id_ubo_lights;

	std::vector< face_uvs > uvs_fonts;

	std::vector< MultiTexture > list_multitex;
	std::unordered_map< std::string, MultiTexture * > map_multitex;

public:	
	GLuint id_bound_program = 0;
	GLuint id_bound_active = 0;
	GLuint id_bound_texture = 0;

private:
	void load_textures( );
	void load_fonts( );

	void read_file( std::string const & path_file, std::string & data );
	void load_geom_shader( std::string const & path_file, GLuint & id_geom );
	void load_vert_shader( std::string const & path_file, GLuint & id_vert );
	void load_frag_shader( std::string const & path_file, GLuint & id_frag );
	void load_shader( std::string const & path_vert, std::string const & path_frag, GLuint & id_program );
	void load_shader( std::string const & path_geom, std::string const & path_vert, std::string const & path_frag, GLuint & id_program );

public:
	TextureMgr( Client & client );
	~TextureMgr();

	void init( );
	void update( );
	void render( ) { }
	void end( ) { }
	void sec( ) { }

	void loader_add( std::string const & name, bool is_geom );

	ShaderLoader const * get_program( std::string const & name );
	GLuint const get_program_id( std::string const & name );

	void bind_program( std::string const & name );
	void bind_program( GLuint const id_prog );
	void unbind_program( );

	MultiTexture * get_texture( std::string const & name_texture );
	GLuint get_texture_id( std::string const & name_texture );
	GLuint get_texture_layer( std::string const & name_texture, std::string const & name_subtexture );

	void bind_texture( GLuint const id_active, GLuint const id_texture );
	void bind_texture_array( GLuint const id_active, GLuint const id_texture );

	face_uvs & get_uvs_fonts( int const id_font );

	void update_uniform( GLuint id_program, std::string const & name_uniform, GLint const value );
	template< int unsigned size >
	void update_uniform( GLuint id_program, std::string const & name_uniform, GLint const ( & values )[ size ] ) { 
		bind_program( id_program );
		glUniform1iv( glGetUniformLocation( id_program, name_uniform.c_str( ) ), size, values );
	}

	void update_uniform( GLuint id_program, std::string const & name_uniform, GLuint const value );

	void update_uniform( GLuint id_program, std::string const & name_uniform, GLfloat const value );
	template< int unsigned size >
	void update_uniform( GLuint id_program, std::string const & name_uniform, GLfloat const ( &values )[ size ] ) {
		bind_program( id_program );
		glUniform1fv( glGetUniformLocation( id_program, name_uniform.c_str( ) ), size, values );
	}

	void update_uniform( GLuint id_program, std::string const & name_uniform, glm::mat4 const & value );
	template< int unsigned size >
	void update_uniform( GLuint id_program, std::string const & name_uniform, glm::mat4 const ( & values )[ size ] ) { 
		bind_program( id_program );
		glUniformMatrix4fv( glGetUniformLocation( id_program, name_uniform.c_str( ) ), size, GL_FALSE, glm::value_ptr( values[ 0 ] ) );
	}

	template< class T >
	void update_uniform( std::string const & name_program, std::string const & name_uniform, T const & value ) { 
		GLuint id_program;
		id_program = get_program_id( name_program.c_str( ) );
		update_uniform( id_program, name_uniform, value );
	}
};