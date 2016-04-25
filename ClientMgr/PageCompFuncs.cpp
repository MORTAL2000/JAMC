#include "PageCompFuncs.h"

#include "Client.h"
#include "Page.h"
#include "PageComp.h"

FuncComp PageCompFuncs::cust_null = [ ] ( PageComp & comp ) { 
	return true; 
};

// Comp Base
FuncComp PageCompFuncs::alloc_base = [ ] ( PageComp & comp ) {
	comp.is_hold = false;
	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::on_down_debug = [ ] ( PageComp & comp ) {
	auto & out = comp.client->display_mgr.out;
	out.str( "" );
	out << "Clicked on component: " << comp.str_name << ", in page: " << comp.parent->str_name;
	comp.client->gui_mgr.print_to_console( out.str( ) );

	return true;
};

// Comp Close
FuncComp PageCompFuncs::alloc_close = [ ] ( PageComp & comp ) { 
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_close;

	comp.color.set( 0.7f, 0.2f, 0.2f, 0.7f );
	comp.vec_dim = glm::ivec2( 24-10, 24-10 );

	comp.vec_anchor_pos = glm::vec2( 1.0f, 1.0f );
	comp.vec_offset_pos = glm::ivec2( 0, 0 ) - comp.vec_dim - glm::ivec2( 5, 5 );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	return true;
};

FuncComp PageCompFuncs::on_down_close = [ ] ( PageComp & comp ) { 
	on_down_debug( comp );

	comp.parent->is_visibile = false;

	return true;
};

// Comp Edit
FuncComp PageCompFuncs::alloc_edit = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_edit;

	comp.parent->add_data< PCDEditable >( std::string( "Edit" ) );
	auto & data_edit = comp.parent->get_data< PCDEditable >( std::string( "Edit" ) );
	data_edit.is_editable = true;

	comp.color.set( 0.2f, 0.7f, 0.2f, 0.7f );
	comp.vec_dim = glm::ivec2( 14, 14 );

	comp.vec_anchor_pos = glm::vec2( 1.0f, 1.0f );
	comp.vec_offset_pos = glm::ivec2( 0, 0 ) - comp.vec_dim - glm::ivec2( 24, 5 );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	return true;
};

FuncComp PageCompFuncs::on_down_edit = [ ] ( PageComp & comp ) {
	on_down_debug( comp );

	auto & data_edit = comp.parent->get_data< PCDEditable >( std::string( "Edit" ) );

	if( data_edit.is_editable ) { 
		data_edit.is_editable = false;

		auto & comp_resize_tl = comp.parent->get_comp( std::string( "RS_TL" ) );
		comp_resize_tl.is_visible = false;
		auto & comp_resize_bl = comp.parent->get_comp( std::string( "RS_BL" ) );
		comp_resize_bl.is_visible = false;
		auto & comp_resize_tr = comp.parent->get_comp( std::string( "RS_TR" ) );
		comp_resize_tr.is_visible = false;
		auto & comp_resize_br = comp.parent->get_comp( std::string( "RS_BR" ) );
		comp_resize_br.is_visible = false;

		auto & comp_resize_l = comp.parent->get_comp( std::string( "RS_L" ) );
		comp_resize_l.is_visible = false;
		auto & comp_resize_r = comp.parent->get_comp( std::string( "RS_R" ) );
		comp_resize_r.is_visible = false;
		auto & comp_resize_t = comp.parent->get_comp( std::string( "RS_T" ) );
		comp_resize_t.is_visible = false;
		auto & comp_resize_b = comp.parent->get_comp( std::string( "RS_B" ) );
		comp_resize_b.is_visible = false;
	}
	else { 
		data_edit.is_editable = true;

		auto & comp_resize_tl = comp.parent->get_comp( std::string( "RS_TL" ) );
		comp_resize_tl.is_visible = true;
		auto & comp_resize_bl = comp.parent->get_comp( std::string( "RS_BL" ) );
		comp_resize_bl.is_visible = true;
		auto & comp_resize_tr = comp.parent->get_comp( std::string( "RS_TR" ) );
		comp_resize_tr.is_visible = true;
		auto & comp_resize_br = comp.parent->get_comp( std::string( "RS_BR" ) );
		comp_resize_br.is_visible = true;

		auto & comp_resize_l = comp.parent->get_comp( std::string( "RS_L" ) );
		comp_resize_l.is_visible = true;
		auto & comp_resize_r = comp.parent->get_comp( std::string( "RS_R" ) );
		comp_resize_r.is_visible = true;
		auto & comp_resize_t = comp.parent->get_comp( std::string( "RS_T" ) );
		comp_resize_t.is_visible = true;
		auto & comp_resize_b = comp.parent->get_comp( std::string( "RS_B" ) );
		comp_resize_b.is_visible = true;
	}

	return true;
};

