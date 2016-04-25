#include "Page.h"

#include "Client.h"

Page::Page( ) :
	client( nullptr ),
	vec_pos( 0, 0 ),
	vec_dim( 0, 0 ),
	is_hold( false ),
	is_edit( false ),
	is_visibile( true ) {
}


Page::~Page( ) { }

void Page::add_comp( std::string & str_name,
	FuncComp func_alloc, FuncComp func_custom ) { 
	Handle< PageComp > handle_comp;

	if( client->resource_mgr.allocate( handle_comp ) ) { 
		auto & comp = handle_comp.get();
		comp.parent = this;
		comp.client = client;
		comp.str_name = str_name;

		if( func_alloc( comp ) &&
			func_custom( comp ) ) {
			list_comps.push_back( handle_comp );
			map_comps.insert( { str_name, &comp } );
		}
	}
}

PageComp & Page::get_comp( std::string & str_name ) { 
	return *map_comps[ str_name ];
}

bool Page::on_down( int button ) {
	auto pos_mouse = client->input_mgr.get_mouse_down( button );
	glm::ivec2 pos_local =  glm::ivec2( pos_mouse.x, pos_mouse.y ) - vec_pos;
	bool is_handled = false;
	int i = list_comps.size( ) - 1;

	while( !is_handled && i >= 0 ) { 
		auto & comp = list_comps[ i ].get();

		if( comp.is_visible ) {
			if( Directional::is_point_in_rect( pos_local, comp.vec_pos, comp.vec_pos + comp.vec_dim ) ) {
				if( comp.func_on_down ) {
					is_handled = comp.func_on_down( comp );
				}
			}
		}

		i--;
	}

	return true;
}

bool Page::on_hold( int button ) { 
	bool is_handled = false;
	int i = list_comps.size( ) - 1;

	while( !is_handled && i >= 0 ) {
		auto & comp = list_comps[ i ].get( );

		if( comp.is_visible ) {
			if( comp.func_on_hold ) {
				is_handled = comp.func_on_hold( comp );
			}
		}

		i--;
	}

	return false;
}

bool Page::on_up( int  button ) {
	bool is_handled = false;
	int i = list_comps.size( ) - 1;

	while( !is_handled && i >= 0 ) {
		auto & comp = list_comps[ i ].get( );

		if( comp.is_visible ) {
			if( comp.func_on_up ) {
				is_handled = comp.func_on_up( comp );
			}
		}

		i--;
	}

	return false;
}

void Page::update( ) { 
	for( int i = 0; i < list_comps.size( ); i++ ) {
		auto & comp = list_comps[ i ].get( );

		if( comp.func_update ) {
			comp.func_update( comp );
		}
	}
}

void Page::resize( ) { 
	for( int i = 0; i < list_comps.size( ); i++ ) { 
		list_comps[ i ].get( ).position( );
	}
}