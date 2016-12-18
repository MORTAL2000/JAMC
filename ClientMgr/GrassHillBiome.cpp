#include "GrassHillBiome.h"

#include "Client.h"

GrassHillBiome::GrassHillBiome( Client & client ) { 
	name_biome = "Grass Hill";
	id_block_surface = client.block_mgr.get_block_loader( "Grass" )->id;
	id_block_depth = client.block_mgr.get_block_loader( "Dirt" )->id;

	noise_biome = {
		0,
		-100, 100,
		3031.9f, 3031.9f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 100.0f } }
	};

	noise_lerp = {
		0,
		-100, 100,
		3031.9f, 3031.9f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 55.0f } }
	};

	list_noise.push_back( {
		0,
		-30, 30,
		101.2f, 101.2f,
		0.008f, 0.008f, 30.0f,
		{ { -30.0f, 30.0f } }
	} );
}


GrassHillBiome::~GrassHillBiome( ) { }
