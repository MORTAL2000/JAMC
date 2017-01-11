#include "OverableComp.h"

OverableComp::OverableComp( Client & client ) {
	name = "Overable";

	func_register = [ &client = client ] ( ) {
		if( !client.resource_mgr.reg_pool< OverableData >( num_comp_default ) ) {
			return 1;
		}

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		if( !comp->add_data< OverableData >( ) ) {
			return 1;
		}

		auto data = comp->get_data< OverableData >( );
		data->func_enter = func_null;
		data->func_over = func_null;
		data->func_exit = func_null;

		return 0;
	};

	func_update = [ ] ( PComp * comp ) {
		comp->dim = comp->parent->dim;

		return 0;
	};

	func_is_over = [ ] ( PComp * comp ) {
		return 1;
	};

	func_enter = [ ] ( PComp * comp ) {
		return comp->get_data< OverableData >( )->func_enter( comp );
	};

	func_over = [ ] ( PComp * comp ) {
		return comp->get_data< OverableData >( )->func_over( comp );
	};

	func_exit = [ ] ( PComp * comp ) {
		return comp->get_data< OverableData >( )->func_exit( comp );
	};
}


OverableComp::~OverableComp( ) { }
