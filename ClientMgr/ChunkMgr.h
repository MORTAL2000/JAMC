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
	static int const size_x = 12;
	static int const size_y = 4;
	static int const size_z = 12;
	static glm::ivec3 const size_vect;
	static int const num_chunks = 
		( size_x * 2 + 1 ) * ( size_y * 2 + 1 ) * ( size_z * 2 + 1 ) + 
		( size_x * 2 + 1 ) * ( size_z * 2 + 1 );
	static int const level_sea = 0;
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

struct NoiseLayer { 
	int height_base;
	int height_min, height_max;

	float seed_x, seed_y;

	float scale_x, scale_y;
	float scale_height;
	std::vector< std::pair< float, float > > list_bounds;
};

struct Biome {
	std::string name_biome;
	NoiseLayer noise_biome;
	NoiseLayer noise_lerp;
	int id_block_surface;
	int id_block_depth;
	std::vector< NoiseLayer > list_noise;
};

struct ChunkNoise {
	int cnt_using = 0;
	int height[ Chunk::size_x ][ Chunk::size_z ];
	int biome[ Chunk::size_x ][ Chunk::size_z ];
	int envir[ Chunk::size_x ][ Chunk::size_z ];
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
	GLuint num_triangles;
	GLuint num_cmds;

	// Mesh;
	VBO vbo_skybox;
	VBO vbo_sun;
	VBO vbo_debug_chunk;
	SMChunk shared_mesh;
	SMChunkIncl shared_mesh_inclusive;

	std::vector< SMChunkIncl::SMHandle > list_handles_inclusive;

	static int const dist_sun = 750;
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

	// Biome Data
	std::vector< Biome > list_biomes;
	std::unordered_map< std::string, GLuint > map_biome_name;

	// Chunk data containers
	std::unordered_map< int, Handle< Chunk > > map_chunks;
	std::unordered_map< int, ChunkNoise > map_noise;
	std::unordered_map< int, ChunkFile > map_file;

	std::unordered_map< int, Chunk & > map_dirty;
	std::unordered_map< int, Chunk & > map_edge;

	std::unordered_map< int, Chunk & > map_render;

	std::vector< Chunk * > list_render;

	// Emitter data
	LightData light_data;

private:
	GLuint idx_solid;
	GLuint idx_trans;

	GLuint idx_cascade = 0;

	static GLuint constexpr num_cascades = 3;

	glm::mat4 mat_model;
	glm::mat3 mat_norm;
	glm::mat4 mat_view_light;
	glm::mat4 mat_ortho_light[ num_cascades ];

	glm::vec2 dim_shadow;
	glm::vec2 pos_shadow;

	GLuint const SHADOW_WIDTH;
	GLuint const SHADOW_HEIGHT;
	GLfloat depth_cascades[ num_cascades ];

	glm::vec4 corners_frustrum[ num_cascades ][ 8 ];
	float sides_ortho[ num_cascades ][ 6 ];

	GLuint id_depth_fbo;
	GLuint id_tex_depth[ num_cascades ];
	GLuint id_tex_blur;

	// Light functions
	void init_light( );
	void calc_light( );

	// Skybox functions
	GLuint id_skybox;
	void init_skybox( );
	void mesh_skybox( );

	// ShadowMap functions
	void init_shadowmap( );

	// Chunk Debug functions
	void init_debug( );

	// Chunk State functions
	void chunk_state( Chunk & chunk, ChunkState const state, bool flag );
	void chunk_state_clear( Chunk & chunk );
	void proc_set_state( SetState & state );

	// Chunk functions
	void render_skybox( );
	void render_exlude( );
	void render_sort( );
	void render_build( );
	void render_pass_shadow( );
	void render_pass_solid( );
	void render_pass_trans( );
	void render_debug( );

	void chunk_update( Chunk & chunk );

	void chunk_add( glm::ivec3 const & pos_lw );
	void chunk_shutdown( Chunk & chunk );

	//void chunk_wait( Chunk & chunk );

	void chunk_init( Chunk & chunk );
	void chunk_read( Chunk & chunk );
	void chunk_load( Chunk & chunk );
	void chunk_gen( Chunk & chunk );

	void chunk_mesh( Chunk & chunk );
	void chunk_buffer( Chunk & chunk );

	void chunk_save( Chunk & chunk );
	void chunk_remove( Chunk & chunk );

	// Block dictionary functions
	void load_block_data( );
	void load_block_mesh( );

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

	void next_skybox( );

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

	void print_center_chunk_mesh( );

	inline void put_sort(
		std::vector< std::pair< float, GLuint > > & list_sort,
		glm::vec3 & pos_gw,
		Block & block, FaceDirection face );

	inline void put_sort(
		std::vector< std::pair< float, GLuint > > & list_sort,
		glm::vec3 & pos_gw, glm::vec3 & scale,
		Block & block, FaceDirection face );
};

// Change to block pointer and face

extern inline void put_face(
	SMChunk::SMHandle & handle, glm::ivec3 const & pos,
	glm::vec4 const & color, Face const & face );

extern inline void put_face(
	SMChunk::SMHandle & handle, glm::ivec3 const & pos,
	glm::vec4 const & color, Face const & face,
	glm::vec3 const & scale_verts, glm::vec2 const & scale_uvs );

