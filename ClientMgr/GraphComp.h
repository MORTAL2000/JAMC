#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "LabelComp.h"

class GraphComp : public PageComponentLoader {
public:
	struct GraphData {
		TimeRecord * record;
		int num_entries;
		int unsigned updates;

		glm::ivec2 padding;

		int id_texture_line;
		int id_subtex_line;

		PComp * comp_label_title;
		LabelComp::LabelData * data_label_title;
	};

	GraphComp( Client & client );
	~GraphComp( );
};

