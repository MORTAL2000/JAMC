#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class ClickableComp : public PageComponentLoader {
public:
	struct ClickableData { 
		PCFunc func_down;
		PCFunc func_hold;
		PCFunc func_up;
	};

	ClickableComp( Client & client );
	~ClickableComp( );
};

