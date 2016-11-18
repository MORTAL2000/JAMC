#include "ClientMgr.h"

#include "BlockSet.h"

ClientMgr::ClientMgr(){}

ClientMgr::~ClientMgr(){}

void ClientMgr::run() {
	
	std::cout << "*** Init ***" << std::endl;

	BlockSet bset;

	bset.print( );

	std::cout << "*** Resize ***" << std::endl;

	bset.resize( 8, 8, 8 );

	bset.print( );

	std::cout << "*** Set Data ***" << std::endl;

	for( int unsigned i = 0; i < 8; ++i ) {
		for( int unsigned j = 0; j < 8; ++j ) {
			for( int unsigned k = 0; k < 8; ++k ) {
				if( j % 2 == 0 ) { 
					bset.set_data( i, j, k, 1 );
				}
				else {
					//if( i % 2 == 0 & k % 2 == 0 ) {
					//	bset.set( i, j, k, 1 );
					//}
					//else { 
						bset.set_data( i, j, k, 0 );
					//}
				}
			}
		}
	}

	bset.print( );

	std::cout << "*** Encode ***" << std::endl;

	bset.encode( );

	bset.print( );

	std::cout << "*** Clear Data ***" << std::endl;

	bset.clear_data( );

	bset.print( );

	std::cout << "*** Decode ***" << std::endl;

	bset.decode( );

	bset.print( );

	std::cout << "*** Clear RLE ***" << std::endl;

	bset.clear_rle( );

	bset.print( );

	std::cout << "*** Clear Data ***" << std::endl;

	bset.clear_data( );

	bset.print( );

	system( "PAUSE" );
	

	client.init();
	client.end();
}