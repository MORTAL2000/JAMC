#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

#include "BorderImageComp.h"
#include "ImageComp.h"

class ImageButtonComp : public PageComponentLoader {
public:
	struct ImageButtonData { 
		PCFunc func_action;

		int padding_image;

		glm::vec4 color_default;
		glm::vec4 color_over;
		glm::vec4 color_down;
		
		PComp * comp_border;
		BorderImageComp::BorderImageData * data_border;

		PComp * comp_image;
		ImageComp::ImageData * data_image;

	};

	ImageButtonComp( Client & client );
	~ImageButtonComp( );
};

