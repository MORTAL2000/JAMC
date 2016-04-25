#include "GuiMgr.h"
#include "Client.h"
#include "glm/glm.hpp"

GuiMgr::GuiMgr( Client & client ) :
	Manager( client ) { }

GuiMgr::~GuiMgr( ) { }

void GuiMgr::init( ) {
	list_page.reserve( size_pages );
	map_page.reserve( size_pages );
	list_order.reserve( size_pages );
	client.resource_mgr.reg_pool< PCDTextField >( 1000 );
	client.resource_mgr.reg_pool< PageComp >( 1000 );

	add_page( std::string( "Console" ),
		PageFuncs::alloc_console,
		PageFuncs::cust_null
		);

	add_page( std::string( "Static" ),
		PageFuncs::alloc_static,
		PageFuncs::cust_null
		);

	is_visible = true;
}

void GuiMgr::update( ) {
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

	for( int i = 0; i < list_order.size( ); i++ ) { 
		list_order[ i ]->update( );
	}
}

void GuiMgr::render( ) {
	if( is_visible ) {
		for( int i = 0; i < list_order.size( ); i++ ) {
			auto & page = *list_order[ i ];
			if( page.is_visibile ) {
				client.texture_mgr.bind_materials( );
				auto & uvs = client.texture_mgr.get_uvs_materials( );

				client.display_mgr.draw_quad( page.vec_pos, page.vec_dim, page.color, uvs );
				for( int i = 0; i < page.list_comps.size( ); i++ ) {
					auto & comp = page.list_comps[ i ].get( );
					if( comp.is_visible ) {
						client.display_mgr.draw_quad( page.vec_pos + comp.vec_pos, comp.vec_dim, comp.color, uvs );
					}
				}
				
				client.texture_mgr.bind_fonts( );
				auto & map_data = page.get_map_data< PCDTextField >( );
				auto iter_data = map_data.begin( );

				std::lock_guard< std::mutex > lock( mtx_console );

				for( iter_data; iter_data != map_data.end( ); iter_data ++ ) {
					auto & data_text = ( ( Handle< PCDTextField > * ) iter_data->second )->get( );
					glm::ivec2 vect_size_text( data_text.size_text * 2 / 3, data_text.size_text );
					glm::ivec2 vect_pos_char;

					for( int j = 0; j < data_text.ptr_str->size( ); j++ ) {
						auto & uvs = client.texture_mgr.get_uvs_fonts( data_text.ptr_str->at( j ) - 32 );
						vect_pos_char = *data_text.ptr_vec_pos + data_text.vec_offset + glm::ivec2( vect_size_text.x * j, 0 );

						if( vect_pos_char.x > page.vec_dim.x - vect_size_text.x - 5 ) break;

						client.display_mgr.draw_quad(
							page.vec_pos + vect_pos_char,
							vect_size_text,
							Color4( 0.0f, 0.0f, 0.0f, 1.0f ),
							uvs );
					}
				}
			}
		}
	}
}

void GuiMgr::end( ) { }

void GuiMgr::sec( ) { }

void GuiMgr::on_down( int button ) {
	bool is_handled = false;
	int i = list_order.size( ) - 1;

	auto & pos_mouse = client.input_mgr.get_mouse_down( button );

	while( !is_handled && i >= 0 ) {
		auto & page = *list_order[ i ];

		if( page.is_visibile ) {
			if( Directional::is_point_in_rect( pos_mouse, page.vec_pos, page.vec_pos + page.vec_dim ) ) {
				is_handled = list_order[ i ]->on_down( button );

				if( is_handled ) {
					auto temp = list_order[ i ];
					list_order.erase( list_order.begin( ) + i );
					list_order.push_back( temp );
				}
			}
		}

		i--;
	}
}

