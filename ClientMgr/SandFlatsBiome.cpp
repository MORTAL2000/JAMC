#include "SandFlatsBiome.h"

#include "Client.h"


SandFlatsBiome::SandFlatsBiome( Client & client ) : 
	BiomeLoader( ) { 

	name_biome = "Sand Flats";
	id_block_surface = client.block_mgr.get_block_loader( "Sand" )->id;
	id_block_depth = client.block_mgr.get_block_loader( "Sand" )->id;

	noise_biome = {
		0,
		-100, 100,
		123.2f, 321.0f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 100.0f } }
	};

	noise_lerp = {
		0,
		-100, 100,
		123.2f, 321.0f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 55.0f } }
	};

	list_noise.push_back( { 
		0,
		-5, 5,
		123.2f, 321.0f,
		0.001f, 0.001f, 5.0f,
		{ { -5.0f, 5.0f } }
	} );
}


SandFlatsBiome::~SandFlatsBiome( ) { }
