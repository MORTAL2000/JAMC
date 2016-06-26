#pragma once

#include "Globals.h"
#include "Entity.h"

struct ECSpawnBlock {
	int time_last;
	int time_update;
	int time_life;
	int num_spawn;
};

class SpawnBlock : public EntityLoader {
public:
	SpawnBlock( );
	~SpawnBlock( );
};

