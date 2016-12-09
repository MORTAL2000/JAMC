#include "IceShardBiome.h"

#include "Client.h"

IceShardBiome::IceShardBiome( ) { 
	name_biome = "Ice Shards";
	id_block_surface = get_client( ).block_mgr.get_block_loader( "Ice" )->id;
	id_block_depth = get_client( ).block_mgr.get_block_loader( "Ice" )->id;

	noise_biome = {
		0,
		-100, 100,
		991.2f, 2991.2f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 100.0f } }
	};

	noise_lerp = {
		0,
		-100, 100,
		991.2f, 2991.2f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 52.5f } }
	};

	list_noise.push_back( {
		10,
		10, 10,
		0, 0,
		0.1f, 0.1f, 0.0f,
		{ { 0.0f, 0.0f } }
	} );

	list_noise.push_back( {
		0,
		0, 15,
		0, 0,
		0.05f, 0.05f, 15.0f,
		{ { -15.0f, 15.0f } }
	} );
}


IceShardBiome::~IceShardBiome( ) { }
