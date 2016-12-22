#pragma once

#include <functional>

class PageComponent;

using PComp = PageComponent;
using PCRegFunc = std::function< int( ) >;
using PCFunc = std::function< int( PComp * ) >;

class PageComponentLoader {
private:

public:
	static PCRegFunc func_reg_null;
	static PCFunc func_null;

	PageComponentLoader( );
	~PageComponentLoader( );

	PCRegFunc func_register;

	std::string name;

	PCFunc func_alloc;
	PCFunc func_release;
	PCFunc func_update;

	PCFunc func_enter;
	PCFunc func_over;
	PCFunc func_exit;

	PCFunc func_down;
	PCFunc func_hold;
	PCFunc func_up;
	 
	PCFunc func_mesh;

	PCFunc func_action;

private:

public:

};

using PCLoader = PageComponentLoader;