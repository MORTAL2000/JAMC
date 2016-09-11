#pragma once

#include "Globals.h"

#include "Directional.h"
#include "SharedMesh.h"

#include <array>
#include <mutex>
#include <vector>

using ChunkVert = BaseVertex< GLubyte, GLubyte, GLbyte, GLubyte >;
using InclusiveVert = BaseVertex< GLfloat, GLfloat, GLfloat, GLfloat >;

using SMChunk = SharedMesh< ChunkVert >;
using SMChunkIncl = SharedMesh< InclusiveVert >;

enum ChunkState {
	CS_Init,
	CS_Read,
	CS_Load,
	CS_Wait,
	CS_Gen,
	CS_SMesh,
	CS_TMesh,
	CS_SBuffer,
	CS_TBuffer,
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

	SMChunk::SMHandle handle_solid;
	SMChunk::SMHandle handle_solid_temp;

	SMChunk::SMHandle handle_trans;
	SMChunk::SMHandle handle_trans_temp;

	std::vector< std::tuple< float, GLuint, glm::vec3 > > list_incl_solid;
	std::vector< std::tuple< float, GLuint, glm::vec3 > > list_incl_trans;

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