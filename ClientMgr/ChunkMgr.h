#pragma once

#include "Globals.h"

#include "Manager.h"
#include "Chunk.h"
#include "WorldSize.h"
#include "ResPool.h" 
#include "VBO.h"
#include "BlockLoader.h"

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
	static int const max_emitters = 8;

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

struct SectionIndex { 
	static int const size_section_bytes = 4096;
	static int const num_sections = 64;

	int index_section[ num_sections ];
};

struct FileHeader { 
	static int const num_regions = 
		WorldSize::Region::size_x * WorldSize::Region::size_y * WorldSize::Region::size_z;
	
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
	int height[ WorldSize::Chunk::size_x ][ WorldSize::Chunk::size_z ];
	int biome[ WorldSize::Chunk::size_x ][ WorldSize::Chunk::size_z ];
	int envir[ WorldSize::Chunk::size_x ][ WorldSize::Chunk::size_z ];
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
	SMTerrain sm_terrain;
	SMChunkIncl sm_inclusive;

	std::vector< SMChunkIncl::SMHandle > list_handles_inclusive;

	static int const dist_sun = 
		WorldSize::Chunk::size_x * WorldSize::World::size_x;
	bool is_sun_pause;
	float pos_deg_light;

	bool is_chunk_debug;
	bool is_shadow_debug;
	bool is_shadows;
	bool is_shadow_solid;
	bool is_shadow_trans;
	bool is_render_solid;
	bool is_render_trans;
	bool is_flatshade;
	bool is_wireframe;

	GLuint id_vbo_chunk_outline;
	int size_chunk_outline;

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

	std::vector< Chunk * > list_render;

	// Emitter data
	LightData light_data;

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

private:
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

	void chunk_set_manip( Chunk & chunk, short unsigned x, short unsigned y, short unsigned z, short id );
	void chunk_manip( Chunk & chunk );

	void chunk_init( Chunk & chunk );
	void chunk_read( Chunk & chunk );
	void chunk_load( Chunk & chunk );
	void chunk_gen( Chunk & chunk );

	void chunk_mesh( Chunk & chunk );
	void chunk_buffer( Chunk & chunk );

	void chunk_save( Chunk & chunk );
	void chunk_remove( Chunk & chunk );

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
	void toggle_shadow_solid( );
	void toggle_shadow_trans( );
	void toggle_flatshade( );
	void toggle_wireframe( );
	void toggle_render_solid( );
	void toggle_render_trans( );

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

	void set_sun_deg( int deg ) {
		pos_deg_light = deg;
	}

	float get_sun_deg( ) { 
		return pos_deg_light;
	}

	void set_sun_pause( bool is_paused ) { 
		is_sun_pause = is_paused;
	}

	bool get_sun_pause( ) { 
		return is_sun_pause;
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
		BlockLoader * block, FaceDirection face );

	inline void put_sort(
		std::vector< std::pair< float, GLuint > > & list_sort,
		glm::vec3 & pos_gw, glm::vec3 & scale,
		BlockLoader * block, FaceDirection face );
};

// Change to block pointer and face

extern inline void put_face(
	SMTerrain::SMTHandle & handle, glm::ivec3 const & pos,
	glm::vec4 const & color, FaceDirection dir, Face const & face );

extern inline void put_face(
	SMTerrain::SMTHandle & handle, glm::ivec3 const & pos,
	glm::vec4 const & color, FaceDirection dir, Face const & face,
	glm::vec3 const & scale_verts, glm::vec2 const & scale_uvs );

