#include "Block.h"
#include "TextureMgr.h"

Block::~Block( ) { }

bool Block::is_visible( Block const & block ) { 
	if( !block.is_trans ) {
		return false;
	}

	if( id == block.id ) {
		return false;
	}

	return true;
}