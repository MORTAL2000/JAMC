#pragma once
#include "Globals.h"
#include "ChunkMgr.h"
#include "VBO.h"

class BlockSelector {
private:
	static int const size_max = 75;
	static int const size_min = 40;
	static int const num_hist = 6;
	Client & client;
	int id_block;
	VBO vbo;

	glm::mat4 mat_translate;
	glm::mat4 mat_rotate;
	glm::mat4 mat_scale;

	bool is_dirty;

	void mesh( );

public:
	BlockSelector( Client & client );
	~BlockSelector( );

	void init( );
	void set_dirty( );
	void set_id_block( int id_block );
	int get_id_block( );
	void change_id_block( int delta_id_block );
	void update( );

	void render( );
};
