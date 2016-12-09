#pragma once

#include "Globals.h"

#include <vector>

struct NoiseLayer {
	int height_base;
	int height_min, height_max;

	float seed_x, seed_y;

	float scale_x, scale_y;
	float scale_height;

	std::vector< std::pair< float, float > > list_bounds;
};

class BiomeLoader {
private:

public:	

	BiomeLoader( );
	~BiomeLoader( );

	std::string name_biome;

	NoiseLayer noise_biome;
	NoiseLayer noise_lerp;

	int id_block_surface;
	int id_block_depth;

	std::vector< NoiseLayer > list_noise;

private:

public:

};

