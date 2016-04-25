#pragma once

#include "Globals.h"

#include <functional>

typedef std::function< bool( Page & page ) > FuncPage;

class PageFuncs {
public:
	static FuncPage cust_null;

	static FuncPage alloc_base;
	static FuncPage alloc_test;

	static FuncPage alloc_console;

	static FuncPage alloc_static;
};