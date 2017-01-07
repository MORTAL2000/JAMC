#include "PageComponent.h"

#include "Page.h"
#include "Client.h"

PageComponent::PageComponent( ) { }


PageComponent::~PageComponent( ) { }


PComp * PageComponent::add_comp( std::string const & name_comp, std::string const & name_loader, PCFunc func_custom ) {
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

	comp->name = name_comp;

	comp->is_visible = true;
	comp->is_hold = false;

	comp->client = client;
	comp->page = page;
	comp->parent = this;
	comp->pc_loader = loader;

	comp->list_comps.clear( );
	comp->map_comps.clear( );

	if( loader->func_alloc( comp ) != 0 ) {
		printf( "ERROR: Failed to allocate Component: %s\n", name_comp.c_str( ) );
		client->resource_mgr.release( handle_comp );
		return nullptr;
	}

	func_custom( comp );

	comp->reposition( );

	list_comps.push_back( handle_comp );
	map_comps.insert( { name_comp, ( int ) list_comps.size( ) - 1 } );

	printf( "SUCCESS: Added Component: %s to Component: %s\n", name_comp.c_str( ), name.c_str( ) );

	return comp;
}

PComp * PageComponent::get_comp( std::string const & name ) {
	return &list_comps[ map_comps[ name ] ].get( );
}

PComp * PageComponent::get_comp_safe( std::string const & name ) {
	auto iter_map = map_comps.find( name );
	if( iter_map == map_comps.end( ) ) {
		return nullptr;
	}

	return &list_comps[ map_comps[ name ] ].get( );
}

void PageComponent::reposition( ) {
	pos.x = parent->pos.x + parent->dim.x * anchor.x + offset.x;
	pos.y = parent->pos.y + parent->dim.y * anchor.y + offset.y;
}