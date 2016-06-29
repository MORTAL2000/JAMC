#pragma once

#include "Globals.h"

#include "Manager.h"
#include "Block.h"
#include "Chunk.h"
#include "ResPool.h" 
#include "VBO.h"

#include <fstream>
#include <unordered_map>

struct SetState {
	glm::ivec3 pos_lw; 
	glm::ivec3 pos_lw_last = pos_lw + glm::ivec3( 1, 1, 1 );
	glm::ivec3 pos_lc;
	std::unordered_map< int, Handle< Chunk > >::iterator iter;
	std::unordered_map< int, Chunk & > map_queue_dirty;
};

struct GetState { 
	glm::ivec3 pos_lw;
	glm::ivec3 pos_lw_last = pos_lw + glm::ivec3( 1, 1, 1 );
	glm::ivec3 pos_lc;
	std::unordered_map< int, Handle< Chunk > >::iterator iter;
};

struct LightData {
	static int const max_emitters = 128;

	struct SunData {
		glm::vec4 pos_sun;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
	} sun_data;

	glm::ivec4 num_emitters;
	glm::vec4 list_pos[ max_emitters ];
	glm::vec4 list_color[ max_emitters ];
	glm::vec4 list_radius[ max_emitters ];
};

struct World { 
	static int const size_x = 16;
	static int const size_y = 4;
	static int const size_z = 16;
	static glm::ivec3 const size_vect;
	static int const num_chunks = 
		( size_x * 2 + 1 ) * ( size_y * 2 + 1 ) * ( size_z * 2 + 1 ) + 
		( size_x * 2 + 1 ) * ( size_z * 2 + 1 );
	static int const level_sea = Chunk::size_y / 2;
};

struct Region { 
	static int const size_x = 16;
	static int const size_y = 4;
	static int const size_z = 16;
};

struct SectionIndex { 
	static int const size_section_bytes = 4096;
	static int const num_sections = 64;

	int index_section[ num_sections ];
};

struct FileHeader { 
	static int const num_regions = 
		Region::size_x * Region::size_y * Region::size_z;
	
	SectionIndex array_index[ num_regions ];
	int idx_last = 0;
};

struct ChunkFile { 
	int cnt_using = 0;
	FileHeader file_header;
	std::fstream file_stream;
};

struct ChunkNoise {
	int cnt_using = 0;
	int height[ Chunk::size_x ][ Chunk::size_z ];
	int biome[ Chunk::size_x ][ Chunk::size_z ];
};

static int const size_buffer = SectionIndex::size_section_bytes / sizeof( int );
static int buffer_section[ size_buffer ];

struct Emitter { 
	glm::vec4 pos;
	glm::vec4 color;
	float radius;
};

