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

	bool is_visible;
	bool is_input;

	std::mutex mtx_console;

	std::vector< PCLoader > list_page_component_loaders;
	std::unordered_map< std::string, int > map_page_component_loaders;

	std::vector< PageLoader > list_page_loaders;
	std::unordered_map< std::string, int > map_page_loaders;

	std::vector< Handle< Page > > list_pages;
	std::unordered_map< std::string, int > map_pages;
	std::list< int > list_order;

public:
	BlockSelector block_selector;

	GuiMgr( Client & client );
	~GuiMgr( );

private:
	void load_components( );
	void add_component_loader( PCLoader & pc_loader );


	void load_pages( );
	void add_page_loader( PageLoader & page_loader );


	void add_page(
		std::string const & name,
		std::string const & name_loader,
		PageFunc func_custom );

	void on_down( int button );
	void on_hold( int button );
	void on_up( int button );

public:
	void init( ) override;
	void update( ) override;
	void render( ) override;
	void end( ) override;
	void sec( ) override;

	void update_comps( Page * page );
	void update_comps( PComp * comp );

	void mesh_comps( Page * page );

	PCLoader * get_component_loader( std::string const & name_loader );
	PCLoader * get_component_loader_safe( std::string const & name_loader );

	PageLoader * get_page_loader( std::string const & name_loader );
	PageLoader * get_page_loader_safe( std::string const & name_loader );

	void toggle_input( );
	bool get_is_input( );
	void handle_input_char( int const key, bool const is_down );
	void process_input( );

};

extern std::unordered_map< std::string, std::vector< int > > 
	get_tokenized_ints( std::string & str_in, std::vector< std::string > & list_delim );