// Comp Resize
FuncComp PageCompFuncs::alloc_resize_top_left = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_top_left;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 5, 5 );

	comp.vec_anchor_pos = glm::vec2( 0.0f, 1.0f );
	comp.vec_offset_pos = glm::ivec2( 0, -comp.vec_dim.y );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::alloc_resize_bot_left = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_bot_left;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 5, 5 );

	comp.vec_anchor_pos = glm::vec2( 0.0f, 0.0f );
	comp.vec_offset_pos = glm::ivec2( 0, 0 );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::alloc_resize_top_right = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_top_right;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 5, 5 );

	comp.vec_anchor_pos = glm::vec2( 1.0f, 1.0f );
	comp.vec_offset_pos = glm::ivec2( -comp.vec_dim.x, -comp.vec_dim.y );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::alloc_resize_bot_right = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_bot_right;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 5, 5 );

	comp.vec_anchor_pos = glm::vec2( 1.0f, 0.0f );
	comp.vec_offset_pos = glm::ivec2( -comp.vec_dim.x, 0 );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;
	
	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::alloc_resize_left = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_left;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 5, 10 );

	comp.vec_anchor_pos = glm::vec2( 0.0f, 0.5f );
	comp.vec_offset_pos = glm::ivec2( 0, -comp.vec_dim.y / 2 );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::alloc_resize_right = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_right;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 5, 10 );

	comp.vec_anchor_pos = glm::vec2( 1.0f, 0.5f );
	comp.vec_offset_pos = glm::ivec2( -comp.vec_dim.x, -comp.vec_dim.y / 2 );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::alloc_resize_top = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_top;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 10, 5 );

	comp.vec_anchor_pos = glm::vec2( 0.5f, 1.0f );
	comp.vec_offset_pos = glm::ivec2( -comp.vec_dim.x / 2, -comp.vec_dim.y );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::alloc_resize_bot = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_resize;
	comp.func_on_hold = on_hold_resize_bot;
	comp.func_on_up = on_up_resize;

	comp.color.set( 1.0f, 1.0f, 1.0f, 0.3f );
	comp.vec_dim = glm::ivec2( 10, 5 );

	comp.vec_anchor_pos = glm::vec2( 0.5f, 0.0f );
	comp.vec_offset_pos = glm::ivec2( -comp.vec_dim.x / 2, 0 );

	comp.vec_anchor_dim = comp.vec_anchor_pos;
	comp.vec_offset_dim = comp.vec_offset_pos + comp.vec_dim;

	comp.is_visible = true;

	return true;
};

FuncComp PageCompFuncs::on_down_resize = [ ] ( PageComp & comp ) {
	on_down_debug( comp );
	comp.is_hold = true;

	return true;
};

FuncComp PageCompFuncs::on_hold_resize_top_left = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_pos.x += mouse_delta.x;
		comp.parent->vec_dim.x -= mouse_delta.x;

		comp.parent->vec_dim.y += mouse_delta.y;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_hold_resize_bot_left = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_pos.x += mouse_delta.x;
		comp.parent->vec_dim.x -= mouse_delta.x;

		comp.parent->vec_pos.y += mouse_delta.y;
		comp.parent->vec_dim.y -= mouse_delta.y;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_hold_resize_top_right = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_dim.x += mouse_delta.x;

		comp.parent->vec_dim.y += mouse_delta.y;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_hold_resize_bot_right = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_dim.x += mouse_delta.x;

		comp.parent->vec_pos.y += mouse_delta.y;
		comp.parent->vec_dim.y -= mouse_delta.y;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_hold_resize_left = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_pos.x += mouse_delta.x;
		comp.parent->vec_dim.x -= mouse_delta.x;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_hold_resize_right = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_dim.x += mouse_delta.x;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_hold_resize_top = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_dim.y += mouse_delta.y;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_hold_resize_bot = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );

		comp.parent->vec_pos.y += mouse_delta.y;
		comp.parent->vec_dim.y -= mouse_delta.y;

		comp.parent->resize( );

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_up_resize = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) {
		comp.is_hold = false;

		return true;
	}

	return false;
};

