#include "BlockSet.h"

#include <iostream>

BlockSet::BlockSet( ) {
	//std::cout << "Sizeof vec size: " << sizeof( vec_size ) << std::endl;
	//std::cout << "Sizeof list data: " << sizeof( list_data ) << std::endl;
	//std::cout << "Sizeof list rle: " << sizeof( list_rle ) << std::endl;
	//std::cout << "Sizeof this: " << sizeof( *this ) << std::endl;
}

BlockSet::~BlockSet( ) { }

void BlockSet::resize( int unsigned x, int unsigned y, int unsigned z ) { 
	vec_size.x = x;
	vec_size.y = y;
	vec_size.z = z;

	idx_rle = 0;
	idx_max = vec_size.x * vec_size.y * vec_size.z;

	list_data.resize( vec_size.x * vec_size.y * vec_size.z );
	list_data.shrink_to_fit( );
}

void BlockSet::encode( ) {
	if( list_data.size( ) != vec_size.x * vec_size.y * vec_size.z ) { 
		printf( "Error! While encoding, list_data does not match BlockSet Size!" );
		return; 
	}

	short id_curr = list_data[ 0 ];
	short id_last = list_data[ 0 ];
	short cnt = 0;

	list_rle.clear( );

	for( int unsigned j = 0; j < vec_size.y; ++j ) {
		for( int unsigned i = 0; i < vec_size.x; ++i ) {
			for( int unsigned k = 0; k < vec_size.z; ++k ) {
				id_curr = get_data( i, j, k );

				if( id_last == id_curr ) {
					++cnt;
				}
				else {
					list_rle.push_back( { id_last, cnt } );
					id_last = id_curr;
					cnt = 1;
				}
			}
		}
	}

	if( cnt != 0 ) { 
		list_rle.push_back( { id_last, cnt } );
	}
}

void BlockSet::decode( ) {
	short index = 0;
	RLEPair pair_rle;
	glm::ivec3 pos;

	list_data.clear( );
	resize( vec_size.x, vec_size.y, vec_size.z );

	for( int unsigned i = 0; i < list_rle.size( ); ++i ) {
		pair_rle = list_rle[ i ];

		for( int unsigned j = 0; j < pair_rle.cnt; ++j ) {
			pos.x = ( index % ( vec_size.z * vec_size.x ) ) / vec_size.z;
			pos.y = index / ( vec_size.z * vec_size.x );
			pos.z = index % vec_size.z;

			set_data( pos, pair_rle.id );
			++index;
		}
	}
}

void BlockSet::clear_data( ) { 
	list_data.clear( );
	list_data.shrink_to_fit( );
}

void BlockSet::clear_rle( ) {
	list_rle.clear( );
	list_rle.shrink_to_fit( );
}

void BlockSet::print( ) {
	if( !list_rle.empty( ) ) {
		for( int unsigned i = 0; i < list_rle.size( ); ++i ) {
			std::cout << "[ " << list_rle[ i ].id << ", " << list_rle[ i ].cnt << " ]" << std::endl;
		}

		std::cout << std::endl;
	}

	if( !list_data.empty( ) ) {
		for( int unsigned j = 0; j < vec_size.y; ++j ) {                                                   
			for( int unsigned i = 0; i < vec_size.x; ++i ) {
				for( int unsigned k = 0; k < vec_size.z; ++k ) {
					std::cout << get_data( i, j, k ) << " ";
				}

				std::cout << std::endl;
			}

			std::cout << std::endl;
		}
	}

	std::cout << "Size: class: " 
		<< sizeof( BlockSet ) 
		<< " rle: " << sizeof( RLEPair ) * list_rle.size( ) 
		<< " data: " << sizeof( short ) * list_data.size( ) 
		<< std::endl << std::endl;
}