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

	bool is_shift;

	PComp * comp_over;
	PComp * comp_over_last;

	PComp * comp_down;
	PComp * comp_down_last;

	PComp * comp_selected;

	std::vector< PCLoader > list_page_component_loaders;
	std::unordered_map< std::string, int > map_page_component_loaders;

	std::vector< PageLoader > list_page_loaders;
	std::unordered_map< std::string, int > map_page_loaders;

	std::unordered_map< std::string, Handle< Page > > map_pages;
	std::list< Handle< Page > > list_order;
	std::queue< std::string > queue_remove;

public:
	std::recursive_mutex mtx_console;

	BlockSelector block_selector;

	GuiMgr( Client & client );
	~GuiMgr( );

private:
	void load_components( );
	void add_component_loader( PCLoader & pc_loader );

	void load_pages( );
	void add_page_loader( PageLoader & page_loader );
	void process_remove( );

	void on_over( );

	void over_page_all( );
	void over_page( Page * page );
	void over_page_self( Page * page );
	void over_page_children( Page * page );

	void over_comp( PComp * comp );
	void over_comp_self( PComp * comp );
	void over_comp_children( PComp * comp );

	void on_click( );

	void down_page_all( );
	void down_page( Page * page );
	void down_page_self ( Page * page );
	void down_page_children( Page * page );

	void down_comp( PComp * comp );
	void down_comp_self( PComp * comp );
	void down_comp_children( PComp * comp );

	void remove_comp( PComp * comp );
	void remove_comp_self( PComp * comp );
	void remove_comp_children( PComp * comp );

	PComp * get_named_parent( PComp * comp, std::string const & name );

public:
	void init( ) override;
	void update( ) override;
	void render( ) override;
	void end( ) override;
	void sec( ) override;

	void update_page( Page * page );
	void update_comp( PComp * comp );
	void update_comp_self( PComp * comp );
	void update_comp_children( PComp * comp );

	void mesh_page( Page * page );
	void mesh_comp( PComp * comp );

	PCLoader * get_component_loader( std::string const & name_loader );
	PCLoader * get_component_loader_safe( std::string const & name_loader );

	PageLoader * get_page_loader( std::string const & name_loader );
	PageLoader * get_page_loader_safe( std::string const & name_loader );

	Page * add_page( std::string const & name, std::string const & name_loader, PageFunc func_custom );
	Page * get_page( std::string const & name );
	Page * get_page_safe( std::string const & name );

	void remove_page( std::string const & name_page );

	void toggle_input( );
	bool get_is_input( );

	void handle_char( char const c );
	void handle_vkey( int const key, bool const is_down );

	void process_input( );

	void add_statistics_entry( std::function< std::string( ) > func_entry );
	void print_to_console( std::string const & str_out );
};

extern std::unordered_map< std::string, std::vector< int > > 
	get_tokenized_ints( std::string & str_in, std::vector< std::string > & list_delim );