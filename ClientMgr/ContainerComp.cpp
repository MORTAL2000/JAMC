#include "ContainerComp.h"

#include "ResourceMgr.h"

#include "PageComponent.h"

ContainerComp::ContainerComp( Client & client ) { 
	name = "Container";

	func_register = [ &client = client ] ( ) { 
		if( !client.resource_mgr.reg_pool< ContainerData >( num_comp_default ) ) {
			return 1;
		}

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) { 
		auto data = comp->add_data< ContainerData >( );
		data->func_reposition = func_null;
		data->func_resize = [ ] ( PComp * comp ) { 
			comp->dim = comp->parent->dim;

			return 0; 
		};

		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get< ContainerData >( );
		data->func_resize( comp );
		data->func_reposition( comp );

		return 0;
	};
}


ContainerComp::~ContainerComp( ) { }
