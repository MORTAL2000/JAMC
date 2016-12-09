#pragma once

#include "Globals.h"
#include "Entity.h"

#include "glm\gtx\hash.hpp"

struct ECWater { 
	float cld_update;
	float time_update;
	float cld_static;
	float time_static;
	float time_now;

	GLuint depth_flow;

	glm::ivec3 pos_curr;
	glm::ivec3 pos_check;

	int id_curr;
	int id_check;

	BlockLoader * block_curr;
	BlockLoader * block_check;

	bool is_start;

	std::unordered_map< glm::ivec3, GLuint > map_grow;
	std::vector< std::pair< glm::ivec3, GLuint > > list_add;
	std::list< std::pair< glm::ivec3, GLuint > > list_static;
};

class WaterBlock : public EntityLoader {
public:
	WaterBlock( );
	~WaterBlock( );
};

