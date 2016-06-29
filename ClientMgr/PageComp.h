#pragma once

#include "Globals.h"

#include "glm/glm.hpp"
#include "PageCompFuncs.h"

class PageComp {
public:
	Client * client;
	Page * parent;

	glm::vec2 vec_anchor_pos;
	glm::vec2 vec_anchor_dim;

	glm::ivec2 vec_offset_pos;
	glm::ivec2 vec_offset_dim;

	glm::ivec2 vec_pos;
	glm::ivec2 vec_dim;

	glm::vec4 color;

	std::string str_name;

	bool is_hold;
	bool is_visible;

	PageComp( );
	~PageComp( );

	FuncComp func_on_down;
	FuncComp func_on_hold;
	FuncComp func_on_up;
	FuncComp func_on_action;

	FuncComp func_update;

	void position( );
};

