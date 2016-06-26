#pragma once

#include "Globals.h"
#include "Entity.h"

struct ECTnt {
	int time_last;
	int time_update;
	int time_life;
	int size_explosion;
};

class Tnt : public EntityLoader {
public:
	Tnt( );
	~Tnt( );
};