void GuiMgr::on_hold( int button ) {
	bool is_handled = false;
	int i = list_order.size( ) - 1;

	while( !is_handled && i >= 0 ) {
		auto & page = *list_order[ i ];

		if( page.is_visibile ) {
			is_handled = list_order[ i ]->on_hold( button );
		}

		i--;
	}
}

void GuiMgr::on_up( int button ) {
	bool is_handled = false;
	int i = list_order.size( ) - 1;

	auto & pos_mouse = client.input_mgr.get_mouse_up( button );

	while( !is_handled && i >= 0 ) {
		auto & page = *list_order[ i ];

		if( page.is_visibile ) {
			if( Directional::is_point_in_rect( pos_mouse, page.vec_pos, page.vec_pos + page.vec_dim ) ) {
				is_handled = list_order[ i ]->on_up( button );
			}
		}

		i--;
	}
}

void GuiMgr::add_page( std::string & str_name, FuncPage func_alloc, FuncPage func_custom ) {
	Handle< Page > handle_page;

	if( client.resource_mgr.allocate( handle_page ) ) {
		auto & page = handle_page.get( );
		page.client = &client;
		page.str_name = str_name;

		if( func_alloc( page ) && func_custom( page ) ) {
			page.resize( );
			list_page.push_back( handle_page );
			map_page.insert( { page.str_name, &page } );
			list_order.push_back( &page );
		}
	}
}

Page & GuiMgr::get_page( std::string & str_name ) {
	return *map_page[ str_name ];
}

void GuiMgr::page_show_all( ) {
	for( int i = 0; i < list_order.size(); i++ ) { 
		list_order[ i ]->is_visibile = true;
	}
}

