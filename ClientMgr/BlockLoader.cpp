#include "BlockLoader.h"

BlockLoader::~BlockLoader( ) { }

bool BlockLoader::is_visible( BlockLoader const & block ) const { 
	if( !block.is_trans ) {
		return false;
	}

	if( id == block.id ) {
		return false;
	}

	return true;
}