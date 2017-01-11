#pragma once

#include <functional>

class Page;

using PageRegFunc = std::function< int( ) >;
using PageFunc = std::function< int( Page * page ) >;

class PageLoader {
private:

public:
	static PageRegFunc regfunc_null;
	static PageFunc func_null;

	PageLoader( );
	~PageLoader( );

	std::string name;

	PageRegFunc func_register;

	PageFunc func_alloc;
	PageFunc func_release;
	PageFunc func_update;
	PageFunc func_mesh;

private:

public:

};

