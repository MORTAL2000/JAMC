#include "CobblestoneChasmBiome.h"

#include "BlockMgr.h"

#include "Client.h"

CobblestoneChasmBiome::CobblestoneChasmBiome( ) { 
	name_biome = "Cobblestone Chasm";
	id_block_surface = get_client( ).block_mgr.get_block_loader( "Cobblestone" )->id;
	id_block_depth = get_client( ).block_mgr.get_block_loader( "Cobblestone" )->id;

	noise_biome = {
		0,
		-100, 100,
		1901.2f, 1101.2f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 100.0f } }
	};

	noise_lerp = {
		0,
		-100, 100,
		1901.2f, 1101.2f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 55.0f } }
	};

	list_noise.push_back( {
		0,
		-6, 6,
		1231.2f, 1231.2f,
		0.01f, 0.01f, 6.0f,
		{ { -6.0f, 6.0f } }
	} );

	list_noise.push_back( {
		30,
		-30, 0,
		234.2f, 234.2f,
		0.03f, 0.009f, 60.0f,
		{ { -60.0f, -30.0f } }
	} );
}


CobblestoneChasmBiome::~CobblestoneChasmBiome( ) { }
