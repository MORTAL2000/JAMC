#include "SandPlateauBiome.h"

#include "Client.h"

SandPlateauBiome::SandPlateauBiome( Client & client ) { 
	name_biome = "Sand Plateau";
	id_block_surface = client.block_mgr.get_block_loader( "Sand" )->id;
	id_block_depth = client.block_mgr.get_block_loader( "Sand" )->id;

	noise_biome = {
		0,
		-100, 100,
		10101.2f, 10101.2f,
		0.0005f, 0.0005f, 100.0f,
		{ { 50.0f, 100.0f } }
	};

	noise_lerp = {
		0,
		-100, 100,
		10101.2f, 10101.2f,
		0.0005f, 0.0005f, 100.0f,
		{ { 50.0f, 55.0f } }
	};

	list_noise.push_back( {
		10,
		0, 12,
		10101.2f, 10101.2f,
		0.01f, 0.01f, 10.0f,
		{ { 0.0f, 10.0f } }
	} );

	list_noise.push_back( {
		10,
		0, 12,
		10101.2f, 10101.2f,
		0.01f, 0.01f, 10.0f,
		{ { 0.0f, 10.0f } }
	} );

	list_noise.push_back( {
		10,
		0, 15,
		10101.2f, 10101.2f,
		0.01f, 0.01f, 10.0f,
		{ { 2.5f, 10.0f } }
	} );

	list_noise.push_back( {
		10,
		0, 17,
		10101.2f, 10101.2f,
		0.01f, 0.01f, 10.0f,
		{ { 5.0f, 10.0f } }
	} );

	list_noise.push_back( {
		10,
		0, 20,
		10101.2f, 10101.2f,
		0.01f, 0.01f, 10.0f,
		{ { 7.5f, 10.0f } }
	} );
}


SandPlateauBiome::~SandPlateauBiome( ) { }
