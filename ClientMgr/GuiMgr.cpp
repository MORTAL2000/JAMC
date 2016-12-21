#include "GuiMgr.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Client.h"
#include "WorldSize.h"

#include "TestPage.h"

#include "TestComp.h"
#include "TextButtonComp.h"
#include "LabelComp.h"
#include "ImageComp.h"
#include "BorderImageComp.h"


GuiMgr::GuiMgr( Client & client ) :
	Manager( client ),
	block_selector( client ) { }

GuiMgr::~GuiMgr( ) { }

void GuiMgr::init( ) {
	printf( "\n*** Gui Manager ***\n" );
	list_pages.reserve( size_pages );
	map_pages.reserve( size_pages );
	//list_order.reserve( size_pages );

	GL_CHECK( block_selector.init( ) );

	printf( "\nLoading Components...\n\n" );
	load_components( );

	printf( "\nLoading Pages...\n\n" );
	load_pages( );

	is_visible = true;
}

void GuiMgr::update( ) {
	block_selector.update( );

	Page * page;
	for( int unsigned i = 0; i < list_pages.size( ); ++i ) { 
		page = &list_pages[ i ].get( );

		page->page_loader->func_update( page );
		page->reposition( );

		update_comps( page );

		if( page->is_visible && page->is_remesh ) {
			client.thread_mgr.task_main( 10, [ this, page ] ( ) {
				page->vbo_mesh.clear( );

				page->page_loader->func_mesh( page );
				mesh_comps( page );

				page->vbo_mesh.buffer( );
			} );

			page->is_remesh = false;
		}
	}

	if( is_visible && client.input_mgr.is_cursor_vis( ) ) {
		if( client.input_mgr.is_mouse_down( 0 ) ) {
			on_down( 0 );
		}
		if( client.input_mgr.get_mouse_hold( 0 ) ) {
			on_hold( 0 );
		}
		if( client.input_mgr.is_mouse_up( 0 ) ) {
			on_up( 0 );
		}
	}
}

void GuiMgr::update_comps( Page * page ) {
	auto iter_comp = page->list_comps.begin( );

	while( iter_comp != page->list_comps.end( ) ) {
		update_comps( &iter_comp->get( ) );

		++iter_comp;
	}
}

void GuiMgr::update_comps( PComp * comp ) {
	comp->pc_loader->func_update( comp );
	comp->reposition( );

	auto iter_comp = comp->list_comps.begin( );

	while( iter_comp != comp->list_comps.end( ) ) { 
		update_comps( &iter_comp->get( ) );

		++iter_comp;
	}
}

void GuiMgr::mesh_comps( Page * page ) { 
	std::queue< PComp * > queue_comps;

	auto iter_comp = page->list_comps.begin( );
	while( iter_comp != page->list_comps.end( ) ) { 
		queue_comps.push( &iter_comp->get( ) );
		++iter_comp;
	}

	PComp * comp;
	while( !queue_comps.empty( ) ) { 
		comp = queue_comps.front( );
		queue_comps.pop( );

		comp->pc_loader->func_mesh( comp );

		iter_comp = comp->list_comps.begin( );
		while( iter_comp != comp->list_comps.end( ) ) { 
			queue_comps.push( &iter_comp->get( ) );
			++iter_comp;
		}
	}
}

void GuiMgr::render( ) {
	block_selector.render( );

	if( !is_visible ) {
		return;
	}

	auto iter_order = list_order.rbegin( );

	while( iter_order != list_order.rend( ) ) {
		auto page = &list_pages[ *iter_order ].get( );

		if( !page->is_visible ) {
			continue;
		}

		client.texture_mgr.update_uniform( "BasicOrtho", "mat_model", page->mat_model );
		page->vbo_mesh.render( client, true );

		iter_order++;
	}
}

void GuiMgr::end( ) { }

void GuiMgr::sec( ) { }

void GuiMgr::load_components( ) { 
	add_component_loader( TestComp( client ) );
	add_component_loader( TextButtonComp( client ) );
	add_component_loader( LabelComp( client ) );
	add_component_loader( ImageComp( client ) );
	add_component_loader( BorderImageComp( client ) );
}

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
	loader.func_release = pc_loader.func_release;

	loader.func_over = pc_loader.func_over;
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

