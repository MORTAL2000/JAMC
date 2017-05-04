#include "GuiMgr.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Client.h"
#include "WorldSize.h"

#include "TestPage.h"
#include "OptionsPage.h"
#include "QuickBarPage.h"
#include "GraphPage.h"
#include "ConsolePage.h"

#include "RootComp.h"
#include "ContainerComp.h"

#include "ResizableComp.h"
#include "ClickableComp.h"
#include "OverableComp.h"

#include "ImageComp.h"
#include "LabelComp.h"
#include "ResizeComp.h"

#include "BorderImageComp.h"

#include "TextButtonComp.h"
#include "CheckboxComp.h"
#include "SliderComp.h"
#include "SliderVComp.h"
#include "MenuComp.h"
#include "TextFieldComp.h"

#include "GraphComp.h"


GuiMgr::GuiMgr( Client & client ) :
	Manager( client ),
	block_selector( client ) { }

GuiMgr::~GuiMgr( ) { }

void GuiMgr::init( ) {
	printf( "\n*** Gui Manager ***\n" );
	map_pages.reserve( size_pages );
	client.resource_mgr.reg_pool< Page >( 128 );
	client.resource_mgr.reg_pool< PComp >( PageComponentLoader::num_comp_default );
	//list_order.reserve( size_pages );

	GL_CHECK( block_selector.init( ) );

	printf( "\nLoading Components...\n\n" );
	load_components( );

	printf( "\nLoading Pages...\n\n" );
	load_pages( );

	is_visible = true;
}

void GuiMgr::load_components( ) {
	add_component_loader( RootComp( client ) );
	add_component_loader( ContainerComp( client ) );

	add_component_loader( ResizableComp( client ) );
	add_component_loader( ClickableComp( client ) );
	add_component_loader( OverableComp( client ) );

	add_component_loader( LabelComp( client ) );
	add_component_loader( ImageComp( client ) );

	add_component_loader( BorderImageComp( client ) );

	add_component_loader( TextButtonComp( client ) );
	add_component_loader( CheckboxComp( client ) );
	add_component_loader( ResizeComp( client ) );
	add_component_loader( SliderComp( client ) );
	add_component_loader( SliderVComp( client ) );
	add_component_loader( MenuComp( client ) );
	add_component_loader( TextFieldComp( client ) );

	add_component_loader( GraphComp( client ) );
}

void GuiMgr::load_pages( ) {
	add_page_loader( TestPage( client ) );
	add_page_loader( OptionsPage( client ) );
	add_page_loader( QuickBarPage( client ) );
	add_page_loader( GraphPage( client ) );
	add_page_loader( ConsolePage( client ) ); 

	//add_page( "TestPage", "Test", PageLoader::func_null );

	add_page( "Options", "Options", PageLoader::func_null );
	add_page( "QuickBar", "QuickBar", PageLoader::func_null );
	add_page( "Console", "Console", PageLoader::func_null );

	int cnt = 1;
	for( auto str_record : { RecordStrings::SLEEP, RecordStrings::FRAME, 
		RecordStrings::TASK_MAIN, RecordStrings::UPDATE, RecordStrings::RENDER, RecordStrings::RENDER_SWAP } ) { 
		add_page( "Graph " + str_record, "Graph", [ & ] ( Page * page ) { 
			page->is_visible = true;
			page->root->anchor = { 1.0f, 1.0f };
			page->root->offset = { -( page->root->dim.x + 2 ), -( page->root->dim.y + 2 ) * cnt };
			cnt++;

			auto data = page->get_data< GraphPage::GraphPageData >( );
			data->data_graph->record = &client.time_mgr.get_record( str_record );
			data->data_graph->data_label_title->text = str_record;

			return 1;
		} );
	}
}

void GuiMgr::update( ) {
	block_selector.update( );

	comp_over = nullptr;

	Page * page;

	// Update/remesh
	auto iter_order = list_order.begin( );
	while( iter_order != list_order.end( ) ) { 
		page = &iter_order->get( );
		update_page( page );
		mesh_page( page );
		++iter_order;
	}

	if( is_visible && client.input_mgr.is_cursor_vis( ) ) {
		on_click( );
		on_over( );
	}

	process_remove( );
}

void GuiMgr::update_page( Page * page ) {
	page->loader->func_update( page );
	page->reposition( );

	update_comp_children( page->root );
}

void GuiMgr::update_comp( PComp * comp ) {
	update_comp_self( comp );
	update_comp_children( comp );
}

void GuiMgr::update_comp_self( PComp * comp ) {
	comp->pc_loader->func_update( comp );
	comp->reposition( );
}

void GuiMgr::update_comp_children( PComp * comp ) {
	auto iter_comp = comp->list_comps.begin( );

	while( iter_comp != comp->list_comps.end( ) ) { 
		update_comp( &iter_comp->get( ) );

		++iter_comp;
	}
}

void GuiMgr::on_over( ) {
	comp_over = nullptr;

	over_page_all( );

	if( comp_over_last ) { 
		if( comp_over_last == comp_over ) { 
			comp_over_last->pc_loader->func_over( comp_over_last );
		}
		else if( comp_over ) { 
			comp_over_last->pc_loader->func_exit( comp_over_last );
			comp_over_last = comp_over;
			comp_over_last->pc_loader->func_enter( comp_over_last );
		}
		else { 
			comp_over_last->pc_loader->func_exit( comp_over_last );
			comp_over_last = nullptr;
		}
	}
	else if( comp_over ) { 
		comp_over_last = comp_over;
		comp_over_last->pc_loader->func_enter( comp_over_last );
	}
}

