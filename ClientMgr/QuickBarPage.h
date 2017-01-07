#pragma once

#include "Client.h"
#include "PageLoader.h"

class QuickBarPage : public PageLoader {
public:
	struct QuickBarData { 
		std::vector< PComp * > list_icons;
	};

	QuickBarPage( Client & client );
	~QuickBarPage( );
};

