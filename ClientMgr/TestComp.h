#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class TestComp : public PageComponentLoader {
public:
	struct CompData { 
		glm::vec4 color;
	};

	TestComp( Client & client );
	~TestComp( );
};

