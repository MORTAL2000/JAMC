#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "BorderImageComp.h"

#include <vector>

class MenuComp : public PageComponentLoader {
public:	
	struct MenuData {
		int id_entry;

		glm::ivec2 dim_entry;

		glm::ivec2 padding_entry;
		glm::ivec2 padding_menu;

		glm::vec4 color_default;
		glm::vec4 color_over;
		glm::vec4 color_down;

		std::vector< PComp * > list_entries;

		PComp * comp;

		PComp * comp_border;
		BorderImageComp::BorderImageData * data_border;

		void add_entry( Client & client, std::string const & label_text, PCFunc func_up );
		void reposition( );
	};

	MenuComp( Client & client );
	~MenuComp( );
};

