#include "BiomeMgr.h"

#include "Client.h"

#include "BaseBiome.h"
#include "IceShardBiome.h"
#include "SandPlateauBiome.h"
#include "CobblestoneChasmBiome.h"
#include "GrassHillBiome.h"
#include "GrassPlateauBiome.h"
#include "OceanBiome.h"

BiomeMgr::BiomeMgr( ) : 
	Manager( ) { }

BiomeMgr::~BiomeMgr( ) { }

void BiomeMgr::init( ) { 
	add_biome( &BaseBiome( ) );
	add_biome( &IceShardBiome( ) );
	add_biome( &SandPlateauBiome( ) );
	add_biome( &CobblestoneChasmBiome( ) );
	add_biome( &GrassHillBiome( ) );
	add_biome( &GrassPlateauBiome( ) );
	add_biome( &OceanBiome( ) );
}

void BiomeMgr::update( ) { }

void BiomeMgr::render( ) { }

void BiomeMgr::end( ) { }

void BiomeMgr::sec( ) { }

void BiomeMgr::add_biome( BiomeLoader * loader_biome ) { 
	BiomeLoader biome;

	biome.name_biome = loader_biome->name_biome;

	biome.noise_biome = loader_biome->noise_biome;
	biome.noise_lerp = loader_biome->noise_lerp;

	biome.id_block_surface = loader_biome->id_block_surface;
	biome.id_block_depth = loader_biome->id_block_depth;

	for( int unsigned i = 0; i < loader_biome->list_noise.size( ); ++i ) {
		biome.list_noise.push_back( loader_biome->list_noise[ i ] );
	}

	list_biomes.push_back( biome );
	map_biome_name.insert( { biome.name_biome, ( GLuint ) list_biomes.size( ) - 1 } );
}

int BiomeMgr::get_num_biomes( ) { 
	return list_biomes.size( );
}

int BiomeMgr::get_biome_id( std::string const & name_biome ) { 
	return map_biome_name[ name_biome ];
}

int BiomeMgr::get_biome_id_safe( std::string const & name_biome ) { 
	auto iter_biome = map_biome_name.find( name_biome );

	if( iter_biome != map_biome_name.end( ) ) { 
		return iter_biome->second;
	}

	else return -1;
}

BiomeLoader * BiomeMgr::get_biome( int id_biome ) { 
	return &list_biomes[ id_biome ];
}

BiomeLoader * BiomeMgr::get_biome_safe( int id_biome ) {
	if( id_biome < 0 || id_biome >= list_biomes.size( ) ) { 
		return nullptr;
	}

	return &list_biomes[ id_biome ];
}

BiomeLoader * BiomeMgr::get_biome( std::string const & name_biome ) { 
	return &list_biomes[ map_biome_name[ name_biome ] ];
}

BiomeLoader * BiomeMgr::get_biome_safe( std::string const & name_biome ) { 
	auto iter_biome = map_biome_name.find( name_biome );

	if( iter_biome == map_biome_name.end( ) ) { 
		return nullptr;
	}

	return &list_biomes[ iter_biome->second ];
}
