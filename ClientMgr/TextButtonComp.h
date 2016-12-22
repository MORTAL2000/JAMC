#pragma once

#include "glm\glm.hpp"

#include "Client.h"
#include "PageComponentLoader.h"
#include "BorderImageComp.h"

class TextButtonComp : public PageComponentLoader {
public:
	struct ButtonData {
		PCFunc func_action;

		glm::vec4 color_down;
		glm::vec4 color_up;

		PComp * comp_border;
		PComp * comp_label;

		BorderImageComp::BorderImageData * data_border;
	};

	TextButtonComp( Client & client );
	~TextButtonComp( );
};

