#pragma once

#include "Globals.h"

#include "Directional.h"

#include <array>
#include <mutex>
#include <vector>

struct ChunkVert { 
	GLfloat vert[ 3 ];
	GLfloat color[ 4 ];
	GLfloat norm[ 3 ];
	GLfloat uv[ 3 ];
	//GLfloat padding[ 4 ];
};

struct ChunkFaceIndices { 
	GLuint indicies[ 6 ];
};

struct ChunkFaceVertices { 
	ChunkVert verts[ 4 ];
};

struct ChunkBuffer {
	int size_solid, size_trans;
	std::vector< ChunkFaceIndices > list_indices;
	std::vector< ChunkFaceVertices > list_vertices_solid;
	std::vector< ChunkFaceVertices > list_vertices_trans;
	std::vector< std::pair< float, int > > list_sort;
};

enum ChunkState { 
	CS_Init,
	CS_Read,
	CS_Load,
	CS_Wait,
	CS_Gen,
	CS_SMesh,
	CS_TMesh,
	CS_Buffer,
	CS_Save,
	CS_Remove,
	CS_Size
};

class Chunk {
public:
	static int const size_x = 32;
	static int const size_y = 64;
	static int const size_z = 32;
	static glm::ivec3 const vec_size;
	static int const size_mesh_b = sizeof( ChunkVert ) * size_x * size_y * size_z * 4 * 6;
	static int const size_mesh_f = size_mesh_b / sizeof( GLfloat );

public:
	glm::ivec3 pos_lw;
	glm::ivec3 pos_gw;

	glm::mat4 mat_model;

	int hash_lw;
	int hash_lw_2d;

	short id_blocks[ Chunk::size_x ][ Chunk::size_y ][ Chunk::size_z ];

	std::recursive_mutex mtx_state;
	int cnt_states;
	bool states[ CS_Size ];

	int cnt_adj;
	std::mutex mtx_adj;
	std::array< Chunk *, FD_Size > ptr_adj;

	GLuint id_vao, id_vbo, id_ibo;

	int idx_solid, idx_trans;
	ChunkBuffer * ptr_buffer = nullptr;
	ChunkFile * ptr_file;
	ChunkNoise * ptr_noise;

	int cnt_solid;
	int cnt_air;

	bool is_loaded;
	bool is_shutdown;
	bool is_working;
	bool is_wait;

public:
	Chunk( );
	Chunk( Chunk const & other ) { }
	~Chunk( );

};