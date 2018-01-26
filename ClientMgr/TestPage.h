#pragma once

#include "Client.h"
#include "PageLoader.h"

class TestPage : public PageLoader {
public:
	struct TestData { 
		glm::vec4 color;
	};

	TestPage( Client & client  );
	~TestPage( );
};

