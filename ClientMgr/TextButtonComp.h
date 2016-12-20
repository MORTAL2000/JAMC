#pragma once

#include "glm\glm.hpp"

#include "PageComponentLoader.h"

class Client;

class TextButtonComp : public PageComponentLoader {
public:
	struct ButtonData {
		PCFunc func_action;

		glm::vec4 color;
		glm::vec4 color_down;
		glm::vec4 color_up;

		int unsigned id_texture;
		int unsigned id_subtex;

		PComp * comp_label;
	};

	TextButtonComp( Client & client );
	~TextButtonComp( );
};

