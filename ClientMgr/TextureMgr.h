#pragma once
#include "Globals.h"

#include "Manager.h"

#include <vector>
#include "Block.h"
#include <array>
#include <unordered_map>

typedef std::array< std::array< float, 2 >, 4 > face_uvs;

struct ShaderLoader { 
	std::string name;
	GLuint id_prog = 0;
};

class TextureMgr :
	public Manager {

private:
	static int const size_terrain_mip_map;
	static int const size_terrain_texture_padding;
	static int const size_terrain_texture;
	static int const size_terrain_atlas;

	std::string const path_shaders;
	std::vector< ShaderLoader > list_shaders;
	std::unordered_map< std::string, int > map_shaders;

	GLuint id_copy;

	GLuint id_ubo_mvp;
	GLuint id_ubo_lights;

	std::vector< std::vector< FaceUvs > > uvs_terrain;
	std::vector< face_uvs > uvs_skybox;
	face_uvs uvs_sun;
	std::vector< face_uvs > uvs_fonts;
	face_uvs uvs_materials;

public:	
	GLuint id_prog = 0;
	GLuint id_active = 0;
	GLuint id_texture = 0;

	GLuint id_terrain;
	GLuint id_skybox;
	GLuint id_fonts;
	GLuint id_materials;

private:
	void load_terrain( );
	void load_skybox( );
	void load_sun( );
	void load_fonts( );
	void load_materials( );

	void read_file( std::string const & path_file, std::string & data );
	void load_vert_shader( std::string const & path_file, GLuint & id_vert );
	void load_frag_shader( std::string const & path_file, GLuint & id_frag );
	void load_shader( std::string const & path_vert, 
		std::string const & path_frag, GLuint & id_prog );

public:
	TextureMgr( Client & client );
	~TextureMgr();

	void init( );
	void update( );
	void render( ) { }
	void end( ) { }
	void sec( ) { }

	void loader_add( std::string const & name );

	int get_num_blocks( );
	int get_num_block_textures( int const id_block );

	void bind_skybox( );
	void bind_terrain( );
	void bind_fonts( );
	void bind_materials( );

	void bind_program( std::string const & name );
	void unbind_program( );

	void bind_texture( GLuint const id_active, GLuint const id_texture );

	FaceUvs & get_uvs_block( int const id_block, int const id_texture );
	face_uvs & get_uvs_skybox( FaceDirection dir_face );
	face_uvs & get_uvs_sun( );
	face_uvs & get_uvs_fonts( int const id_font );
	face_uvs & get_uvs_materials( );

};