#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "ImageComp.h"

class CheckboxComp : public PageComponentLoader {
public:
	struct CheckboxData { 
		bool is_checked;

		int unsigned id_texture;
		int unsigned id_subtex_checked;
		int unsigned id_subtex_unchecked;

		PCFunc func_checked;
		PCFunc func_unchecked;

		PComp * comp_border;
		PComp * comp_image;
		ImageComp::ImageData * data_image;
	};

	CheckboxComp( Client & client );
	~CheckboxComp( );
};

