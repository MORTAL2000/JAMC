#include "PageLoader.h"

#include "Page.h"

PageFunc PageLoader::func_null = [ ] ( Page * page ) {
	return 0;
};

PageFunc PageLoader::func_update_comps = [ ] ( Page * page ) { 
	auto iter_comp = page->list_comps.begin( );

	while( iter_comp != page->list_comps.end( ) ) {
		PComp * comp = &iter_comp->get( );
		comp->reposition( );
		iter_comp++;
	}

	return 0;
};

PageFunc PageLoader::func_mesh_comps = [ ] ( Page * page ) { 
	auto iter_comp = page->list_comps.begin( );

	while( iter_comp != page->list_comps.end( ) ) {
		PComp * comp = &iter_comp->get( );
		comp->pc_loader->func_mesh( comp );
		iter_comp++;
	}

	return 0;
};

PageLoader::PageLoader( ) : 
	func_alloc( func_null ),
	func_release( func_null ),
	func_update( func_null ),
	func_mesh( func_null ) { }


PageLoader::~PageLoader( ) { }
