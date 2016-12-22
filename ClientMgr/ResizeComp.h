#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class ResizeComp : public PageComponentLoader {
public:
	ResizeComp( Client & client );
	~ResizeComp( );
};

