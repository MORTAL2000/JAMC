#pragma once

#include "Globals.h"
#include "Entity.h"

struct ECGravBlock {
	enum StateBlock {
		SB_Active,
		SB_Snap,
		SB_Size
	};

	StateBlock state;
	int time_life;

	int time_snap_length;
	int time_snap_start;
	int time_snap_end;

	glm::vec3 pos_snap_start;
	glm::vec3 pos_snap_end;
};

class GravBlock : public EntityLoader{
public:
	GravBlock( );
	~GravBlock( );
};

