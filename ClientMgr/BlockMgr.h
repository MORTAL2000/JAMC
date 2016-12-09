#pragma once

#include "Globals.h"
#include "Manager.h"
#include "BlockLoader.h"

#include <vector>
#include <unordered_map>

class BlockMgr : public Manager {
private:
	std::vector< BlockLoader > list_block_loader;
	std::unordered_map< std::string, int > map_block_loader;

public:
	BlockMgr( );
	~BlockMgr( );

public:	
	void init( ) override;
	void update( ) override;
	void render( ) override;
	void end( ) override;
	void sec( ) override;

	void load_block_data( );
	void load_block_mesh( );

	BlockLoader * get_block_loader( int const id );
	BlockLoader * get_block_loader_safe( int const id );

	BlockLoader * get_block_loader( std::string const & name );
	BlockLoader * get_block_loader_safe( std::string const & name );

	std::string const & get_block_string( int const id );

	int get_num_blocks( );


};

