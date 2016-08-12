#pragma once
#include "Globals.h"
#include "ChunkMgr.h"
#include "VBO.h"
#include "SharedMesh.h"

class BlockSelector {
private:
	static int constexpr size_max = 75;
	static int constexpr size_min = 40;
	static int constexpr num_hist = 6;
	Client & client;
	int id_temp;
	int id_block;
	VBO vbo;
	//SharedMesh shared_mesh;
	//SharedMesh::SMHandle handles[ 2 ];

	glm::mat4 mat_translate;
	glm::mat4 mat_rotate;
	glm::mat4 mat_scale;

	bool is_dirty;

	void mesh( );

public:
	BlockSelector( Client & client );
	~BlockSelector( );

	//void clear_mesh( );
	//void release_mesh( );
	//void make_mesh( );

	void init( );
	void set_dirty( );
	void set_id_block( int id_block );
	int get_id_block( );
	void change_id_block( int delta_id_block );
	void update( );

	void render( );
};