void  GuiMgr::over_page_all( ) {
	auto iter_order = list_order.begin( );
	while( !comp_over && iter_order != list_order.end( ) ) { 
		over_page( &iter_order->get( ) );

		++iter_order;
	}
}

void GuiMgr::over_page( Page * page ) {
	if( !page->is_visible ) { 
		return;
	}

	over_page_children( page );
}

void GuiMgr::over_page_self( Page * page ) { 
	if( comp_over ) {
		return;
	}
}

void GuiMgr::over_page_children( Page * page ) { 
	if( comp_over ) { 
		return;
	}

	over_comp( page->root );
}

void GuiMgr::over_comp( PComp * comp ) {
	if( !comp->is_visible ) { 
		return;
	}

	over_comp_children( comp );
	over_comp_self( comp );
}

void GuiMgr::over_comp_self( PComp * comp ) {
	if( comp_over ) { 
		return;
	}

	if( Directional::is_point_in_rect(
			client.input_mgr.get_mouse( ),
			comp->page->pos + comp->pos,
			comp->page->pos + comp->pos + comp->dim ) &&
		comp->pc_loader->func_is_over( comp ) != 0 ) {

		comp_over = comp;
	}
}

void GuiMgr::over_comp_children( PComp * comp ) {
	if( comp_over ) { 
		return;
	}

	auto iter_comp = comp->list_comps.rbegin( );

	while( !comp_over && iter_comp != comp->list_comps.rend( ) ) {
		over_comp( &iter_comp->get( ) );

		++iter_comp;
	}
}

void GuiMgr::on_click( ) {
	if( client.input_mgr.is_mouse_down( 0 ) || client.input_mgr.is_mouse_down( 1 ) ) {
		comp_down = nullptr;
		down_page_all( );

		if( comp_down ) {
			comp_down->pc_loader->func_down( comp_down );

			if( comp_down->parent && 
				comp_down->parent->parent && 
				comp_down->parent->parent->parent &&
				comp_down->parent->parent->parent->pc_loader->name == "TextField" ) { 
				comp_selected = comp_down;
				is_input = true;
				std::cout << "Selected Text Field" << std::endl;
			}

			return;
		}

		comp_selected = nullptr;
		is_input = false;

		return;
	}

	if( client.input_mgr.get_mouse_hold( 0 ) || client.input_mgr.get_mouse_hold( 1 ) ) { 
		if( comp_down ) {
			comp_down->pc_loader->func_hold( comp_down );
			
			return;
		}

		return;
	}

	if( client.input_mgr.is_mouse_up( 0 ) || client.input_mgr.is_mouse_up( 1 ) ) { 
		if( comp_down ) {
			comp_down->pc_loader->func_up( comp_down );

			return;
		}

		return;
	}
}

void  GuiMgr::down_page_all( ) {
	auto iter_order = list_order.begin( );
	while( !comp_down && iter_order != list_order.end( ) ) {
		down_page( &iter_order->get( ) );

		++iter_order;
	}

	if( comp_down ) { 
		--iter_order;
		auto temp = *iter_order;
		list_order.erase( iter_order );
		list_order.push_front( temp );
	}
}

void GuiMgr::down_page( Page * page ) {
	if( !page->is_visible ) {
		return;
	}

	down_page_children( page );
}

void GuiMgr::down_page_self( Page * page ) {
	if( comp_down ) {
		return;
	}
}

void GuiMgr::down_page_children( Page * page ) {
	if( comp_down ) {
		return;
	}

	down_comp( page->root );
}

void GuiMgr::down_comp( PComp * comp ) {
	if( !comp->is_visible ) {
		return;
	}

	down_comp_children( comp );
	down_comp_self( comp );
}

void GuiMgr::down_comp_self( PComp * comp ) {
	if( comp_down ) {
		return;
	}

	if( Directional::is_point_in_rect(
			client.input_mgr.get_mouse( ),
			comp->page->pos + comp->pos,
			comp->page->pos + comp->pos + comp->dim ) &&
		comp->pc_loader->func_is_down( comp ) ) {

		comp_down = comp;
	}
}

void GuiMgr::down_comp_children( PComp * comp ) {
	if( comp_down ) {
		return;
	}

	PComp * comp_child;
	auto iter_comp = comp->list_comps.rbegin( );

	while( !comp_down && iter_comp != comp->list_comps.rend( ) ) {
		comp_child = &iter_comp->get( );
		down_comp( comp_child );

		++iter_comp;
	}
}

void GuiMgr::remove_page( std::string const & name_page ) {
	queue_remove.push( name_page );
}

void GuiMgr::remove_comp( PComp * comp ) { 
	remove_comp_children( comp );
	remove_comp_self( comp );
}

void GuiMgr::remove_comp_self( PComp * comp ) { 
	comp->clear_data( );
	comp->page = nullptr;
	comp->parent = nullptr;
	comp->pc_loader = nullptr;
}

void GuiMgr::remove_comp_children( PComp * comp ) { 
	auto iter_children = comp->list_comps.rbegin( );
	while( iter_children != comp->list_comps.rend( ) ) { 
		remove_comp( &iter_children->get( ) );
		iter_children->release( );
		++iter_children;
	}

	comp->list_comps.clear( );
	comp->list_comps.shrink_to_fit( );
	comp->map_comps.clear( );
}

PComp * GuiMgr::get_named_parent( PComp * comp, std::string const & name ) {
	while( comp != nullptr ) { 
		if( comp->pc_loader->name == name ) { 
			return comp;
		}

		comp = comp->parent;
	}

	return nullptr;
}

