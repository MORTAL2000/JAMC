#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "BorderImageComp.h"
#include "ImageComp.h"
#include "LabelComp.h"

class CheckboxComp : public PageComponentLoader {
public:
	struct CheckboxData { 
		bool is_checked;

		glm::vec4 color_default;
		glm::vec4 color_down;
		glm::vec4 color_over;

		int unsigned id_texture;
		int unsigned id_subtex_checked;
		int unsigned id_subtex_unchecked;

		PCFunc func_checked;
		PCFunc func_unchecked;

		PComp * comp_border;
		PComp * comp_image;
		PComp * comp_label;

		BorderImageComp::BorderImageData * data_border;
		ImageComp::ImageData * data_image;
		LabelComp::LabelData * data_label;

		void set_checked( bool is_checked ) { 
			this->is_checked = is_checked;

			if( is_checked ) {
				data_image->id_subtex = id_subtex_checked;
			}
			else { 
				data_image->id_subtex = id_subtex_unchecked;
			}
		}
	};

	CheckboxComp( Client & client );
	~CheckboxComp( );
};