void GuiMgr::print_to_console( std::string const & str_print ) {
	std::string str_console( "Console" );
	auto & page = get_page( str_console );
	auto & data_console = page.get_data< PCDConsole >( str_console );

	std::unique_lock< std::mutex > lock( mtx_console );
	if( data_console.size < PCDConsole::size_max ) {
		data_console.list_strings[ ( data_console.index + data_console.size ) % PCDConsole::size_max ] = str_print;
		data_console.size++;
	}
	else {
		data_console.list_strings[ ( data_console.index + data_console.size ) % PCDConsole::size_max ] = str_print;
		data_console.index++;
		if( data_console.index >= PCDConsole::size_max ) data_console.index -= PCDConsole::size_max;
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
}

void GuiMgr::toggle_input( ) { 
	if( is_input ) {
		auto & page = get_page( std::string( "Console" ) );
		auto & comp_command = page.get_comp( std::string( "Command" ) );
		auto & data_command = page.get_data< PCDCommand >( std::string( "Console" ) );
		auto & data_text = page.get_data< PCDTextField >( std::string( "Command" ) );

		comp_command.color.set( 0.0f, 0.0f, 0.0f, 0.2f );
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

		comp_command.color.set( 0.0f, 0.0f, 0.0f, 0.3f );

		data_command.str_command = "";
		data_command.index_history = data_command.index;

		is_input = true;
	}
}

bool GuiMgr::get_is_input( ) {
	return is_input;
}

void GuiMgr::handle_input_char( int const key, bool const is_down ) { 
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
}

void GuiMgr::process_input( ) { 
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

	pos_start = str_command.find( "/cmd " );
	pos_end = str_command.find( " ", pos_start + 5 );

	if( pos_start != str_command.npos ) {
		pos_start += 5;
		token = str_command.substr( pos_start, pos_end - pos_start );

		if( token == "resetpos" ) {
			auto & pos_camera = client.display_mgr.camera.pos_camera;
			pos_camera = glm::vec3( 0, Chunk::size_y / 2, 0 );
			out << "Command: Resetting position to " << Directional::print_vec( pos_camera );
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
			int id = client.display_mgr.block_select.id_block;
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

				pos_start = str_command.find( "n:" );
				if( pos_start != std::string::npos ) {
					pos_end = str_command.find( " ", pos_start + 2 );
					token = str_command.substr( pos_start + 2, pos_end - ( pos_start + 2 ) );
					pos_start = pos_end;

					while( ( pos_end = str_command.find( " ", pos_start + 1 ) ) < str_command.find( ":", pos_start + 1 ) ) {
						token += str_command.substr( pos_start, ( pos_end - pos_start ) );
						pos_start = pos_end;
					}

					Block * ptr_block = client.chunk_mgr.get_block_data_safe( token );
					if( ptr_block ) {
						id = ptr_block->id;
					}
				}

				out.str( "" );
				out << "Command: Sphere at:" << Directional::print_vec( vec_pos ) << " radius:" << r << " type:" << client.chunk_mgr.get_block_string( id ) << ".";
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
			int id = client.display_mgr.block_select.id_block;
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

				pos_start = str_command.find( "n:" );
				if( pos_start != std::string::npos ) {
					pos_end = str_command.find( " ", pos_start + 2 );
					token = str_command.substr( pos_start + 2, pos_end - ( pos_start + 2 ) );
					pos_start = pos_end;

					while( pos_end != std::string::npos && ( pos_end = str_command.find( " ", pos_start + 1 ) ) < str_command.find( ":", pos_start + 1 )  ) { 
						token += str_command.substr( pos_start, ( pos_end - pos_start ) );
						pos_start = pos_end;
					}

					Block * ptr_block = client.chunk_mgr.get_block_data_safe( token );
					if( ptr_block ) { 
						id = ptr_block->id;
					}
				}

				out.str( "" );
				out << "Command: Rectangle at:" << Directional::print_vec( vec_pos ) << " dim:" << Directional::print_vec( vec_dim ) << " type:" << client.chunk_mgr.get_block_string( id ) << ".";
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
			int id = client.display_mgr.block_select.id_block;
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

				pos_start = str_command.find( "n:" );
				if( pos_start != std::string::npos ) {
					pos_end = str_command.find( " ", pos_start + 2 );
					token = str_command.substr( pos_start + 2, pos_end - ( pos_start + 2 ) );
					pos_start = pos_end;

					while( ( pos_end = str_command.find( " ", pos_start + 1 ) ) < str_command.find( ":", pos_start + 1 ) ) {
						token += str_command.substr( pos_start, ( pos_end - pos_start ) );
						pos_start = pos_end;
					}

					Block * ptr_block = client.chunk_mgr.get_block_data_safe( token );
					if( ptr_block ) {
						id = ptr_block->id;
					}
				}

				out.str( "" );
				out << "Command: Ellipsoid at:" << Directional::print_vec( vec_pos ) << " dim:" << Directional::print_vec( vec_dim ) << " type:" << client.chunk_mgr.get_block_string( id ) << ".";
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
			}

			if( ( iter_map = map.find( list_delim[ 1 ] ) ) != map.end( ) && iter_map->second.size() == 1 ) {
				client.chunk_mgr.set_sun_pause( ( bool ) iter_map->second[ 0 ] );
			}
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

				e.color = glm::vec3( 1.0f, 1.0f, 1.0f );
				iter_map = map.find( list_delim[ 1 ] );
				if( iter_map != map.end( ) && iter_map->second.size( ) == 3 ) { 
					e.color.r = iter_map->second[ 0 ] / 255.0f;
					e.color.g = iter_map->second[ 1 ] / 255.0f;
					e.color.b = iter_map->second[ 2 ] / 255.0f;
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
}

std::unordered_map< std::string, std::vector< int > > get_tokenized_ints( 
	std::string & str_in, std::vector< std::string > & list_delim ) {
	std::unordered_map< std::string, std::vector< int > > map_token;

	std::vector< std::pair< int, int > > list_pair;
	std::vector< std::string > list_token;
	int index;

	for( int i = 0; i < list_delim.size( ); i++ ) {
		index = str_in.find( list_delim[ i ], 0 );
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
				end = list_token[ i ].find( " ", start );
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