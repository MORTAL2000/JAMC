#pragma once

#include <functional>

class PageComponent;

using PComp = PageComponent;
using PCFunc = std::function< int( PComp * ) >;

class PageComponentLoader {
private:

public:
	static PCFunc func_null;

	PageComponentLoader( );
	~PageComponentLoader( );

	std::string name;

	PCFunc func_alloc;
	PCFunc func_release;
	PCFunc func_update;
	PCFunc func_mesh;

	PCFunc func_over;
	PCFunc func_down;
	PCFunc func_hold;
	PCFunc func_up;
	 
	PCFunc func_action;

private:

public:

};

using PCLoader = PageComponentLoader;