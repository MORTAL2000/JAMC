#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "LabelComp.h"

class GraphComp : public PageComponentLoader {
public:
	struct GraphData { 
		TimeRecord & record;

		PComp * comp_label_title;
		LabelComp::LabelData * data_label_title;
	};

	GraphComp( Client & client );
	~GraphComp( );
};

