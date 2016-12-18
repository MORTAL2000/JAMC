#include "BiomeMgr.h"

#include "Client.h"

#include "simplexnoise.h"

#include "BaseBiome.h"
#include "IceShardBiome.h"
#include "SandPlateauBiome.h"
#include "CobblestoneChasmBiome.h"
#include "GrassHillBiome.h"
#include "GrassPlateauBiome.h"
#include "OceanBiome.h"
#include "SandFlatsBiome.h"

BiomeMgr::BiomeMgr( Client & client ) : 
	Manager( client ) { }

BiomeMgr::~BiomeMgr( ) { }

void BiomeMgr::init( ) { 
	add_biome( &BaseBiome( client ) );

	add_biome( &OceanBiome( client ) );
	add_biome( &CobblestoneChasmBiome( client ) );
	add_biome( &SandFlatsBiome( client ) );

	add_biome( &GrassHillBiome( client ) );

	add_biome( &IceShardBiome( client ) );

	add_biome( &SandPlateauBiome( client ) );
	add_biome( &GrassPlateauBiome( client ) );
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
	return ( int ) list_biomes.size( );
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

void BiomeMgr::get_biome_data( int x, int y, int & id_biome, int & height_biome ) {
	static auto biome_base = client.biome_mgr.get_biome( "Base" );
	float noise, noise_lerp, noise_total;

	noise_total = 0;
	for( auto & layer_noise : biome_base->list_noise ) {
		noise = raw_noise_2d(
			layer_noise.seed_x + x * layer_noise.scale_x,
			layer_noise.seed_y + y * layer_noise.scale_y ) *
			layer_noise.scale_height;

		for( auto & pair_bounds : layer_noise.list_bounds ) {
			if( noise >= pair_bounds.first &&
				noise <= pair_bounds.second ) {

				noise += layer_noise.height_base;
				if( noise < layer_noise.height_min ) { noise = layer_noise.height_min; }
				if( noise > layer_noise.height_max ) { noise = layer_noise.height_max; }

				noise_total += noise;

				break;
			}
		}
	}

	id_biome = 0;
	height_biome = noise_total;

	for( GLuint idx_biome = 1; idx_biome < client.biome_mgr.get_num_biomes( ); ++idx_biome ) {
		auto biome = client.biome_mgr.get_biome( idx_biome );
		auto & layer_biome = biome->noise_biome;

		noise = raw_noise_2d(
			layer_biome.seed_x + x * layer_biome.scale_x,
			layer_biome.seed_y + y * layer_biome.scale_y ) *
			layer_biome.scale_height;

		for( auto & pair_bounds : layer_biome.list_bounds ) {
			if( noise >= pair_bounds.first &&
				noise <= pair_bounds.second ) {

				id_biome = idx_biome;
				break;
			}
		}

		if( id_biome == idx_biome ) {
			noise_total = 0;

			for( auto & layer_height : biome->list_noise ) {
				noise = raw_noise_2d(
					layer_height.seed_x + x * layer_height.scale_x,
					layer_height.seed_y + y * layer_height.scale_y ) *
					layer_height.scale_height;

				for( auto & pair_bounds : layer_height.list_bounds ) {
					if( noise >= pair_bounds.first &&
						noise <= pair_bounds.second ) {

						noise += layer_height.height_base;
						if( noise < layer_height.height_min ) { noise = layer_height.height_min; }
						if( noise > layer_height.height_max ) { noise = layer_height.height_max; }

						noise_total += noise;
						break;
					}
				}
			}

			auto & layer_lerp = biome->noise_lerp;
			noise_lerp = 1.0f;

			noise = raw_noise_2d(
				layer_lerp.seed_x + x * layer_lerp.scale_x,
				layer_lerp.seed_y + y * layer_lerp.scale_y ) *
				layer_lerp.scale_height;

			for( auto & pair_bounds : layer_lerp.list_bounds ) {
				if( noise >= pair_bounds.first &&
					noise <= pair_bounds.second ) {

					noise_lerp = ( noise - pair_bounds.first ) / ( pair_bounds.second - pair_bounds.first );

					break;
				}
			}

			height_biome = height_biome * ( 1.0f - noise_lerp ) + noise_lerp * noise_total;
		}
	}
}

/*
int BiomeMgr::get_biome_noise_id( int x, int y ) {
	BiomeLoader * biome_loader;
	NoiseLayer * biome_layer;
	std::pair< float, float > * biome_bounds;
	float noise, id_biome;

	noise = 0;
	id_biome = 0;
	for( int i = 0; i < get_num_biomes( ); ++i ) { 
		biome_loader = get_biome( i );
		biome_layer = &biome_loader->noise_biome;

		noise = raw_noise_2d(
			( biome_layer->seed_x + x ) * biome_layer->scale_x,
			( biome_layer->seed_y + y ) * biome_layer->scale_y
		) * biome_layer->scale_height;

		for( int j = 0; j < biome_layer->list_bounds.size( ); ++j ) {
			biome_bounds = &biome_layer->list_bounds[ j ];

			if( noise >= biome_bounds->first && noise <= biome_bounds->second ) { 
				id_biome = i;
			}
		}
	}

	return id_biome;
}

int BiomeMgr::get_biome_noise_height( int x, int y, int id_biome ) {
	BiomeLoader * biome_loader;
	NoiseLayer * layer;
	std::pair< float, float > * bounds;
	float noise, height, lerp;

	noise = 0;
	height = 0;

	biome_loader = get_biome( "Base" );

	for( int i = 0; i < biome_loader->list_noise.size( ); ++i ) {
		layer = &biome_loader->list_noise[ i ];

		noise = raw_noise_2d(
			( layer->seed_x + x ) * layer->scale_x,
			( layer->seed_y + y ) * layer->scale_y
		) * layer->scale_height;

		for( int j = 0; j < layer->list_bounds.size( ); ++j ) {
			bounds = &layer->list_bounds[ j ];

			if( noise >= bounds->first && noise <= bounds->second ) {
				noise += layer->height_base;

				if( noise > layer->height_max ) noise = layer->height_max;
				if( noise < layer->height_min ) noise = layer->height_min;

				break;
			}
		}
	}
}
*/