#include "ClickableComp.h"


ClickableComp::ClickableComp( Client & client ) { 
	name = "Clickable";

	func_register = [ &client = client ] ( ) { 
		if( !client.resource_mgr.reg_pool< ClickableData >( num_comp_default ) ) {
			return 1;
		}

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) { 
		if( !comp->add_data< ClickableData >( ) ) { 
			return 1;
		}

		auto data = comp->get_data< ClickableData >( );
		data->func_down = func_null;
		data->func_hold = func_null;
		data->func_up = func_null;

		return 0; 
	};

	func_update = [ &client = client ] ( PComp * comp ) { 
		comp->dim = comp->parent->dim;

		return 0;
	};

	func_is_down = [ ] ( PComp * comp ) { 
		return 1; 
	};

	func_down = [ ] ( PComp * comp ) {
		return comp->get_data< ClickableData >( )->func_down( comp->parent ); 
	};

	func_hold = [ ] ( PComp * comp ) {
		return comp->get_data< ClickableData >( )->func_hold( comp->parent );
	};

	func_up = [ ] ( PComp * comp ) {
		return comp->get_data< ClickableData >( )->func_up( comp->parent );
	};
}


ClickableComp::~ClickableComp( ) { }
