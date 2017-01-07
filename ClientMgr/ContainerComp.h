#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class ContainerComp : public PageComponentLoader {
public:
	struct ContainerData { 
		PCFunc func_resize;
		PCFunc func_reposition;
	};

	ContainerComp( Client & client );
	~ContainerComp( );
};

