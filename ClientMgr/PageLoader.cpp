#include "PageLoader.h"

#include "Page.h"

PageFunc PageLoader::func_null = [ ] ( Page * page ) {
	return 0;
};

PageLoader::PageLoader( ) : 
	func_alloc( func_null ),
	func_release( func_null ),
	func_update( func_null ),
	func_mesh( func_null ) { }


PageLoader::~PageLoader( ) { }
