#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class ImageComp : public PageComponentLoader {
public:
	struct ImageData { 
		glm::vec4 color;

		int unsigned id_texture;
		int unsigned id_subtex;

		PCFunc func_resize;
	};

	ImageComp( Client & client );
	~ImageComp( );
};

