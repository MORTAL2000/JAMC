#include "PageComponentLoader.h"

PCFunc PageComponentLoader::func_null = [ ] ( PComp * comp ) { 
	return 0;
};

PageComponentLoader::PageComponentLoader( ) :
	func_alloc( func_null ), 
	func_release( func_null ),
	func_update( func_null ),
	func_mesh( func_null ),
	func_over( func_null ),
	func_down( func_null ),
	func_hold( func_null ),
	func_up( func_null ),
	func_action( func_null ) { }


PageComponentLoader::~PageComponentLoader( ) { }