void GuiMgr::mesh_page( Page * page ) {
	if( page->is_visible && page->is_remesh ) {
		//client.thread_mgr.task_main( 10, [ this, page ] ( ) {
			page->vbo_mesh.clear( );

			page->loader->func_mesh( page );

			mesh_comp( page->root );

			page->vbo_mesh.buffer( );
		//} );

		page->is_remesh = false;
	}
}

void GuiMgr::mesh_comp( PComp * comp ) { 
	if( !comp->is_visible ) { 
		return;
	}

	comp->pc_loader->func_mesh( comp );

	auto iter_child = comp->list_comps.begin( );
	while( iter_child != comp->list_comps.end( ) ) {
		mesh_comp( &iter_child->get( ) );
		++iter_child;
	}
}

void GuiMgr::render( ) {
	block_selector.render( );

	if( !is_visible ) {
		return;
	}

	auto iter_order = list_order.rbegin( );

	while( iter_order != list_order.rend( ) ) {
		auto page = &iter_order->get( );

		if( !page->is_visible ) {
			++iter_order;
			continue;
		}

		client.texture_mgr.update_uniform( "BasicOrtho", "mat_model", page->mat_model );
		page->vbo_mesh.render( client, true );

		++iter_order;
	}
}

void GuiMgr::end( ) { }

void GuiMgr::sec( ) { }

void GuiMgr::add_component_loader( PCLoader & pc_loader ) { 
	if( map_page_component_loaders.find( pc_loader.name ) != map_page_component_loaders.end( ) ) { 
		printf( "ERROR: Duplicate Page Component Loader: %s\n", pc_loader.name.c_str( ) );
		return;
	}

	if( pc_loader.func_register( ) != 0 ) { 
		printf( "ERROR: Duplicate Registering Data Pools for Component Loader: %s\n", pc_loader.name.c_str( ) );
		return;
	}

	PCLoader loader;

	loader.name = pc_loader.name;

	loader.func_alloc = pc_loader.func_alloc;
	loader.func_update = pc_loader.func_update;
	loader.func_release = pc_loader.func_release;


	loader.func_is_over = pc_loader.func_is_over;
	loader.func_is_down = pc_loader.func_is_down;

	loader.func_enter = pc_loader.func_enter;
	loader.func_over = pc_loader.func_over;
	loader.func_exit = pc_loader.func_exit;

	loader.func_down = pc_loader.func_down;
	loader.func_hold = pc_loader.func_hold;
	loader.func_up = pc_loader.func_up;

	loader.func_mesh = pc_loader.func_mesh;

	loader.func_action = pc_loader.func_action;

	list_page_component_loaders.push_back( loader );
	map_page_component_loaders.insert( { loader.name, ( int ) list_page_component_loaders.size( ) - 1 } );

	printf( "SUCCESS: Added Page Component Loader: %s\n", loader.name.c_str( ) );
}

PCLoader * GuiMgr::get_component_loader( std::string const & name_loader ) { 
	return &list_page_component_loaders[ map_page_component_loaders[ name_loader ] ];
}

PCLoader * GuiMgr::get_component_loader_safe( std::string const & name_loader ) {
	auto iter_loader = map_page_component_loaders.find( name_loader );

	if( iter_loader == map_page_component_loaders.end( ) ) { 
		return nullptr;
	}

	return &list_page_component_loaders[ iter_loader->second ];
}

void GuiMgr::add_page_loader( PageLoader & page_loader ) { 
	if( map_page_loaders.find( page_loader.name ) != map_page_loaders.end( ) ) { 
		printf( "ERROR: Duplicate Page Loader: %s\n", page_loader.name.c_str( ) );
		return;
	}
	
	if( page_loader.func_register( ) != 0 ) {
		printf( "ERROR: Duplicate Registering Data Pools for Page Loader: %s\n", page_loader.name.c_str( ) );
		return;
	}

	PageLoader loader;

	loader.name = page_loader.name;

	loader.func_alloc = page_loader.func_alloc;
	loader.func_release = page_loader.func_release;

	loader.func_update = page_loader.func_update;
	loader.func_mesh = page_loader.func_mesh;

	list_page_loaders.push_back( loader );
	map_page_loaders.insert( { loader.name, ( int ) list_page_loaders.size( ) - 1 } );

	printf( "SUCCESS: Added Page Loader: %s\n", loader.name.c_str( ) );
}

void GuiMgr::process_remove( ) { 
	while( !queue_remove.empty( ) ) {
		auto iter_page = map_pages.find( queue_remove.front( ) );
		queue_remove.pop( );

		if( iter_page == map_pages.end( ) ) {
			return;
		}

		auto iter_order = std::find( list_order.begin( ), list_order.end( ), iter_page->second );
		if( iter_order == list_order.end( ) ) {
			return;
		}

		auto page = &iter_page->second.get( );
		remove_comp( page->root );
		page->h_root.release( );
		page->vbo_mesh.clear( );

		iter_page->second.release( );
		list_order.erase( iter_order );
		map_pages.erase( iter_page );
	}
}

PageLoader * GuiMgr::get_page_loader( std::string const & name_loader ) { 
	return &list_page_loaders[ map_page_loaders[ name_loader ] ];
}

PageLoader * GuiMgr::get_page_loader_safe( std::string const & name_loader ) {
	auto iter_map = map_page_loaders.find( name_loader );

	if( iter_map == map_page_loaders.end( ) ) { 
		return nullptr;
	}

	return &list_page_loaders[ iter_map->second ];
}

