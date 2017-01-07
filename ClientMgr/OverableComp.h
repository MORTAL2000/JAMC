#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class OverableComp : public PageComponentLoader {
public:
	struct OverableData {
		PCFunc func_enter;
		PCFunc func_over;
		PCFunc func_exit;
	};

	OverableComp( Client & client );
	~OverableComp( );
};

