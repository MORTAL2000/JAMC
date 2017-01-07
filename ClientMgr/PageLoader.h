#pragma once

#include <functional>

class Page;

using PageFunc = std::function< int( Page * page ) >;

class PageLoader {
private:

public:
	static PageFunc func_null;

	PageLoader( );
	~PageLoader( );

	std::string name;

	PageFunc func_alloc;
	PageFunc func_release;
	PageFunc func_update;
	PageFunc func_mesh;

private:

public:

};
