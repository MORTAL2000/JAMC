#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class ResizableComp : public PageComponentLoader {
public:
	struct ResizableData { 
		PCFunc func_resize;
	};

	ResizableComp( Client & client );
	~ResizableComp( );
};

