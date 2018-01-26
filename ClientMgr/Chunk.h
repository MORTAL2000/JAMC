#pragma once
#include "Globals.h"

#include "SharedMesh.h"
#include "SharedMeshTerrain.h"
#include "BlockSet.h"
#include "Directional.h"

#include <array>
#include <mutex>
#include <vector>

struct VertTerrain {
	uint64_t x : 5;			// 0-4
	uint64_t y : 6;			// 5-10
	uint64_t z : 5;			// 11-15

	uint64_t r : 5;			// 16-20
	uint64_t g : 5;			// 21-25
	uint64_t b : 5;			// 26-30
	uint64_t a : 5;			// 31, 0-3

	uint64_t tex1 : 10;		// 4-13
	uint64_t tex2 : 10;		// 14-23

	uint64_t orient : 3;	// 24-26
	uint64_t scale : 5;		// 27-31
};

using SMTerrain = SharedMeshTerrain< VertTerrain >;

using InclusiveVert = BaseVertex< GLfloat, GLfloat, GLfloat, GLfloat >;
using SMChunkIncl = SharedMesh< InclusiveVert >;
using SMHandle = SMChunkIncl::SharedMeshHandle;

enum ChunkState {
	CS_Init,
	CS_Read,
	CS_Load,
	CS_Adj,
	CS_Gen,
	CS_Manip,
	CS_Encode,
	CS_Decode,
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
	glm::ivec3 pos_lw;
	glm::ivec3 pos_gw;

	glm::mat4 mat_model;

	int hash_lw;
	int hash_lw_2d;

	BlockSet< WorldSize::Chunk::size_x, WorldSize::Chunk::size_z > block_set;

	std::recursive_mutex mtx_state;
	int cnt_states;
	bool states[ CS_Size ];

	int cnt_adj;
	std::recursive_mutex mtx_adj;
	std::array< Chunk *, FaceDirection::FD_Size > ptr_adj;

	SMHandle handle_inclusive;
	SMHandle handle_inclusive_temp;

	SMTerrain::SMTHandle handle_solid;
	SMTerrain::SMTHandle handle_solid_temp;

	SMTerrain::SMTHandle handle_trans;
	SMTerrain::SMTHandle handle_trans_temp;
	
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