// Comp Titlebar
FuncComp PageCompFuncs::alloc_titlebar = [ ] ( PageComp & comp ) { 
	if( !alloc_base( comp ) ) { return false; }

	comp.func_on_down = on_down_titlebar;
	comp.func_on_hold = on_hold_titlebar;
	comp.func_on_up = on_up_titlebar;

	comp.color.set( 0.0f, 0.0f, 0.3f, 0.7f );
	comp.vec_dim = glm::ivec2( 400, 24 );

	comp.vec_anchor_pos = glm::vec2( 0.0f, 1.0f );
	comp.vec_anchor_dim = glm::vec2( 1.0f, 1.0f );

	comp.vec_offset_pos = glm::ivec2( 0, -comp.vec_dim.y );
	comp.vec_offset_dim = glm::ivec2( 0, 0 );

	comp.position( );

	comp.parent->add_data< PCDTextField >( comp.parent->str_name );
	auto & data_text = comp.parent->get_data< PCDTextField >( comp.parent->str_name );

	data_text.size_text = 18;
	data_text.ptr_str = &comp.parent->str_name;
	data_text.ptr_vec_pos = &comp.vec_pos;
	data_text.vec_offset = glm::ivec2( 5, 3 );

	return true;
};

FuncComp PageCompFuncs::on_down_titlebar = [ ] ( PageComp & comp ) {
	on_down_debug( comp );
	comp.is_hold = true;

	return true;
};

FuncComp PageCompFuncs::on_hold_titlebar = [ ] ( PageComp & comp ) {
	auto & data_edit = comp.parent->get_data< PCDEditable >( std::string( "Edit" ) );
	auto & mouse_delta = comp.client->input_mgr.get_mouse_delta( );
	auto mouse_delta_vec = glm::ivec2( mouse_delta.x, mouse_delta.y );

	if( comp.is_hold && data_edit.is_editable ) { 
		comp.parent->vec_pos += mouse_delta_vec;

		return true;
	}

	return false;
};

FuncComp PageCompFuncs::on_up_titlebar = [ ] ( PageComp & comp ) {
	if( comp.is_hold ) { 
		comp.is_hold = false;

		return true;
	}

	return false;
};

// Comp Console
FuncComp PageCompFuncs::alloc_console = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	comp.func_update = update_console;

	auto & data_console = comp.parent->get_data < PCDConsole >( comp.parent->str_name );
	data_console.list_pos.resize( PCDConsole::size_max );
	data_console.list_strings.resize( PCDConsole::size_max );

	return true;
};

FuncComp PageCompFuncs::update_console = [ ] ( PageComp & comp ) {
	auto & data_console = comp.parent->get_data < PCDConsole >( comp.parent->str_name );
	auto & comp_title = comp.parent->get_comp( std::string( "Title" ) );
	auto & comp_command = comp.parent->get_comp( std::string( "Command" ) );

	int num_new_showing  = ( comp.parent->vec_dim.y - comp_title.vec_dim.y - comp_command.vec_dim.y - 15 ) / data_console.size_text;
	clamp( num_new_showing, 0, data_console.size );

	if( num_new_showing > data_console.num_showing ) {
		for( int i = data_console.num_showing; i < num_new_showing && i < data_console.size; i++ ) {
			auto & out = comp.client->display_mgr.out; 
			out.str( "" );
			out << comp.str_name << i;

			comp.parent->add_data< PCDTextField >( out.str( ) );
			auto & data_text = comp.parent->get_data< PCDTextField >( out.str( ) );

			data_text.size_text = data_console.size_text;
			data_console.list_pos[ i ] = glm::ivec2( 5, 10 + comp_command.vec_dim.y + i * data_console.size_text );
			data_text.ptr_str = &data_console.list_strings[ ( data_console.index + data_console.size - 1 - i ) % PCDConsole::size_max ];
			data_text.ptr_vec_pos = &data_console.list_pos[ i ];
		}

		data_console.num_showing = num_new_showing;
	}
	else if( num_new_showing < data_console.num_showing ) {
		for( int i = num_new_showing; i < data_console.num_showing && i < data_console.size; i++ ) {
			auto & out = comp.client->display_mgr.out;
			out.str( "" );
			out << comp.str_name << i;

			comp.parent->remove_data< PCDTextField >( out.str( ) );
		}

		data_console.num_showing = num_new_showing;
	}

	if( data_console.is_dirty ) { 
		for( int i = 0; i < data_console.num_showing; i++ ) {
			auto & out = comp.client->display_mgr.out;
			out.str( "" );
			out << comp.str_name << i;
			auto & data_text = comp.parent->get_data< PCDTextField >( out.str( ) );
			data_text.ptr_str = &data_console.list_strings[ ( data_console.index + data_console.size - 1 - i ) % PCDConsole::size_max ];
		}

		data_console.is_dirty = false;
	}

	return true;
};