Page * GuiMgr::add_page( std::string const & name_page, std::string const & name_loader, PageFunc func_custom ) { 
	if( map_pages.find( name_page ) != map_pages.end( ) ) { 
		printf( "ERROR: Duplicate Page: %s\n", name_page.c_str( ) );
		return nullptr;
	}

	PageLoader * loader = get_page_loader_safe( name_loader );

	if( loader == nullptr ) { 
		printf( "ERROR: No Page Loader found: %s\n", name_loader.c_str( ) );
		return nullptr;
	}

	Handle< Page > handle_page;
	auto page = client.resource_mgr.allocate( handle_page );

	if( page == nullptr ) { 
		printf( "ERROR: Cannot allocate Page Handle: %s\n", name_page.c_str( ) );
		return nullptr;
	}
	page->root = client.resource_mgr.allocate( page->h_root );

	if( !page->root ) { 
		printf( "ERROR: Cannot allocate Root Handle: %s\n", name_page.c_str( ) );
		client.resource_mgr.release( handle_page );
		return nullptr;
	}

	page->root->anchor = { 0.0f, 0.0f };
	page->root->dim = { 100, 100 };
	page->root->offset = { 0.0f, 0.0f };

	page->root->name = "Root";
	page->root->is_visible = true;
	page->root->client = &client;
	page->root->page = page;
	page->root->parent = nullptr;
	page->root->pc_loader = get_component_loader_safe( "Root" );

	if( !page->root->pc_loader ) {
		printf( "ERROR: No Component Loader found for Root\n" );
		client.resource_mgr.release( page->h_root );
		client.resource_mgr.release( handle_page );
		return nullptr;
	}

	page->client = &client;

	page->name = name_page;
	page->loader = loader;

	page->is_visible = true;

	if( page->loader->func_alloc( page ) != 0 ) { 
		printf( "ERROR: Error Allocating Page: %s\n", name_page.c_str( ) );
		client.resource_mgr.release( page->h_root );
		client.resource_mgr.release( handle_page );
		return nullptr;
	}

	page->vbo_mesh.init( );

	func_custom( page );
	   
	page->reposition( );

	map_pages.insert( { name_page, handle_page } );
	list_order.push_front( handle_page );

	printf( "SUCCESS: Added Page: %s\n", name_page.c_str( ) );
	return page;
}

Page * GuiMgr::get_page( std::string const & name ) {
	return &map_pages[ name ].get( );
}

Page * GuiMgr::get_page_safe( std::string const & name ) {
	auto iter_map = map_pages.find( name );

	if( iter_map == map_pages.end( ) ) { 
		return nullptr;
	}

	return &map_pages[ name ].get( );
}

void GuiMgr::toggle_input( ) {
	//comp_selected = 

	is_input = !is_input;
}

bool GuiMgr::get_is_input( ) {
	return is_input;
}

void GuiMgr::handle_char( char const c ) {
	if( !comp_selected ) { 
		return;
	}

	if( !isprint( c ) ) {
		return;
	}

	PComp * parent = get_named_parent( comp_selected, "TextField" );

	if( !parent ) { 
		return;
	}

	auto data = parent->get_data< TextFieldComp::TextFieldData >( );

	if( data->pos_hl_e > data->pos_hl_s ) {
		data->text.erase(
			data->text.begin( ) + data->pos_hl_s,
			data->text.begin( ) + data->pos_hl_e );
		data->pos_hl_e = data->pos_hl_s;
		data->pos_curs = data->pos_hl_s;
	}

	if( data->pos_curs > data->text.size( ) ) { 
		data->pos_curs = ( int ) data->text.size( );
	}

	data->text.insert( data->text.begin( ) + data->pos_curs, c );
	data->pos_curs += 1;

	parent->page->is_remesh = true;
}

void GuiMgr::handle_vkey( int const key, bool const is_down ) {
	if( key == VK_SHIFT ) { 
		is_shift = is_down;
		return;
	}

	if( !is_down ) { 
		return;
	}

	PComp * parent = get_named_parent( comp_selected, "TextField" );

	if( !parent ) { 
		return;
	}

	auto data_text = parent->get_data< TextFieldComp::TextFieldData >( );

	switch( key ) { 
		case VK_BACK:
			if( data_text->pos_hl_e > data_text->pos_hl_s ) {
				data_text->text.erase(
					data_text->text.begin( ) + data_text->pos_hl_s,
					data_text->text.begin( ) + data_text->pos_hl_e );
				data_text->pos_hl_e = data_text->pos_hl_s;
				data_text->pos_curs = data_text->pos_hl_s;
			}
			else { 
				if( data_text->text.empty( ) ) {
					return;
				}

				if( data_text->pos_curs <= 0 ) {
					return;
				}

				data_text->text.erase( data_text->text.begin( ) + data_text->pos_curs - 1 );
				data_text->pos_curs -= 1;
			}

			parent->page->is_remesh = true;

			return;

		case VK_LEFT:
			if( data_text->pos_curs > 0 ) {
				data_text->pos_curs -= 1;
				data_text->pos_hl_e = data_text->pos_hl_s;

				parent->page->is_remesh = true;
			}

			return;

		case VK_UP: {
			auto data_page = parent->page->get_data < ConsolePage::ConsoleData > ( );			
			
			if( data_page->idx_history_recall == 0 ) {
				data_page->list_history[ data_page->idx_history ] = data_text->text;
			}

			data_page->idx_history_recall++;

			int idx = data_page->idx_history - data_page->idx_history_recall;
			idx = ( ( idx % ConsolePage::ConsoleData::num_history_max ) + 
				ConsolePage::ConsoleData::num_history_max ) % 
				ConsolePage::ConsoleData::num_history_max;

			data_text->text = data_page->list_history[ idx ];

			data_text->pos_curs = ( int ) data_text->text.size( );
			data_text->pos_hl_s = 0;
			data_text->pos_hl_e = 0;

			return;
		}

		case VK_DOWN: {
			auto data_page = parent->page->get_data < ConsolePage::ConsoleData >( );
			data_page->idx_history_recall--;

			if( data_page->idx_history_recall < 0 ) {
				data_page->idx_history_recall = 0;
			}

			int idx = data_page->idx_history - data_page->idx_history_recall;
			idx = ( ( idx % ConsolePage::ConsoleData::num_history_max ) +
				ConsolePage::ConsoleData::num_history_max ) %
				ConsolePage::ConsoleData::num_history_max;

			data_text->text = data_page->list_history[ idx ];

			data_text->pos_curs = ( int ) data_text->text.size( );
			data_text->pos_hl_s = 0;
			data_text->pos_hl_e = 0;

			return;
		}

		case VK_RIGHT:
			data_text->pos_curs += 1;
			data_text->pos_hl_e = data_text->pos_hl_s;
			parent->page->is_remesh = true;

			return;

		case VK_RETURN: {
			process_input( );

			data_text->pos_curs += 0;
			data_text->pos_hl_s = 0;
			data_text->pos_hl_e = 0;

			return;
		}
	}
}

