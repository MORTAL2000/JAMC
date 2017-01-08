#pragma once

#include "Client.h"
#include "PageLoader.h"
#include "MenuComp.h"

class GraphPage : public PageLoader {
public:
	struct GraphPageData { 
		PComp * comp_menu;
		MenuComp::MenuData * data_menu;
	};

	GraphPage( Client & client );
	~GraphPage( );
};

