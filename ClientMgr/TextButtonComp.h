#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "ContainerComp.h"
#include "ClickableComp.h"
#include "OverableComp.h"
#include "BorderImageComp.h"
#include "LabelComp.h"

#include "glm\glm.hpp"

class TextButtonComp : public PageComponentLoader {
public:
	struct TextButtonData {
		PCFunc func_action;

		glm::vec4 color_default;
		glm::vec4 color_over;
		glm::vec4 color_down;


		PComp * comp_clickable;
		ClickableComp::ClickableData * data_clickable;

		PComp * comp_overable;
		OverableComp::OverableData * data_overable;

		PComp * comp_border;
		BorderImageComp::BorderImageData * data_border;

		PComp * comp_label;
		LabelComp::LabelData * data_label;
	};
	  
	TextButtonComp( Client & client );
	~TextButtonComp( );
};

