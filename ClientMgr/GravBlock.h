#pragma once

#include "Globals.h"
#include "Entity.h"

struct ECGravBlock {
	int time_life;
};

class GravBlock : public EntityLoader{
public:
	GravBlock( );
	~GravBlock( );
};

