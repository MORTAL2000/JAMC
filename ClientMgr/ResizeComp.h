#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "ImageComp.h"

class ResizeComp : public PageComponentLoader {
public:
	struct ResizeData {
		glm::vec4 color_exit;
		glm::vec4 color_enter;

		PComp * comp_image;
		ImageComp::ImageData * data_image;
	};

	ResizeComp( Client & client );
	~ResizeComp( );
};

