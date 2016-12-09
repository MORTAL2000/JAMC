#include "GrassPlateauBiome.h"

#include "Client.h"

GrassPlateauBiome::GrassPlateauBiome( ) { 
	name_biome = "Grass Plateau";
	id_block_surface = get_client( ).block_mgr.get_block_loader( "Grass" )->id;
	id_block_depth = get_client( ).block_mgr.get_block_loader( "Dirt" )->id;

	noise_biome = {
		0,
		-100, 100,
		331.9f, 331.9f,
		0.001f, 0.001f, 100.0f,
		{ { 50.0f, 100.0f } }
	};

	noise_lerp = {
		0,
		-100, 100,
		331.9f, 331.9f,
		0.001f, 0.001f, 100.0f,
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


GrassPlateauBiome::~GrassPlateauBiome( ) { }
