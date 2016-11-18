#pragma once

#include <vector>
#include <iostream>

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

		if( idx_target >= idx_rle ) return -2;

		int unsigned i = 0;
		int unsigned idx = 0;

		for( i = 0; i < list_rle.size( ); ++i ) {
			idx += list_rle[ i ].cnt;
		}
	}

	// Case 1: Larger than allocated index.
	// Case 2: Larger than the current index.
	// Case 3: Exactly at the current index.
	// Case 4: Splitting some indexed run length.
	inline void set_rle( int unsigned x, int unsigned y, int unsigned z, short data ) {
		int unsigned idx_target =
			x * vec_size.y * vec_size.z +
			y * vec_size.z + z;

		// If target index is larger than allocated index.
		if( idx_target > idx_max ) return;

		// If target index is at current index.
		if( idx_rle == idx_target ) {
			//std::cout << "Add at curr idx" << std::endl;
			// If the data is the same as the last run length.
			if( !list_rle.empty( ) && data == list_rle.back( ).id ) {
				list_rle.back( ).cnt += 1;
			}
			// Else the data is a different run data.
			else {
				list_rle.push_back( { data, 1 } );
			}

			idx_rle += 1;

			return;
		}
		// If target index is after the current index.
		else if( idx_target > idx_rle ) {
			//std::cout << "Add after curr idx" << std::endl;

			idx_target = idx_target - idx_rle;
			list_rle.push_back( { -1, ( short ) idx_target } );
			list_rle.push_back( { data, 1 } );

			idx_rle = idx_target + 1;

			return;
		}


		// If the target index is within an existing run length
		int unsigned i = 0;
		int unsigned idx = 0;
		int unsigned idx_last = 0;
		//std::cout << "Add in run" << std::endl;

		while( idx <= idx_target ) {
			idx += list_rle[ i ].cnt;
			i++;
		}

		i--;
		idx_last = idx - list_rle[ i ].cnt;

		if( idx_target == idx_last ) {
			//std::cout << "start of run" << std::endl;
			list_rle.insert( list_rle.begin( ) + i, { data, 1 } );

			list_rle[ i + 1 ].cnt--;
			if( !list_rle[ i + 1 ].cnt ) {
				list_rle.erase( list_rle.begin( ) + i + 1 );
			}
		}
		else if( idx_target == idx - 1 ) {
			//std::cout << "end of run" << std::endl;
			list_rle[ i ].cnt--;
			list_rle.insert( list_rle.begin( ) + i + 1, { data, 1 } );
		}
		else {
			//std::cout << "middle of run" << std::endl;
			list_rle[ i ].cnt = idx_target - idx_last;
			list_rle.insert( list_rle.begin( ) + i + 1, { data, 1 } );
			list_rle.insert( list_rle.begin( ) + i + 2, { list_rle[ i ].id, ( short ) ( idx - idx_target ) } );
		}
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

