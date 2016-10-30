#pragma once

#include <vector>

#include "glm\glm.hpp"

class BlockSet {
private:
	struct RLEPair { 
		short id;
		short cnt;
	};

private:
	glm::ivec3 vec_size;
	std::vector< RLEPair > list_rle;
	std::vector< short > list_data;

public:
	BlockSet( );
	~BlockSet( );

private:

public:
	void resize( int unsigned x, int unsigned y, int unsigned z );

	inline void set( int unsigned x, int unsigned y, int unsigned z, short data ) {
		list_data[ x * vec_size.y * vec_size.z + y * vec_size.z + z ] = data;
	}

	inline short get( int unsigned x, int unsigned y, int unsigned z ) {
		return list_data[ x * vec_size.y * vec_size.z + y * vec_size.z + z ];
	}

	inline void set( glm::ivec3 pos, short data ) {
		list_data[ pos.x * vec_size.y * vec_size.z + pos.y * vec_size.z + pos.z ] = data;
	}

	inline short get( glm::ivec3 pos ) {
		return list_data[ pos.x * vec_size.y * vec_size.z + pos.y * vec_size.z + pos.z ];
	}

	void encode( );
	void decode( );

	void clear_data( );
	void clear_rle( );

	void print( );
};

