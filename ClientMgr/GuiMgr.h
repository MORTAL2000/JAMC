#pragma once

#include "Globals.h"
#include "Manager.h"

#include "Page.h"
#include "BlockSelector.h"

#include <vector>
#include <unordered_map>
#include <string>

class GuiMgr : public Manager {

private:
	static int const size_pages = 100;

	std::mutex mtx_console;

	std::vector< Handle< Page > > list_page;
	std::unordered_map< std::string, Page * > map_page;
	std::vector< Page * > list_order;

	bool is_visible;
	bool is_input;

public:
	GuiMgr( );
	~GuiMgr( );

	BlockSelector block_selector;

private:
	void on_down( int button );
	void on_hold( int button );
	void on_up( int button );

public:
	void init( ) override;
	void update( ) override;
	void render( ) override;
	void end( ) override;
	void sec( ) override;

	void add_page( std::string & str_name, FuncPage func_alloc, FuncPage func_custom );

	Page & get_page( std::string & str_name );

	void page_show_all( );

	void print_to_console( std::string const & str_print );
	void print_to_static( std::string const & str_print );

	void update_static( );
	void clear_static( );

	void toggle_input( );
	bool get_is_input( );
	void handle_input_char( int const key, bool const is_down );
	void process_input( );
};

extern std::unordered_map< std::string, std::vector< int > > get_tokenized_ints( 
	std::string & str_in, std::vector< std::string > & list_delim );