#include "Page.h"

#include "Client.h"
#include "Directional.h"

const glm::vec3 verts_page[ 4 ] = { 
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 } 
};

Page::Page( ) { }

Page::~Page( ) { }

PComp * Page::add_comp( std::string const & name_comp, std::string const & name_loader, PCFunc func_custom ) { 
	if( map_comps.find( name_comp ) != map_comps.end( ) ) { 
		printf( "ERROR: Duplicate Component Name: %s\n", name_comp.c_str( ) );
		return nullptr;
	}
	

	PCLoader * loader = client->gui_mgr.get_component_loader_safe( name_loader );

	if( loader == nullptr ) { 
		printf( "ERROR: Cannot Find Loader: %s, for Component: %s\n", name_loader.c_str( ), name_comp.c_str( ) );
		return nullptr;
	}

	Handle< PComp > handle_comp;
	PComp * comp = client->resource_mgr.allocate( handle_comp );

	if( comp == nullptr ) {
		printf( "ERROR: Failed to allocate Component: %s\n", name_comp.c_str( ) );
		return nullptr;
	}

	comp->page = this;
	comp->parent = nullptr;
	comp->pc_loader = loader;

	comp->name = name_comp;

	if( loader->func_alloc( comp ) != 0 ) { 
		printf( "ERROR: Failed to allocate Component: %s\n", name_comp.c_str( ) );
		client->resource_mgr.release( handle_comp );
		return false;
	}
	
	func_custom( comp );

	comp->reposition( );

	list_comps.push_back( handle_comp );
	map_comps.insert( { name_comp, ( int ) list_comps.size( ) - 1 } );

	printf( "SUCCESS: Added Component: %s to Page: %s\n", name_comp.c_str( ), name.c_str( ) );
	return comp;
}

PComp * Page::get_comp( std::string const & name ) {
	return &list_comps[ map_comps[ name ] ].get( );
}

PComp * Page::get_comp_safe( std::string const & name ) {
	auto iter_map = map_comps.find( name );
	if( iter_map == map_comps.end( ) ) { 
		return nullptr;
	}

	return &list_comps[ map_comps[ name ] ].get( );
}

bool Page::on_down( int button ) {
	bool is_handled = false;
	glm::ivec2 pos_l = client->input_mgr.get_mouse_down( button ) - pos;

	auto iter_comps = list_comps.begin( );
	PComp * comp;

	while( iter_comps != list_comps.end( ) ) { 
		comp = &iter_comps->get( );

		if( comp->is_visible &&
			Directional::is_point_in_rect( pos_l, comp->pos, comp->pos + comp->dim ) ) {

			is_handled = comp->pc_loader->func_down( comp );

			if( is_handled ) { 
				break;
			}
		}

		++iter_comps;
	}

	if( !is_handled ) {
		is_hold = true;
		is_handled = true;
	}

	return is_handled;
}

bool Page::on_hold( int button ) {
	bool is_handled = false;

	auto iter_comps = list_comps.begin( );
	PComp * comp;

	while( iter_comps != list_comps.end( ) ) {
		comp = &iter_comps->get( );

		is_handled = comp->pc_loader->func_hold( comp );

		if( is_handled ) {
			break;
		}

		++iter_comps;
	}

	if( !is_handled && is_hold ) {
		offset += client->input_mgr.get_mouse_delta( );

		return true;
	}

	return false;
}

bool Page::on_up( int button ) {
	bool is_handled = false;
	glm::ivec2 pos_l = client->input_mgr.get_mouse_up( button ) - pos;


	PComp * comp;
	auto iter_comps = list_comps.begin( );

	while( iter_comps != list_comps.end( ) ) {
		comp = &iter_comps->get( );

		if( comp->is_visible &&
			Directional::is_point_in_rect( pos_l, comp->pos, comp->pos + comp->dim ) ) {

			is_handled = comp->pc_loader->func_up( comp );

			if( is_handled ) {
				break;
			}
		}

		++iter_comps;
	}

	if( !is_handled ) {
		is_hold = false;
		is_handled = true;
	}

	return is_handled;
}

void Page::reposition( ) { 
	pos.x = client->display_mgr.get_window( ).x * anchor.x + offset.x;
	pos.y = client->display_mgr.get_window( ).y * anchor.y + offset.y;
}