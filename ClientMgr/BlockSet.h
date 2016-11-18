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
	int unsigned idx_rle;
	int unsigned idx_max;
	std::vector< short > list_data;

public:
	BlockSet( );
	~BlockSet( );

private:

public:
	void resize( int unsigned x, int unsigned y, int unsigned z );

	inline short get_rle( int unsigned x, int unsigned y, int unsigned z ) {
		short unsigned idx_target = 
			x * vec_size.y * vec_size.z + 
			y * vec_size.z + 
			z;

		if( idx_rle >= idx_target ) return -2;

		int unsigned i = 0;
		int unsigned idx = 0;

		for( i = 0; i < list_rle.size( ); ++i ) { 
			idx += list_rle[ i ].cnt;
		}
	}

	inline void set_rle( int unsigned x, int unsigned y, int unsigned z, short data ) {
		int unsigned idx_target = 
			x * vec_size.y * vec_size.z + 
			y * vec_size.z + z;

		// If we are setting the last index
		if( idx_rle == idx_target ) {
			// If the data is the same as the last run length.
			if( data == list_rle.back( ).id ) { 
				list_rle.back( ).cnt += 1;
			}
			// Else the data is a different run data.
			else { 
				list_rle.push_back( { data, 1 } );
			}

			idx_rle += 1;
			return;
		}
		else if( idx_target > idx_rle ) { 
			idx_target = idx_target - idx_rle;
			list_rle.push_back( { -1, ( short ) idx_target } );
			list_rle.push_back( {  data, 1 } );

			idx_rle += idx_target + 1;
		}

		int unsigned i = 0;
		int unsigned idx = 0;
		int unsigned idx_last = 0;

		while( idx <= idx_target ) { 
			idx += list_rle[ i ].cnt;
			i++;
		}
		
		i--;
		idx_last = idx - list_rle[ i ].cnt;
		idx_target = idx_target - idx_last;
	}

	inline void set_data( int unsigned x, int unsigned y, int unsigned z, short data ) {
		list_data[ x * vec_size.y * vec_size.z + y * vec_size.z + z ] = data;
	}

	inline short get_data( int unsigned x, int unsigned y, int unsigned z ) {
		return list_data[ x * vec_size.y * vec_size.z + y * vec_size.z + z ];
	}

	inline void set_data( glm::ivec3 pos, short data ) {
		list_data[ pos.x * vec_size.y * vec_size.z + pos.y * vec_size.z + pos.z ] = data;
	}

	inline short get_data( glm::ivec3 pos ) {
		return list_data[ pos.x * vec_size.y * vec_size.z + pos.y * vec_size.z + pos.z ];
	}

	void encode( );
	void decode( );

	void clear_data( );
	void clear_rle( );

	void print( );
};

