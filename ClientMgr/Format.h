#pragma once

#include <string>

// General Functions
template< class T, int x >
static inline void fill_array( T( &ref_array )[ x ], T data ) {
	for( int i = 0; i < x; i++ ) {
		ref_array[ i ] = data;
	}
}

template< class T, int x, int y >
static inline void fill_array( T( &ref_array )[ x ][ y ], T data ) {
	for( int i = 0; i < x; i++ ) {
		for( int j = 0; j < y; j++ ) {
			ref_array[ i ][ j ] = data;
		}
	}
}

template< class T, int x, int y, int z >
static inline void fill_array( T( &ref_array )[ x ][ y ][ z ], T data ) {
	for( int i = 0; i < x; i++ ) {
		for( int j = 0; j < y; j++ ) {
			for( int k = 0; k < z; k++ ) {
				ref_array[ i ][ j ][ k ] = data;
			}
		}
	}
}

static inline void clamp( int & x, int a, int b ) {
	x < a ? x = a : ( x > b ? x = b : x );
}

static inline void clamp( float & x, float a, float b ) {
	x < a ? x = a : ( x > b ? x = b : x );
}

// Print formatting
extern inline void printTabbedLine( int num_tabs, char const * data_print );
extern inline void printTabbedLine( int num_tabs, std::string & data_print );
extern inline void printTabbedLine( int num_tabs, std::ostringstream & data_print );