#include "BaseBiome.h"

#include "Client.h"
//#include "BlockMgr.h"
//#include "BlockLoader.h"

BaseBiome::BaseBiome( Client & client ) {
	name_biome = "Base";
	id_block_surface = client.block_mgr.get_block_loader( "Grass" )->id;
	id_block_depth = client.block_mgr.get_block_loader( "Dirt" )->id;

	list_noise.push_back( {
		3,
		-7, 13,
		500.0f, 500.0f,
		0.001f, 0.001f, 10.0f,
		{ { -10.0f, 10.0f } }
	} );
}


BaseBiome::~BaseBiome( ) { 

}
