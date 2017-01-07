#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class RootComp : public PageComponentLoader {
public:
	RootComp( Client & client );
	~RootComp( );
};