void GuiMgr::process_input( ) { 
	PComp * parent = get_named_parent( comp_selected, "TextField" );

	if( !parent || parent->page->loader->name != "Console" ) { 
		return;
	}

	auto data = parent->get_data< TextFieldComp::TextFieldData >( );

	auto & str_command = data->text;
	if( str_command.size( ) <= 0 ) return;

	auto data_page = parent->page->get_data< ConsolePage::ConsoleData >( );

	data_page->list_history[ data_page->idx_history ] = str_command;
	data_page->idx_history_recall = 0;

	data_page->idx_history++;
	data_page->idx_history = ( 
		( data_page->idx_history % ConsolePage::ConsoleData::num_history_max ) + 
			ConsolePage::ConsoleData::num_history_max 
		) % ConsolePage::ConsoleData::num_history_max;

	auto & out = client.display_mgr.out;
	int pos_start = 0;
	int pos_end = 0;
	std::string token = "";

	pos_start = ( int ) str_command.find( "/cmd " );
	pos_end = ( int ) str_command.find( " ", pos_start + 5 );

	if( pos_start != str_command.npos ) {
		pos_start += 5;
		token = str_command.substr( pos_start, pos_end - pos_start );

		if( token == "resetpos" ) {
			auto & pos = client.entity_mgr.entity_player->h_state.get( ).pos;
			pos = glm::vec3( 0, WorldSize::Chunk::size_y / 2 + 5.0f, 0 );
			out << "Command: Resetting position to " << Directional::print_vec( pos );
			print_to_console( out.str( ) );
		}
		else if( token == "clear" ) {  
			auto data = parent->page->get_data< ConsolePage::ConsoleData >( );
			data->clear_text( );
		}
		else if( token == "printprio" ) {
			int range = 0;
			std::vector< std::string > list_delim { "r:", "b:" };
			auto map = get_tokenized_ints( str_command, list_delim );

			auto iter_map = map.find( list_delim[ 0 ] );
			if( iter_map != map.end( ) && iter_map->second.size() == 1 ) { 
				range = iter_map->second[ 0 ];
				iter_map = map.find( list_delim[ 1 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 1 ) {  
					client.chunk_mgr.print_prio( range, iter_map->second[ 0 ] );
				}
				else { 
					client.chunk_mgr.print_prio( range, 2 );
				}
			}
		}
		else if( token == "s" || token == "sph" || token == "sphere" ) { 
			int r = 0;
			int id = block_selector.get_id_block( );
			auto & pos_camera = client.display_mgr.camera.pos_camera;
			glm::ivec3 vec_pos( floor( pos_camera.x ), floor( pos_camera.y ), floor( pos_camera.z ) );
			std::vector< std::string > list_delim { "r:", "p:", "id:" };
			auto map = get_tokenized_ints( str_command, list_delim );

			auto iter_map = map.find( list_delim[ 0 ] );
			if( iter_map != map.end() && iter_map->second.size( ) > 0 ) {
				r = iter_map->second[ 0 ];

				iter_map = map.find( list_delim[ 1 ] );
				if( iter_map != map.end( ) ) {
					if( iter_map->second.size( ) == 3 ) {
						vec_pos.x = iter_map->second[ 0 ];
						vec_pos.y = iter_map->second[ 1 ];
						vec_pos.z = iter_map->second[ 2 ];
					}
				}

				iter_map = map.find( list_delim[ 2 ] );
				if( iter_map != map.end( ) ) { 
					if( iter_map->second.size( ) == 1 ) { 
						id = iter_map->second[ 0 ];
					}
				}

				pos_start = ( int ) str_command.find( "n:" );
				if( pos_start != std::string::npos ) {
					pos_end = ( int ) str_command.find( " ", pos_start + 2 );
					token = str_command.substr( pos_start + 2, pos_end - ( pos_start + 2 ) );
					pos_start = pos_end;

					if( pos_end != std::string::npos ) {
						while( pos_end != std::string::npos && ( pos_end = ( int ) str_command.find( " ", pos_start + 1 ) ) <= ( int ) str_command.find( ":", pos_start + 1 ) ) {
							token += str_command.substr( pos_start, ( pos_end - pos_start ) );
							pos_start = pos_end;
						}
					}

					if( token == "Air" ) {
						id = -1;
					}
					else {
						BlockLoader * ptr_block = client.block_mgr.get_block_loader_safe( token );
						if( ptr_block ) {
							id = ptr_block->id;
						}
					}
				}

				out.str( "" );
				out << "Command: Sphere at:" << Directional::print_vec( vec_pos ) << " radius:" << r << " type:" << client.block_mgr.get_block_string( id ) << ".";
				print_to_console( out.str( ) );
				client.thread_mgr.task_async( 10, [ &, vec_pos, r, id ] ( ) { 
					client.chunk_mgr.set_sphere( vec_pos, r, id );
				} );
			}
			else { 
				out.str( "" );
				out << "Command: Incorrect sphere usage - <required> [optional]";
				print_to_console( out.str( ) );
				out.str( "" );
				out << "Command Format is: '/cmd <s, sph, sphere> <r:int> [p:int int int | id:int]'.";
				print_to_console( out.str( ) );
			}
		}
		else if( token == "r" || token == "rect" || token == "rectangle" ) {
			int id = block_selector.get_id_block( );
			auto & pos_camera = client.display_mgr.camera.pos_camera;
			glm::ivec3 vec_pos( floor( pos_camera.x ), floor( pos_camera.y ), floor( pos_camera.z ) );
			glm::ivec3 vec_dim( 0, 0, 0 );
			std::vector< std::string > list_delim { "d:", "p:", "id:" };
			auto map = get_tokenized_ints( str_command, list_delim );

			auto iter_map = map.find( list_delim[ 0 ] );
			if( iter_map != map.end( ) && iter_map->second.size( ) == 3 ) { 
				vec_dim.x = iter_map->second[ 0 ];
				vec_dim.y = iter_map->second[ 1 ];
				vec_dim.z = iter_map->second[ 2 ];

				iter_map = map.find( list_delim[ 1 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 3 ) { 
					vec_pos.x = iter_map->second[ 0 ];
					vec_pos.y = iter_map->second[ 1 ];
					vec_pos.z = iter_map->second[ 2 ];
				}

				iter_map = map.find( list_delim[ 2 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 1 ) { 
					id = iter_map->second[ 0 ];
				}

				pos_start = ( int ) str_command.find( "n:" );
				if( pos_start != std::string::npos ) {
					pos_end = ( int ) str_command.find( " ", pos_start + 2 );
					token = str_command.substr( pos_start + 2, pos_end - ( pos_start + 2 ) );
					pos_start = pos_end;

					if( pos_end != std::string::npos ) {
						while( pos_end != std::string::npos && ( pos_end = ( int ) str_command.find( " ", pos_start + 1 ) ) <= ( int ) str_command.find( ":", pos_start + 1 ) ) {
							token += str_command.substr( pos_start, ( pos_end - pos_start ) );
							pos_start = pos_end;
						}
					}

					if( token == "Air" ) { 
						id = -1;
					}
					else {
						BlockLoader * ptr_block = client.block_mgr.get_block_loader_safe( token );
						if( ptr_block ) {
							id = ptr_block->id;
						}
					}
				}

				out.str( "" );
				out << "Command: Rectangle at:" << Directional::print_vec( vec_pos ) << " dim:" << Directional::print_vec( vec_dim ) << " type:" << client.block_mgr.get_block_string( id ) << ".";
				print_to_console( out.str( ) );

				client.thread_mgr.task_async( 10, [ &, vec_pos, vec_dim, id ] ( ) {
					client.chunk_mgr.set_rect( vec_pos, vec_dim, id );
				} );
			}
			else { 
				out.str( "" );
				out << "Command: Incorrect rectanglular box usage - <required> [optional]";
				print_to_console( out.str( ) );
				out.str( "" );
				out << "Command Format is: '/cmd <r, rect, rectangle> <d:int int int> [p:int int int | id:int]'.";
				print_to_console( out.str( ) );
			}
		}
		else if( token == "e" || token == "ellip" || token == "ellipsoid" ) {
			int id = block_selector.get_id_block( );
			auto & pos_camera = client.display_mgr.camera.pos_camera;
			glm::ivec3 vec_pos( floor( pos_camera.x ), floor( pos_camera.y ), floor( pos_camera.z ) );
			glm::ivec3 vec_dim( 0, 0, 0 );
			std::vector< std::string > list_delim { "d:", "p:", "id:" };
			auto map = get_tokenized_ints( str_command, list_delim );

			auto iter_map = map.find( list_delim[ 0 ] );
			if( iter_map != map.end( ) && iter_map->second.size( ) == 3 ) {
				vec_dim.x = iter_map->second[ 0 ];
				vec_dim.y = iter_map->second[ 1 ];
				vec_dim.z = iter_map->second[ 2 ];

				iter_map = map.find( list_delim[ 1 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 3 ) {
					vec_pos.x = iter_map->second[ 0 ];
					vec_pos.y = iter_map->second[ 1 ];
					vec_pos.z = iter_map->second[ 2 ];
				}

				iter_map = map.find( list_delim[ 2 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 1 ) {
					id = iter_map->second[ 0 ];
				}

				pos_start = ( int ) str_command.find( "n:" );
				if( pos_start != std::string::npos ) {
					pos_end = ( int ) str_command.find( " ", pos_start + 2 );
					token = str_command.substr( pos_start + 2, pos_end - ( pos_start + 2 ) );
					pos_start = pos_end;

					if( pos_end != std::string::npos ) {
						while( pos_end != std::string::npos && ( pos_end = ( int ) str_command.find( " ", pos_start + 1 ) ) <= ( int ) str_command.find( ":", pos_start + 1 ) ) {
							token += str_command.substr( pos_start, ( pos_end - pos_start ) );
							pos_start = pos_end;
						}
					}

					if( token == "Air" ) {
						id = -1;
					}
					else {
						BlockLoader * ptr_block = client.block_mgr.get_block_loader_safe( token );
						if( ptr_block ) {
							id = ptr_block->id;
						}
					}
				}

				out.str( "" );
				out << "Command: Ellipsoid at:" << Directional::print_vec( vec_pos ) << " dim:" << Directional::print_vec( vec_dim ) << " type:" << client.block_mgr.get_block_string( id ) << ".";
				print_to_console( out.str( ) );

				client.thread_mgr.task_async( 10, [ &, vec_pos, vec_dim, id ] ( ) {
					client.chunk_mgr.set_ellipsoid( vec_pos, vec_dim, id );
				} );
			}
			else {
				out.str( "" );
				out << "Command: Incorrect ellipsoid usage - <required> [optional]";
				print_to_console( out.str( ) );
				out.str( "" );
				out << "Command Format is: '/cmd <e, ellip, ellipsoid> <d:int int int> [p:int int int | id:int]'.";
				print_to_console( out.str( ) );
			}
		}
		else if( token == "setsun" ) { 
			std::vector< std::string > list_delim { "d:", "p:" };
			auto map = get_tokenized_ints( str_command, list_delim );

			auto iter_map = map.find( list_delim[ 0 ] );
			if( iter_map != map.end( ) && iter_map->second.size( ) == 1 ) {
				//client.chunk_mgr.set_sun( iter_map->second[ 0 ] );
			}
			else { 
				out.str( "" );
				out << "Command: Incorrect setsun usage - <required> [optional]";
				print_to_console( out.str( ) );
				out.str( "" );
				out << "Command Format is: '/cmd <setsun> <d:int> [p:bool]'.";
				print_to_console( out.str( ) );
				return;
			}

			if( ( iter_map = map.find( list_delim[ 1 ] ) ) != map.end( ) && iter_map->second.size() == 1 ) {
				client.chunk_mgr.set_sun_pause( iter_map->second[ 0 ] != 0 );
			}
		}
		else if( token == "setfov" ) {
			pos_start = ( int ) str_command.find( "f:", 0 );
			if( pos_start == std::string::npos ) {
				return;
			}

			pos_start += 2;
			float fov = atof( str_command.substr( pos_start ).c_str( ) );

			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "Setting fov to: \'" << fov << "\'";
			print_to_console( out.str( ) );

			client.display_mgr.fov = fov;
			client.display_mgr.set_proj( );
		}
		else if( token == "bias_l" ) {
			pos_start = ( int ) str_command.find( "f:", 0 );
			if( pos_start == std::string::npos ) {
				return;
			}

			pos_start += 2;
			float bias = atof( str_command.substr( pos_start ).c_str( ) );

			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "Setting bias_l to: \'" << bias << "\'";
			print_to_console( out.str( ) );

			client.texture_mgr.bind_program( "Terrain" );
			int idx_bias = glGetUniformLocation( client.texture_mgr.id_bound_program, "bias_l" );
			glUniform1f( idx_bias, bias );
		}
		else if( token == "bias_b" ) {
			pos_start = ( int ) str_command.find( "f:", 0 );
			if( pos_start == std::string::npos ) {
				return;
			}

			pos_start += 2;
			float bias = atof( str_command.substr( pos_start ).c_str( ) );

			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "Setting bias_b to: \'" << bias << "\'";
			print_to_console( out.str( ) );

			client.texture_mgr.bind_program( "Terrain" );
			int idx_bias = glGetUniformLocation( client.texture_mgr.id_bound_program, "bias_l" );
			glUniform1f( idx_bias, bias );
			idx_bias = glGetUniformLocation( client.texture_mgr.id_bound_program, "bias_h" );
			glUniform1f( idx_bias, bias );
		}
		else if( token == "bias_h" ) {
			pos_start = ( int ) str_command.find( "f:", 0 );
			if( pos_start == std::string::npos ) {
				return;
			}

			pos_start += 2;
			float bias = atof( str_command.substr( pos_start ).c_str( ) );

			auto & out = client.display_mgr.out;
			out.str( "" );
			out << "Setting bias_h to: \'" << bias << "\'";
			print_to_console( out.str( ) );

			client.texture_mgr.bind_program( "Terrain" );
			int idx_bias = glGetUniformLocation( client.texture_mgr.id_bound_program, "bias_h" );
			glUniform1f( idx_bias, bias );
		}
		else if( token == "addemitter" ) {
			std::vector< std::string > list_delim { "p:", "c:", "r:" };
			auto map = get_tokenized_ints( str_command, list_delim );

			auto iter_map = map.find( list_delim[ 0 ] );
			if( iter_map != map.end( ) && iter_map->second.size( ) == 3 ) {
				Emitter e;
				e.pos.x = iter_map->second[ 0 ];
				e.pos.y = iter_map->second[ 1 ];
				e.pos.z = iter_map->second[ 2 ];
				e.pos.w = 1.0f;

				e.color = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
				iter_map = map.find( list_delim[ 1 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 3 ) {
					e.color = { 
						e.color.r = iter_map->second[ 0 ] / 255.0f,
						e.color.g = iter_map->second[ 1 ] / 255.0f,
						e.color.b = iter_map->second[ 2 ] / 255.0f,
						e.color.a = 0.0f
					};
				}

				e.radius = 10.0f;
				iter_map = map.find( list_delim[ 2 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 1 ) { 
					e.radius = iter_map->second[ 0 ];
				}

				client.chunk_mgr.add_emitter( e );

				out.str( "" );
				out << "Command: Emitter added at " << Directional::print_vec( e.pos ) << 
					" with color " << Directional::print_vec( e.color ) << 
					" and radius " << e.radius << ".";
				print_to_console( out.str( ) );
			}
			else {
				out.str( "" );
				out << "Command: Incorrect addemitter usage - <required> [optional] (range)";
				print_to_console( out.str( ) );
				out.str( "" );
				out << "Command Format is: '/cmd <addemitter> <p:int, int, int> [c:int, int, int](0-255) [r:int]'.";
				print_to_console( out.str( ) );
			}
		}
		else if( token == "clearemitters" ) {
			client.chunk_mgr.clear_emitters( );
		}
		else if( token == "printdirty" ) { 
			client.chunk_mgr.print_dirty( );
		}
		else if( token == "togglewireframe" ) { 
			static bool is_wireframe = false;
			is_wireframe = !is_wireframe;
			if( is_wireframe ) { 
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
				glDisable( GL_CULL_FACE );
			}
			else { 
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
				glEnable( GL_CULL_FACE );
			}
		}
		else if( token == "toggleshadows" ) {
			client.chunk_mgr.toggle_shadows( );
		}
		else if( token == "toggleshadowmap" ) {
			client.chunk_mgr.toggle_shadow_debug( );
		}
		else if( token == "togglelimiter" ) {
			client.display_mgr.toggle_limiter( );
		}
		else if( token == "togglevsync" ) {
			client.display_mgr.toggle_vsync( );
		}
		else if( token == "toggleflatshade" ) {
			client.chunk_mgr.toggle_flatshade( );
		}
		else if( token == "printchunkmesh" ) {
			client.chunk_mgr.print_center_chunk_mesh( );
		}
		else if( token == "saveall" ) {
			client.chunk_mgr.save_all( );
		}
		else if( token == "shutdownall" ) {
			client.chunk_mgr.shutdown_all( );
		}
		else if( token == "deleteworld" ) {
			client.chunk_mgr.shutdown_all( );
			//while( client.chunk_mgr.
			client.chunk_mgr.delete_world( );
		}
		else if( token == "chunkflow" ) { 
			client.chunk_mgr.flow_center_chunk( );
		}
		else {
			out.str( "" );
			out << "Command: '" << token << "' is not a valid command.";
			print_to_console( out.str( ) );

			out.str( "" );
			out << "Command: Format is: '/cmd <command> <required arguements> [optional arguements]'.";
			print_to_console( out.str( ) );

			out.str( "" );
			out << "Command: Valid Commands: 'resetpos', 'sphere', 'rectangle', 'ellipsoid', 'setsun', 'printdirty', 'addemitter'";
			print_to_console( out.str( ) );
		}
	}
	else {
		print_to_console( "Say: " + str_command );
	}

	data->text.clear( );
}

void GuiMgr::print_to_console( std::string const & str_out ) { 
	auto page = get_page( "Console" );

	if( !page ) { 
		return;
	}

	page->get_data< ConsolePage::ConsoleData >( )->push_text( str_out );
}

std::unordered_map< std::string, std::vector< int > > get_tokenized_ints( 
	std::string & str_in, std::vector< std::string > & list_delim ) {
	std::unordered_map< std::string, std::vector< int > > map_token;

	std::vector< std::pair< int, int > > list_pair;
	std::vector< std::string > list_token;
	int index;

	for( int i = 0; i < list_delim.size( ); i++ ) {
		index = ( int ) str_in.find( list_delim[ i ], 0 );
		if( index != str_in.npos ) {
			list_pair.push_back( { i, index } );
		}
	}

	if( list_pair.empty( ) ) {
		return map_token;
	}

	std::sort(
		list_pair.begin( ),
		list_pair.end( ),
		[ ] ( std::pair< int, int > const & lho, std::pair< int, int > const & rho ) {
		return lho.second < rho.second;
	}
	);

	for( int i = 0; i < list_pair.size( ) - 1; i++ ) {
		list_token.push_back(
			str_in.substr(
				list_pair[ i ].second + list_delim[ list_pair[ i ].first ].size( ),
				list_pair[ i + 1 ].second - ( list_pair[ i ].second + list_delim[ list_pair[ i ].first ].size( ) )
				)
			);
	}

	list_token.push_back(
		str_in.substr(
			list_pair[ list_pair.size( ) - 1 ].second + list_delim[ list_pair[ list_pair.size( ) - 1 ].first ].size( ),
			str_in.npos
			)
		);

	std::string token_int;
	int start, end;

	for( int i = 0; i < list_token.size( ); i++ ) {
		auto & list_int = map_token.insert( { list_delim[ list_pair[ i ].first ], { } } ).first->second;
		start = end = 0;

		while( end != list_token[ i ].npos ) {
			end = ( int ) list_token[ i ].find( " ", start );
			token_int = list_token[ i ].substr( start, end - start );

			if( token_int.size( ) > 0 && 
				isdigit( token_int[ 0 ] ) ) {

				list_int.push_back( std::stoi( token_int ) );
			}
			else if( token_int.size( ) > 0 && 
				token_int[ 0 ] == '-' && 
				isdigit( token_int[ 1 ] ) ) {

				token_int.erase( token_int.begin( ) );
				list_int.push_back( -std::stoi( token_int ) );
			}
			start = end + 1;
		}
	}

	return map_token;
}