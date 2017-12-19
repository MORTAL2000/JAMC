#pragma once

#include "glm\glm.hpp"

class WorldSize {
public:
	struct Chunk { 
		static int const size_x = 32;
		static int const size_y = 64;
		static int const size_z = 32;
		static glm::ivec3 const vec_size;
	};

	struct World {
		static int const size_x = 16;
		static int const size_y = 4;
		static int const size_z = 16;
		static glm::ivec3 const vec_size;
		static int const num_chunks =
			(size_x * 2 + 3) * (size_y * 2 + 3 + 1) * (size_z * 2 + 3);
		static int const level_sea = 0;
	};

	struct Region {
		static int const size_x = 16;
		static int const size_y = 4;
		static int const size_z = 16;
	};

};

