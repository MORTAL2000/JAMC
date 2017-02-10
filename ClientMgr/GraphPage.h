#pragma once

#include "Client.h"
#include "PageLoader.h"
#include "MenuComp.h"
#include "GraphComp.h"

class GraphPage : public PageLoader {
public:
	struct GraphPageData { 
		PComp * comp_graph;
		GraphComp::GraphData * data_graph;

		PComp * comp_menu;
		MenuComp::MenuData * data_menu;
	};

	GraphPage( Client & client );
	~GraphPage( );
};

