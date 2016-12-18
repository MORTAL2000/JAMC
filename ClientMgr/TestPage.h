#pragma once

#include "PageLoader.h"
#include "Client.h"

class TestPage : public PageLoader {
public:
	struct TestData { 
		glm::vec4 color;
	};

	TestPage( Client & client  );
	~TestPage( );
};