void GuiMgr::load_pages( ) { 
	add_page_loader( TestPage( client ) );

	add_page( "Test", "Test", [ ] ( Page * page ) {
		page->dim = { 300, 300 };

		auto button = page->get_comp( "TestButton" );
		auto & button_data = button->get_data< TextButtonComp::ButtonData >( );

		button_data.func_action = [ ] ( PComp * comp ) {
			printf( "I am a banana!\n" );

			return 0;
		};

		return 0; 
	} );

	add_page( "Test2", "Test", [ ] ( Page * page ) { 
		auto & page_data = page->get_data< TestPage::TestData >( );
		page_data.color = { 0.5f, 1.0f, 0.5f, 0.5f };

		page->dim = { 300, 300 };
		page->offset = -page->dim;

		auto background = page->get_comp( "Background" );
		auto & bg_data = background->get_data< BorderImageComp::BorderImageData >( );
		bg_data.color = page_data.color;

		auto button = page->get_comp( "TestButton" );
		auto & button_data = button->get_data< TextButtonComp::ButtonData >( );

		button_data.func_action = [ ] ( PComp * comp ) {
			printf( "I am an orange!\n" );

			return 0;
		};

		return 0;
	} );
}

void GuiMgr::add_page_loader( PageLoader & page_loader ) { 
	if( map_page_loaders.find( page_loader.name ) != map_page_loaders.end( ) ) { 
		printf( "ERROR: Duplicate Page Loader: %s\n", page_loader.name.c_str( ) );
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

void GuiMgr::add_page( std::string const & name_page, std::string const & name_loader, PageFunc func_custom ) { 
	if( map_pages.find( name_page ) != map_pages.end( ) ) { 
		printf( "ERROR: Duplicate Page: %s\n", name_page.c_str( ) );
		return;
	}

	PageLoader * loader = get_page_loader_safe( name_loader );

	if( loader == nullptr ) { 
		printf( "ERROR: No Page Loader found: %s\n", name_loader.c_str( ) );
		return;
	}

	Handle< Page > handle_page;
	auto page = client.resource_mgr.allocate( handle_page );

	if( page == nullptr ) { 
		printf( "ERROR: Cannot allocate Page Handle: %s\n", name_page.c_str( ) );
		return;
	}

	page->client = &client;

	page->name = name_page;
	page->page_loader = loader;

	if( page->page_loader->func_alloc( page ) != 0 ) { 
		printf( "ERROR: Error Allocating Page: %s\n", name_page.c_str( ) );
		client.resource_mgr.release( handle_page );
		return;
	}

	page->vbo_mesh.init( );

	func_custom( page );

	page->reposition( );

	list_pages.push_back( handle_page );
	map_pages.insert( { name_page, ( int ) list_pages.size( ) - 1 } );
	list_order.push_back( ( int ) list_pages.size( ) - 1 );

	printf( "SUCCESS: Added Page: %s\n", name_page.c_str( ) );
}

void GuiMgr::on_down( int button ) {
	bool is_handled = false;

	auto & pos_mouse = client.input_mgr.get_mouse_down( button );

	Page * page;
	auto iter_order = list_order.begin( );
	while( !is_handled && iter_order != list_order.end( ) ) {
		page = &list_pages[ *iter_order ].get( );

		if( page->is_visible ) {
			if( Directional::is_point_in_rect( pos_mouse, page->pos, page->pos + page->dim ) ) {
				is_handled = page->on_down( button );

				if( is_handled ) { 
					int id_order = *iter_order;
					iter_order = list_order.erase( iter_order );
					list_order.push_front( id_order );
				}
			}
		}

		iter_order++;
	}
}

void GuiMgr::on_hold( int button ) {
	bool is_handled = false;
	auto iter_order = list_order.begin( );

	Page * page;
	while( !is_handled && iter_order != list_order.end( ) ) {
		page = &list_pages[ *iter_order ].get( );

		if( page->is_visible ) {
			is_handled = page->on_hold( button );
		}

		iter_order++;
	}
}

void GuiMgr::on_up( int button ) {
	bool is_handled = false;

	auto & pos_mouse = client.input_mgr.get_mouse_up( button );

	Page * page;
	auto iter_order = list_order.begin( );
	while( !is_handled && iter_order != list_order.end( ) ) {
		page = &list_pages[ *iter_order ].get( );

		if( page->is_visible ) {
			if( Directional::is_point_in_rect( pos_mouse, page->pos, page->pos + page->dim ) ) {
				is_handled = page->on_up( button );
			}
		}

		iter_order++;
	}
}

/*
void GuiMgr::print_to_console( std::string const & str_print ) {
	std::string str_console( "Console" );
	auto & page = get_page( str_console );
	auto & data_console = page.get_data< PCDConsole >( str_console );

	std::string token;
	int pos_s, pos_e;
	pos_s = 0;
	pos_e = 0;

	std::unique_lock< std::mutex > lock( mtx_console );

	while( pos_e != std::string::npos ) {
		pos_e = ( int ) str_print.find( '\n', pos_s );
		token = str_print.substr( pos_s, pos_e - pos_s );
		pos_s = pos_e + 1;

		if( data_console.size < PCDConsole::size_max ) {
			data_console.list_strings[ ( data_console.index + data_console.size ) % PCDConsole::size_max ] = token;
			data_console.size++;
		}
		else {
			data_console.list_strings[ ( data_console.index + data_console.size ) % PCDConsole::size_max ] = token;
			data_console.index++;
			if( data_console.index >= PCDConsole::size_max ) data_console.index -= PCDConsole::size_max;
		}
	}

	data_console.is_dirty = true;
}

void GuiMgr::print_to_static( std::string const & str_print ) {
	std::string str_static( "Static" );
	auto & page = get_page( str_static );
	auto & data_static = page.get_data< PCDStatic >( str_static );

	if( data_static.size < PCDStatic::size_max ) {
		data_static.list_strings[ ( data_static.index + data_static.size ) % PCDStatic::size_max ] = str_print;
		data_static.size++;
	}
	else {
		data_static.list_strings[ ( data_static.index + data_static.size ) % PCDStatic::size_max ] = str_print;
		data_static.index++;
		if( data_static.index >= PCDStatic::size_max ) data_static.index -= PCDStatic::size_max;
	}

	data_static.is_dirty = true;
}

void GuiMgr::update_static( ) { 
	std::string str_static( "Static" );
	auto & comp = get_page( str_static ).get_comp( str_static );

	PageCompFuncs::update_static( comp );
}

void GuiMgr::clear_static() {
	std::string str_static( "Static" );
	auto & data_static = get_page( str_static ).get_data< PCDStatic >( str_static );

	data_static.index = 0;
	data_static.size = 0;
}*/

void GuiMgr::toggle_input( ) {
	/*
	if( is_input ) {
		auto & page = get_page( std::string( "Console" ) );
		auto & comp_command = page.get_comp( std::string( "Command" ) );
		auto & data_command = page.get_data< PCDCommand >( std::string( "Console" ) );
		auto & data_text = page.get_data< PCDTextField >( std::string( "Command" ) );

		comp_command.color = { 0.0f, 0.0f, 0.0f, 0.2f };
		is_input = false;

		if( !data_command.str_command.size( ) ) { 
			return;
		}

		client.gui_mgr.process_input( );

		data_command.list_strings[ data_command.index ] = data_command.str_command;
		data_command.str_command = "";

		data_command.index += 1;
		if( data_command.index >= PCDCommand::size_max ) data_command.index -= PCDCommand::size_max;
	}
	else { 
		auto & page = get_page( std::string( "Console" ) );
		auto & comp_command = page.get_comp( std::string( "Command" ) );
		auto & data_command = page.get_data< PCDCommand >( std::string( "Console" ) );

		comp_command.color = { 0.0f, 0.0f, 0.0f, 0.3f };

		data_command.str_command = "";
		data_command.index_history = data_command.index;

		is_input = true;
	}
	*/
}

bool GuiMgr::get_is_input( ) {
	return is_input;
}

void GuiMgr::handle_input_char( int const key, bool const is_down ) { 
	/*
	std::string str_console( "Console" );
	auto & page = get_page( str_console );
	auto & data_command = page.get_data< PCDCommand >( str_console );
	//auto & data_text = page.get_data< PCDTextField >( std::string( "Command" ) );

	if( key == VK_SHIFT ) {
		if( is_down ) data_command.is_shift = true;
		else data_command.is_shift = false;
		return;
	}

	if( !is_down ) {
		return;
	}

	if( !data_command.is_shift ) {
		auto & command = data_command.str_command;
		if( key >= 65 && key <= 90 ) {
			command += key + 32;
		}
		else if( key >= 48 && key <= 57 ) { 
			command += key;
		}
		else {
			switch( key ) {
				case VK_BACK:
					if( command.size( ) > 0 ) {
						command.erase( command.end( ) - 1 );
					}
				break;
				case VK_UP:
					data_command.index_history -= 1;
					if( data_command.index_history < 0 ) data_command.index_history += PCDCommand::size_max;
					data_command.str_command = data_command.list_strings[ data_command.index_history ];
					//data_text.ptr_str = &data_command.list_strings[ data_command.index ];
				break;
				case VK_DOWN:
					data_command.index_history += 1;
					if( data_command.index_history >= PCDCommand::size_max ) data_command.index_history -= PCDCommand::size_max;
					data_command.str_command = data_command.list_strings[ data_command.index_history ];
					//data_text.ptr_str = &data_command.list_strings[ data_command.index ];
				break;
				case VK_SPACE:
					command += ' ';
				break;
				case VK_OEM_1:
					command += ';';
				break;
				case VK_OEM_2:
					command += '/';
				break;
				case VK_OEM_3:
					command += '`';
				break;
				case VK_OEM_4:
					command += '[';
				break;
				case VK_OEM_5:
					command += '\\';
				break;
				case VK_OEM_6:
					command += ']';
				break;
				case VK_OEM_7:
					command += '\'';
				break;
				case VK_OEM_MINUS:
					command += '-';
				break;
				case VK_OEM_PLUS:
					command += '=';
				break;
				case VK_OEM_COMMA:
					command += ',';
				break;
				case VK_OEM_PERIOD:
					command += '.';
				break;
				default:
				break;
			}
		}
	}
	else {
		auto & command = data_command.str_command;
		if( key >= 65 && key <= 90 ) {
			command += key;
		}
		else if( key >= 48 && key <= 57 ) {
			switch( key ) {
				case 48:
					command += ')';
				break;
				case 49:
					command += '!';
				break;
				case 50:
					command += '@';
				break;
				case 51:
					command += '#';
				break;
				case 52:
					command += '$';
				break;
				case 53:
					command += '%';
				break;
				case 54:
					command += '^';
				break;
				case 55:
					command += '&';
				break;
				case 56:
					command += '*';
				break;
				case 57:
					command += '(';
				break;
			}
		}
		else {
			switch( key ) {
				case VK_BACK:
				if( command.size( ) > 0 ) {
					command.clear( );
				}
				break;
				case VK_SPACE:
				command += ' ';
				break;
				case VK_OEM_1:
				command += ':';
				break;
				case VK_OEM_2:
				command += '?';
				break;
				case VK_OEM_3:
				command += '~';
				break;
				case VK_OEM_4:
				command += '{';
				break;
				case VK_OEM_5:
				command += '|';
				break;
				case VK_OEM_6:
				command += '}';
				break;
				case VK_OEM_7:
				command += '\"';
				break;
				case VK_OEM_MINUS:
				command += '_';
				break;
				case VK_OEM_PLUS:
				command += '+';
				break;
				case VK_OEM_COMMA:
				command += '<';
				break;
				case VK_OEM_PERIOD:
				command += '>';
				break;
				default:
				break;
			}
		}
	}
	*/
}

void GuiMgr::process_input( ) { 
	/*
	std::string str_console( "Console" );
	auto & page = get_page( str_console );
	auto & data_command = page.get_data< PCDCommand >( str_console );
	//auto & data_text = page.get_data< PCDTextField >( std::string( "Command" ) );
	auto & str_command = data_command.str_command;
	auto & out = client.display_mgr.out;
	int pos_start = 0;
	int pos_end = 0;
	std::string token = "";

	if( str_command.size( ) <= 0 ) return;

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
		else if( token == "sphere" ) { 
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
				out << "Command Format is: '/cmd <sphere> <r:int> [p:int int int | id:int]'.";
				print_to_console( out.str( ) );
			}
		}
		else if( token == "rectangle" ) {
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
				out << "Command: Incorrect rectangle usage - <required> [optional]";
				print_to_console( out.str( ) );
				out.str( "" );
				out << "Command Format is: '/cmd <rectangle> <d:int int int> [p:int int int | id:int]'.";
				print_to_console( out.str( ) );
			}
		}
		else if( token == "ellipsoid" ) {
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
				out << "Command Format is: '/cmd <ellipsoid> <d:int int int> [p:int int int | id:int]'.";
				print_to_console( out.str( ) );
			}
		}
		else if( token == "setsun" ) { 
			std::vector< std::string > list_delim { "d:", "p:" };
			auto map = get_tokenized_ints( str_command, list_delim );

			auto iter_map = map.find( list_delim[ 0 ] );
			if( iter_map != map.end( ) && iter_map->second.size( ) == 1 ) {
				client.chunk_mgr.set_sun( iter_map->second[ 0 ] );
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
		else if( token == "printglerror" ) { 
			auto & out = client.display_mgr.out;
			out.str( "" );
			//out << checkGlErrors( );
			client.gui_mgr.print_to_console( out.str( ) );
		}
		else if( token == "printchunkmesh" ) {
			client.chunk_mgr.print_center_chunk_mesh( );
		}
		/*else if( token == "clearmesh" ) { 
			block_selector.clear_mesh( );
		}
		else if( token == "releasemesh" ) {
			block_selector.release_mesh( );
		}
		else if( token == "makemesh" ) {
			client.thread_mgr.task_async( 10, [ & ] ( ) {
				block_selector.make_mesh( );
			} );
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
	*/
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

	if( list_pair.size( ) > 0 ) {
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
				if( token_int.size( ) > 0 && isdigit( token_int[ 0 ] ) ) {
					list_int.push_back( std::stoi( token_int ) );
				}
				start = end + 1;
			}
		}
	}

	return map_token;
}