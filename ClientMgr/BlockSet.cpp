#include "BlockSet.h"

#include <iostream>
#include <iomanip>

int unsigned RLERun::count( ) {
	int unsigned size = 0;

	for( int i = 0; i < run.size( ); ++i ) {
		size += run[ i ].cnt;
	}

	return size;
}

int unsigned RLERun::count_id( short id ) {
	int unsigned size = 0;

	for( int i = 0; i < run.size( ); ++i ) {
		if( run[ i ].id == id ) {
			size += run[ i ].cnt;
		}
	}

	return size;
}

short RLERun::get( short index ) {
	int idx_data = 0;
	auto iter = run.begin( );

	while( iter != run.end( ) ) {
		// If we arent in the pair, lets keep walking
		if( index >= idx_data + iter->cnt ) {
			idx_data += iter->cnt;
			iter += 1;

			continue;
		}

		// Else we got an id!
		return iter->id;
	}

	// We got nothing =( lets return null block.
	return -2;
}

void RLERun::set( short index, short id ) {
	short idx_data = 0;
	short idx_run = 0;

	// Lets walk the run
	for( idx_run = 0; idx_run < ( int ) run.size( ); ++idx_run ) {
		// until we find the pair our index is in
		if( index < idx_data + run[ idx_run ].cnt ) {
			break;
		}

		idx_data += run[ idx_run ].cnt;
	}

	if( idx_run == run.size( ) ) {
		if( index == idx_data ) {
			if( run.back( ).id == id ) {
				run.back( ).cnt += 1;
			}
			else {
				run.push_back( { id, 1 } );
			}

			return;
		}
		else if( index > idx_data ) {
			run.push_back( { -2, index - idx_data } );
			run.push_back( { id, 1 } );

			return;
		}
	}

	// We in a pair!

	// If the id is the same, we got nothin to do
	if( id == run[ idx_run ].id ) {
		return;
	}

	// If our pair count is one, merge lef or right pair
	if( run[ idx_run ].cnt == 1 ) {
		if( idx_run > 0 && run[ idx_run - 1 ].id == id ) {
			run[ idx_run ].cnt += run[ idx_run - 1 ].cnt;
			run.erase( run.begin( ) + idx_run - 1 );
			idx_run -= 1;
		}

		if( idx_run < run.size( ) - 1 && run[ idx_run + 1 ].id == id ) {
			run[ idx_run ].cnt += run[ idx_run + 1 ].cnt;
			run.erase( run.begin( ) + idx_run + 1 );
		}

		run[ idx_run ].id = id;
		return;
	}

	// Lets find out if there should be a run left over before and after the set
	short before = index - idx_data;
	short after = idx_data + run[ idx_run ].cnt - index - 1;

	if( before == 0 ) {
		if( idx_run > 0 && run[ idx_run - 1 ].id == id ) {
			run[ idx_run ].cnt -= 1;
			run[ idx_run - 1 ].cnt += 1;
		}
		else {
			run[ idx_run ].cnt -= 1;
			run.insert( run.begin( ) + idx_run, { id, 1 } );
			idx_run += 1;
		}
	}
	else {
		run[ idx_run ].cnt -= before;
		run.insert( run.begin( ) + idx_run, { run[ idx_run ].id, before } );
		idx_run += 1;
		
		if( after > 0 ) {
			run.insert( run.begin( ) + idx_run + 1, { run[ idx_run ].id, after } );
			run[ idx_run ].cnt -= after;
		}

		run[ idx_run ].id = id;
	}
}

void RLERun::clear( ) {
	run.clear( );
	run.shrink_to_fit( );
}

void RLERun::clear_fill( short size, short id ) { 
	run.clear( );
	run.shrink_to_fit( );

	run.emplace_back( RLEPair { id, size } );
}

void RLERun::print( ) {
	std::cout << "[ ";

	auto iter = run.begin( );
	while( iter != run.end( ) ) {
		std::cout << "{ id:" << iter->id << ", cnt:" << iter->cnt << " }";
		if( ( iter + 1 ) != run.end( ) ) {
			std::cout << ", ";
		}

		iter += 1;
	}

	std::cout << " ]" << std::endl;
}