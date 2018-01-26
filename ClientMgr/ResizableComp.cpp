#include "ResizableComp.h"

#include "ResourceMgr.h"

#include "Page.h"

ResizableComp::ResizableComp( Client & client ) { 
	name = "Resizable";

	func_register = [ &client = client ] () { 
		if( !client.resource_mgr.reg_pool< ResizableData >( num_comp_default ) ) {
			return 1;
		}

		return 0; 
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		auto data = comp->add_data< ResizableData >( );

		if( !data ) { 
			return 1;
		};

		data->func_resize = func_null;

		return 0; 
	};

	func_update = [ &client = client ] ( PComp * comp ) {
		comp->dim = comp->parent->dim;
		auto data = comp->get< ResizableData >( );
		auto iter_children = comp->list_comps.begin( );

		while( iter_children != comp->list_comps.end( ) ) { 
			data->func_resize( &iter_children->get( ) );
			++iter_children;
		}
		
		return 0;
	};
}


ResizableComp::~ResizableComp( ) { }