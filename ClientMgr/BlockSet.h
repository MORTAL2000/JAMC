#pragma once

#include <vector>
#include <mutex>

struct RLEPair {
	short id;
	short cnt;
};

struct RLERun {
	std::vector< RLEPair > run;

	int unsigned count( );
	int unsigned count_id( short id );

	short get( short index );
	void set( short index, short id );

	void clear( );
	void clear_fill( short size, short id );

	void print( );

	int unsigned size_bytes( );
};

template< int unsigned t_x, int unsigned t_y, int unsigned t_z >
struct BlockRegion { 
	typedef std::array< std::array< std::array< short, t_y >, t_z >, t_x > BlockData;

	BlockData blocks;

	BlockRegion( ) :
		blocks( ) { 
		for( int unsigned i = 0; i < t_x; ++i ) {
			for( int unsigned j = 0; j < t_y; ++j ) {
				for( int unsigned k = 0; k < t_z; ++k ) {
					blocks[ i ][ k ][ j ] = -2;
				}
			}
		}
	}

	short get( int unsigned x, int unsigned y, int unsigned z ) { 
		return blocks[ x ][ z ][ y ];
	}

	void set( int unsigned x, int unsigned y, int unsigned z, short id ) { 
		blocks[ x ][ z ][ y ] = id;
	}
};

template< int unsigned t_x, int unsigned t_z >
struct BlockSet {
	std::array< std::array< RLERun, t_z >, t_x > mat_runs;
	std::array< std::mutex, t_x > mat_mutex;
	//std::mutex mat_mutex;

	void clear( ) { 
		for( int unsigned i = 0; i < t_x; ++i ) {
			//std::lock_guard< std::mutex > lock( mat_mutex );
			std::lock_guard< std::mutex > lock( mat_mutex[ i ] );
			for( int unsigned j = 0; j < t_z; ++j ) {
				mat_runs[ i ][ j ].clear( );
			}
		}
	};
	
	void clear_fill( int unsigned size, short id ) { 
		//std::lock_guard< std::mutex > lock( mat_mutex );
		for( int unsigned i = 0; i < t_x; ++i ) {
			std::lock_guard< std::mutex > lock( mat_mutex[ i ] );
			for( int unsigned j = 0; j < t_z; ++j ) {
				mat_runs[ i ][ j ].clear_fill( size, id );
			}
		}
	};

	void set( int unsigned x, int unsigned y, int unsigned z, short id ) { 
		//std::lock_guard< std::mutex > lock( mat_mutex );
		std::lock_guard< std::mutex > lock( mat_mutex[ x ] );
		mat_runs[ x ][ z ].set( y, id );
	};

	short get( int unsigned x, int unsigned y, int unsigned z ) {
		//std::lock_guard< std::mutex > lock( mat_mutex );
		std::lock_guard< std::mutex > lock( mat_mutex[ x ] );
		return mat_runs[ x ][ z ].get( y );
	};

	template< int unsigned t_y >
	void set_data( BlockRegion< t_x, t_y, t_z > & region ) {
		//std::lock_guard< std::mutex > lock( mat_mutex );
		for( int unsigned i = 0; i < t_x; ++i ) {
			std::lock_guard< std::mutex > lock( mat_mutex[ i ] );

			for( int unsigned j = 0; j < t_z; ++j ) {
				mat_runs[ i ][ j ].run.clear( );

				short id = -2;
				int unsigned cnt = 0;
				for( int unsigned k = 0; k < t_y; ++k ) {
					if( region.get( i, k, j ) == id ) {
						++cnt;
					}
					else {
						if( cnt != 0 ) {
							mat_runs[ i ][ j ].run.push_back( { id, ( short ) cnt } );
						}
						id = region.get( i, k, j );
						cnt = 1;
					}
				}

				if( cnt != 0 ) {
					mat_runs[ i ][ j ].run.push_back( { id, ( short ) cnt } );
				}
			}
		}
	};

	template< int unsigned t_y >
	void get_data( BlockRegion< t_x, t_y, t_z > & region ) {
		//std::lock_guard< std::mutex > lock( mat_mutex );
		for( int unsigned i = 0; i < t_x; ++i ) {
			std::lock_guard< std::mutex > lock( mat_mutex[ i ] );

			for( int unsigned j = 0; j < t_z; ++j ) {

				int unsigned k = 0;
				for( auto & pair : mat_runs[ i ][ j ].run ) {
					if( k + pair.cnt >= t_y ) {
						for( int unsigned l = k; l < t_y; ++l ) {
							region.set( i, l, j, pair.id );
						}
						break;
					}

					for( int unsigned l = k; l < k + pair.cnt; ++l ) {
						region.set( i, l, j, pair.id );
					}

					k += pair.cnt;
				}
			}
		}
	};

	int unsigned size_bytes() {
		int unsigned size = sizeof( BlockSet );

		for (int i = 0; i < t_x; ++i) {
			for (int k = 0; k < t_z; ++k) {
				size += mat_runs[i][k].size_bytes();
			}
		}

		return size;
	}
};