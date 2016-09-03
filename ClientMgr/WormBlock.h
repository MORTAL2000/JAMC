#pragma once

#include "Entity.h"

struct ECWorm { 
	glm::ivec3 pos_curr;
	glm::ivec3 pos_last;
	glm::vec3 heading;

	GLuint num_visited;
	GLuint cnt_step;

	float scale_x;
	float scale_y;

	float seed_x;
	float seed_y;

	float max_x;
	float max_y;

	float speed;

	float time_life;
};

class WormBlock : public EntityLoader {
public:
	WormBlock( );
	~WormBlock( );
};

