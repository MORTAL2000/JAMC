#pragma once

#include "Globals.h"
#include "glm/glm.hpp"

struct ChunkDataState { 
	glm::vec3 pos_lw;
	glm::vec3 pos_gw;

	static Vect3< int > size_blocks;
	static Vect3< int > dimension_chunk;
};

