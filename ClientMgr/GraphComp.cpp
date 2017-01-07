#include "GraphComp.h"



GraphComp::GraphComp( Client & client ) { 
	name = "Graph";

	func_alloc = [ &client = client ] ( PComp * comp ) {

		return 0;
	};
}


GraphComp::~GraphComp( ) { }
