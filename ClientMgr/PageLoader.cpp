#include "PageLoader.h"

#include "Page.h"

PageRegFunc PageLoader::regfunc_null = [ ] ( ) {

	return 0;
};

PageFunc PageLoader::func_null = [ ] ( Page * page ) {

	return 0;
};

PageLoader::PageLoader( ) : 
	func_register( regfunc_null ),
	func_alloc( func_null ),
	func_release( func_null ),
	func_update( func_null ),
	func_mesh( func_null ) { }


PageLoader::~PageLoader( ) { }
