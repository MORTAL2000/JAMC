#pragma once

#include "Globals.h"

#include <vector>
#include <unordered_map>

#include "Manager.h"
#include "BiomeLoader.h"

class BiomeMgr : public Manager {
private:
	std::vector< BiomeLoader > list_biomes;
	std::unordered_map< std::string, int > map_biome_name;

public:
	BiomeMgr( Client & client );
	~BiomeMgr( );

private:
	void add_biome( BiomeLoader * loader_biome );

public:
	void init( ) override;
	void update( ) override;
	void render( ) override;
	void end( ) override;
	void sec( ) override;

	int get_num_biomes( );

	int get_biome_id( std::string const & name_biome );
	int get_biome_id_safe( std::string const & name_biome );

	BiomeLoader * get_biome( int id_biome );
	BiomeLoader * get_biome_safe( int id_biome );
	BiomeLoader * get_biome( std::string const & name_biome );
	BiomeLoader * get_biome_safe( std::string const & name_biome );

	void get_biome_data( int x, int y, int & id_biome, int & height_biome );

	//int get_biome_noise_id( int x, int y );
	//int get_biome_noise_height( int x, int y, int id_biome );

};