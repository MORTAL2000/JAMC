#include "PageFuncs.h"

#include "Client.h"
#include "Page.h"
#include "PageComp.h"

FuncPage PageFuncs::cust_null = [ ] ( Page & page ) { return true; };

FuncPage PageFuncs::alloc_base = [ ] ( Page & page ) {
	page.color.set( 0.3f, 0.3f, 0.3f, 0.9f );
	page.vec_pos = glm::ivec2( 0, 0 );
	page.vec_dim = glm::ivec2( 400, 200 );

	page.add_comp(
		std::string( "Title" ),
		PageCompFuncs::alloc_titlebar,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "Close" ),
		PageCompFuncs::alloc_close,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "Edit" ),
		PageCompFuncs::alloc_edit,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_TL" ),
		PageCompFuncs::alloc_resize_top_left,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_BL" ),
		PageCompFuncs::alloc_resize_bot_left,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_TR" ),
		PageCompFuncs::alloc_resize_top_right,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_BR" ),
		PageCompFuncs::alloc_resize_bot_right,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_L" ),
		PageCompFuncs::alloc_resize_left,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_R" ),
		PageCompFuncs::alloc_resize_right,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_T" ),
		PageCompFuncs::alloc_resize_top,
		PageCompFuncs::cust_null
		);

	page.add_comp(
		std::string( "RS_B" ),
		PageCompFuncs::alloc_resize_bot,
		PageCompFuncs::cust_null
		);

	return true;
};

FuncPage PageFuncs::alloc_test = [ ]( Page & page ) {
	if( !alloc_base( page ) ) { return false; }

	return true;
};

FuncPage PageFuncs::alloc_console = [ ] ( Page & page ) {
	if( !alloc_base( page ) ) { return false; }

	page.add_data< PCDConsole >( page.str_name );
	auto & data_console = page.get_data < PCDConsole >( page.str_name );

	data_console.num_showing = 0;
	data_console.size_text = 12;
	data_console.list_strings.reserve( PCDConsole::size_max );
	data_console.list_pos.reserve( PCDConsole::size_max );

	page.add_comp(
		std::string( "Console" ),
		PageCompFuncs::alloc_console,
		PageCompFuncs::cust_null
		);

	page.add_data< PCDCommand >( page.str_name );
	auto & data_command = page.get_data < PCDCommand >( page.str_name );

	data_command.index = 0;
	data_command.size_text = 12;
	data_command.list_strings.reserve( PCDCommand::size_max );

	page.add_comp(
		std::string( "Command" ),
		PageCompFuncs::alloc_command,
		PageCompFuncs::cust_null
		);

	page.vec_dim.x = 650;
	page.vec_dim.y = 215;

	return true;
};

FuncPage PageFuncs::alloc_static = [ ] ( Page & page ) {
	if( !alloc_base( page ) ) { return false; }

	page.add_data< PCDStatic >( page.str_name );
	auto & data_static = page.get_data < PCDStatic >( page.str_name );

	data_static.num_showing = 0;
	data_static.size_text = 12;
	data_static.list_strings.reserve( PCDStatic::size_max );
	data_static.list_pos.reserve( PCDStatic::size_max );

	page.add_comp(
		std::string( "Static" ),
		PageCompFuncs::alloc_static,
		PageCompFuncs::cust_null
		);

	auto & comp_title = page.get_comp( std::string( "Title" ) );

	page.vec_dim.x = 650;
	page.vec_dim.y = 9 * data_static.size_text + comp_title.vec_dim.y + 10;

	page.vec_pos.y = page.client->display_mgr.get_window( ).y - page.vec_dim.y;

	return true;
};