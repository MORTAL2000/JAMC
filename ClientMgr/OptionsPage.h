#pragma once

#include "Client.h"
#include "PageLoader.h"

class OptionsPage : public PageLoader {
public:
	OptionsPage( Client & client );
	~OptionsPage( );
};

