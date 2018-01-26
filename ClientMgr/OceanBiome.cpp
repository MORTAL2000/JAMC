#include "OceanBiome.h"
#include "Client.h"

#include "BlockMgr.h"

OceanBiome::OceanBiome( Client & client ) { 
	name_biome = "Ocean";
	id_block_surface = client.block_mgr.get_block_loader( "Sand" )->id;
	id_block_depth = client.block_mgr.get_block_loader( "Sand" )->id;

	noise_biome = {
		0,
		-100, 100,
		6031.9f, 6031.9f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 100.0f } }
	};

	noise_lerp = {
		0,
		-100, 100,
		6031.9f, 6031.9f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 65.0f } }
	};

	list_noise.push_back( {
		-5,
		-30, -3,
		6631.9f, 6631.9f,
		0.01f, 0.01f, 30.0f,
		{ { -30.0f, 30.0f } }
	} );

	list_noise.push_back( {
		-10,
		-50, -5,
		6931.9f, 6931.9f,
		0.01f, 0.01f, 40.0f,
		{ { -40.0f, 40.0f } }
	} );
}


OceanBiome::~OceanBiome( ) { }
