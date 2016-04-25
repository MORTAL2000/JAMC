#pragma once
#include "Globals.h"

#include "Manager.h"

#include <vector>
#include "Block.h"
#include <array>

typedef std::array< std::array< float, 2 >, 4 > face_uvs;

class TextureMgr :
	public Manager {

private:
	static int const size_terrain_mip_map;
	static int const size_terrain_texture_padding;
	static int const size_terrain_texture;
	static int const size_terrain_atlas;

	GLuint id_terrain;
	GLuint id_temp;
	GLuint id_skybox;
	GLuint id_fonts;
	GLuint id_materials;
	GLuint id_bound = -1;

	std::vector< std::vector< face_uvs > > uvs_terrain;
	std::vector< face_uvs > uvs_skybox;
	face_uvs uvs_sun;
	std::vector< face_uvs > uvs_fonts;
	face_uvs uvs_materials;

public:
	GLuint id_prog;

private:
	void load_terrain( );
	void load_skybox( );
	void load_sun( );
	void load_fonts( );
	void load_materials( );

public:
	TextureMgr( Client & client );
	~TextureMgr();

	void init( );
	void update( );
	void render( ) { }
	void end( ) { }
	void sec( ) { }

	void read_file( std::string const & path_file, std::string & data );
	void load_vert_shader( std::string const & path_file, GLuint & id_vert );
	void load_frag_shader( std::string const & path_file, GLuint & id_frag );
	void load_shader( std::string const & path_vert, std::string const & path_frag, GLuint & id_prog );

	int get_num_blocks( );
	int get_num_block_textures( int const id_block );

	void bind_skybox( );
	void bind_terrain( );
	void bind_fonts( );
	void bind_materials( );

	void use_prog( ) { glUseProgram( id_prog ); }
	void unuse_prog( ) { glUseProgram( 0 ); }

	face_uvs & get_uvs_block( int const id_block, int const id_texture );
	face_uvs & get_uvs_skybox( FaceDirection dir_face );
	face_uvs & get_uvs_sun( );
	face_uvs & get_uvs_fonts( int const id_font );
	face_uvs & get_uvs_materials( );

};