// Comp Command
FuncComp PageCompFuncs::alloc_command = [ ] ( PageComp & comp ) { 
	if( !alloc_base( comp ) ) { return false; }

	comp.func_update = PageCompFuncs::update_command;
	comp.func_on_down = PageCompFuncs::on_down_command;

	auto & data_command = comp.parent->get_data < PCDCommand >( comp.parent->str_name );
	data_command.size_text = 12;
	data_command.list_strings.resize( PCDCommand::size_max );

	comp.parent->add_data< PCDTextField >( comp.str_name );
	auto & data_text = comp.parent->get_data< PCDTextField >( comp.str_name );
	data_text.size_text = data_command.size_text;
	data_text.ptr_str = &data_command.str_command;
	data_text.ptr_vec_pos = &data_command.vect_pos;

	comp.color.set( 0.0f, 0.0f, 0.0f, 0.2f );

	comp.vec_dim = glm::ivec2( 0, data_command.size_text + 6 );

	comp.vec_anchor_pos = glm::vec2( 0.0f, 0.0f );
	comp.vec_anchor_dim = glm::vec2( 1.0f, 0.0f );
	comp.vec_offset_pos = glm::ivec2( 5, 5 );
	comp.vec_offset_dim = glm::ivec2( -5, comp.vec_dim.y + 5 );

	comp.position( );

	data_command.vect_pos = comp.vec_pos + glm::ivec2( 3, 3 );

	return true;
};

FuncComp PageCompFuncs::update_command = [ ] ( PageComp & comp ) { 
	return true;
};

FuncComp PageCompFuncs::on_down_command = [ ] ( PageComp & comp ) { 
	on_down_debug( comp );

	comp.client->gui_mgr.toggle_input( );

	return true;
};

// Comp Static
FuncComp PageCompFuncs::alloc_static = [ ] ( PageComp & comp ) {
	if( !alloc_base( comp ) ) { return false; }

	//comp.func_update = update_text_static;

	auto & data_static = comp.parent->get_data < PCDStatic >( comp.parent->str_name );
	data_static.list_pos.resize( PCDStatic::size_max );
	data_static.list_strings.resize( PCDStatic::size_max );

	return true;
};

FuncComp PageCompFuncs::update_static = [ ] ( PageComp & comp ) {
	auto & data_static = comp.parent->get_data < PCDStatic >( comp.parent->str_name );
	int size_resize = comp.parent->get_comp( std::string( "Title" ) ).vec_dim.y + 10 + data_static.size_text * data_static.size;
	if( comp.parent->vec_dim.y < size_resize ) { 
		comp.parent->vec_pos.y -= size_resize - comp.parent->vec_dim.y;
		comp.parent->vec_dim.y = size_resize; 
		comp.parent->resize( );
	}

	int num_new_showing = ( comp.parent->vec_dim.y - comp.parent->get_comp( std::string( "Title" ) ).vec_dim.y - 10 ) / data_static.size_text;
	clamp( num_new_showing, 0, data_static.size );

	if( num_new_showing > data_static.num_showing ) {
		for( int i = data_static.num_showing; i < num_new_showing; i++ ) {
			auto & out = comp.client->display_mgr.out;
			out.str( "" );
			out << comp.str_name << i;

			comp.parent->add_data< PCDTextField >( out.str( ) );
			auto & data_text = comp.parent->get_data< PCDTextField >( out.str( ) );

			data_text.size_text = data_static.size_text;
			data_static.list_pos[ i ] = glm::ivec2( 5, 5 + i * data_static.size_text );
			data_text.ptr_str = &data_static.list_strings[ ( data_static.index + data_static.size - 1 - i ) % PCDStatic::size_max ];
			data_text.ptr_vec_pos = &data_static.list_pos[ i ];
		}

		data_static.num_showing = num_new_showing;
	}
	else if( num_new_showing < data_static.num_showing ) {
		for( int i = data_static.num_showing - 1; i >= num_new_showing; i-- ) {
			auto & out = comp.client->display_mgr.out;
			out.str( "" );
			out << comp.str_name << i;

			comp.parent->remove_data< PCDTextField >( out.str( ) );
		}

		data_static.num_showing = num_new_showing;
	}

	if( data_static.is_dirty ) {
		for( int i = 0; i < data_static.num_showing; i++ ) {
			auto & out = comp.client->display_mgr.out;
			out.str( "" );
			out << comp.str_name << i;
			auto & data_text = comp.parent->get_data< PCDTextField >( out.str( ) );
			data_text.ptr_str = &data_static.list_strings[ ( data_static.index + data_static.size - 1 - i ) % PCDStatic::size_max ];
		}

		data_static.is_dirty = false;
	}

	return true;
};