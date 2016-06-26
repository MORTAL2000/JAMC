#pragma once

#include "Globals.h"
#include "Entity.h"

struct ECPlayer {
	float veloc_vmax;
	float veloc_hmax;
	float veloc_walk;
	float veloc_run;
	float accel_walk;
	float accel_run;

	int cld_input;
	int last_input;

	bool is_standing;
	bool is_godmode;
};

class Player : public EntityLoader { 
public :
	Player( );
	~Player( );
};