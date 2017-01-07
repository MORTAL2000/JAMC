#pragma once

#include "Client.h"
#include "PageLoader.h"

class GraphPage : public PageLoader {
public:
	GraphPage( Client & client );
	~GraphPage( );
};