class ChunkMgr :
	public Manager {

private:
	VBO vbo;

	static int const dist_sun = 1000;
	bool is_sun_pause;
	float pos_deg_light;

	bool is_chunk_debug;
	bool is_shadow_debug;
	bool is_shadows;

	GLuint id_vbo_chunk_outline;
	int size_chunk_outline;

	// Block data containers
	std::vector< Block > list_block_data;
	std::unordered_map< std::string, int > map_block_data;

	// Center chunk pos
	glm::ivec3 pos_center_chunk_lw;

	// Map container mutex
	std::mutex mtx_noise;
	std::mutex mtx_file;

	// Chunk data container mutex
	std::recursive_mutex mtx_chunks, mtx_dirty, mtx_edge, mtx_render;

	// Chunk data containers
	std::unordered_map< int, Handle< Chunk > > map_chunks;
	std::unordered_map< int, ChunkNoise > map_noise;
	std::unordered_map< int, ChunkFile > map_file;

	std::unordered_map< int, Chunk & > map_dirty;
	std::unordered_map< int, Chunk & > map_edge;

	std::unordered_map< int, Chunk & > map_render;

	// Mesh pool data
	static int const size_pool_buff = 128;
	std::mutex mtx_pool_buff;
	std::array< ChunkBuffer, size_pool_buff > list_pool_buff;
	std::vector< ChunkBuffer * > list_avail_buff;

	std::vector< Chunk * > list_render;

	// Emitter data
	LightData light_data;

private:
	GLuint const SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;
	GLfloat const near_plane = 1.0f;
	GLfloat const far_plane = 756.0f;
	GLfloat const dim_ortho = 64.0f;

	GLuint id_depth_fbo;
	GLuint id_tex_depth;

	// Light functions
	void init_light( );
	void calc_light( );
	void proc_set_state( SetState & state );

	// Mesh functions
	ChunkBuffer * get_buffer( );
	void put_buffer( ChunkBuffer *& buffer );

	void chunk_state( Chunk & chunk, ChunkState const state, bool flag );
	void chunk_state_clear( Chunk & chunk );

	// Chunk functions
	void render_skybox( );
	void render_exlude( );
	void render_sort( );
	void render_pass_shadow( );
	void render_pass_norm( );
	void render_debug( );

	void chunk_update( Chunk & chunk );
	void chunk_render( Chunk & chunk );
	void chunk_render_shadowmap( Chunk & chunk );

	void chunk_vbo( Chunk & chunk );

	void chunk_add( glm::ivec3 const & pos_lw );
	void chunk_shutdown( Chunk & chunk );

	//void chunk_wait( Chunk & chunk );

	void chunk_init( Chunk & chunk );
	void chunk_read( Chunk & chunk );
	void chunk_load( Chunk & chunk );
	void chunk_gen( Chunk & chunk );

	void chunk_smesh( Chunk & chunk );
	void chunk_tmesh( Chunk & chunk );
	void chunk_buffer( Chunk & chunk );

	void chunk_save( Chunk & chunk );
	void chunk_remove( Chunk & chunk );

	// Block dictionary functions
	void load_block_data( );

public:
	ChunkMgr( Client & client );
	~ChunkMgr( );

public:
	// General Public functions
	void init( );
	void update( );
	void render( );
	void end( );
	void sec( );

	void toggle_chunk_debug( );
	void toggle_shadow_debug( );
	void toggle_shadows( );

	int get_block( glm::vec3 const & pos_gw );

	// Block manipulation functions
	void set_block( glm::ivec3 const & pos_gw, int const id );
	void set_block( glm::vec3 const & pos_gw, int const id );
	void set_block( glm::ivec3 const & pos_gw, SetState & state, int const id );

	void set_sphere( glm::ivec3 const & pos_gw, int const size, int const id );
	void set_sphere( glm::vec3 const & pos_gw, int const size, int const id );
	void set_sphere( glm::ivec3 const & pos_gw, SetState & state, int const size, int const id );

	void set_rect( glm::ivec3 const & pos_gw, glm::ivec3 const & dim, int const id );
	void set_rect( glm::vec3 const & pos_gw, glm::ivec3 const & dim, int const id );
	void set_rect( glm::ivec3 const & pos_gw, SetState & state, glm::ivec3 const & dim, int const id );

	void set_ellipsoid( glm::ivec3 const & pos_gw, glm::ivec3 const & dim, int const id );
	void set_ellipsoid( glm::vec3 const & pos_gw, glm::ivec3 const & dim, int const id );
	void set_ellipsoid( glm::ivec3 const & pos_gw, SetState & state, glm::ivec3 const & dim, int const id );

	void set_tree( glm::ivec3 const & pos_gw, int const id );
	void set_tree( glm::vec3 const & pos_gw, int const id );
	void set_tree( glm::ivec3 const & pos_gw, SetState & state, int const id );

	void explode_sphere( glm::vec3 const & pos_gw, int const size );
	void explode_sphere_recur( glm::vec3 const & pos_gw, int const size, int depth );

	// Block data accessor
	Block & get_block_data( int const id );
	Block & get_block_data( std::string const & name );
	Block * get_block_data_safe( std::string const & name );
	std::string const & get_block_string( int const id );

	int get_num_blocks( );

	void set_sun( int deg ) {
		pos_deg_light = deg;
	}

	void set_sun_pause( bool is_paused ) { 
		is_sun_pause = is_paused;
	}

	void print_dirty( );
	void print_prio( int const range, int const base_prio );

	// Emitter Functions
	void add_emitter( Emitter & emitter );
	void clear_emitters( );
	LightData & get_light_data( ) {
		return light_data;
	}
};

// Change to block pointer and face

extern void put_face( 
	std::vector< ChunkFaceVertices > & buffer_verts, glm::ivec3 const & pos,
	FaceVerts const & verts, glm::vec4 const & color, 
	glm::vec3 const & normal, FaceUvs const & uvs );

extern void put_face(
	std::vector< ChunkFaceVertices > & buffer_verts, glm::ivec3 const & pos,
	FaceVerts const & verts, glm::vec4 const & color,
	FaceNorms const & normal, FaceUvs const & uvs );

extern void put_face(
	std::vector< ChunkFaceVertices > & buffer_verts, glm::ivec3 const & pos,
	FaceVerts const & verts, glm::ivec3 const & scale_verts,
	glm::vec4 const & color, glm::vec3 const & normal,
	FaceUvs const & uvs, glm::ivec2 const & scale_uvs );