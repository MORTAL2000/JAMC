#pragma once

#include "Globals.h"

#include "Manager.h"
#include "Entity.h"
#include "Color4.h"
#include "VBO.h"

#include <mutex>

enum ErrorEntity {
	EE_Ok,
	EE_Failed,
	EE_Size
};

struct ErrorEntityLookup { 
	static const char * to_text[ ]; 
};

struct ECPlayer { 

};

struct ECTnt {
	int time_last;
	int time_update;
	int time_life;
	int size_explosion;
};

struct ECGravBlock { 
	//int id_block;
	int time_life;
};

struct ECSpawnBlock { 
	int time_last;
	int time_update;
	int time_life;
	int num_spawn;
};

class EntityMgr : public Manager {
private:
	std::vector< EntityLoader > list_loader;
	std::unordered_map< std::string, int > map_loader;

	std::mutex mtx_entity;
	std::vector< Handle< Entity > > list_entity;
	std::unordered_map< std::string, Entity & > map_entity;

	VBO vbo;

public:
	EFAlloc alloc_base;
	EFCustom custom_base;
	EFRelease release_base;

private:
	void entity_integrate( ECState & ec_state );
	void entity_terrain_collide( ECState & ec_state );

	void entity_stop( ECState & ec_state );

	void entity_mesh( Entity & entity );

public:
	EntityMgr( Client & client );
	~EntityMgr( );

	void init( );
	void update( );
	void render( );
	void end( );
	void sec( );

	void loader_add( std::string const & str_name, EFAlloc ef_alloc, 
		EFRelease ef_release, EFUpdate ef_update, EFMesh ef_mesh );

	void entity_add( std::string const & str_name, EFCustom ef_custom );

	void entity_remove( Handle< Entity > & h_entity );
};

