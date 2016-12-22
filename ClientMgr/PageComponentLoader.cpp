#include "PageComponentLoader.h"

PCRegFunc PageComponentLoader::func_reg_null = [ ] ( ) {
	return 0;
};

PCFunc PageComponentLoader::func_null = [ ] ( PComp * comp ) { 
	return 0;
};

PageComponentLoader::PageComponentLoader( ) :
	func_register( func_reg_null ),
	func_alloc( func_null ),
	func_release( func_null ),
	func_update( func_null ),
	func_mesh( func_null ),
	func_enter( func_null ),
	func_over( func_null ),
	func_exit( func_null ),
	func_down( func_null ),
	func_hold( func_null ),
	func_up( func_null ),
	func_action( func_null ) { }


PageComponentLoader::~PageComponentLoader( ) { }
