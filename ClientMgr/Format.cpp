#include "Format.h"

#include <iostream>
#include <sstream>

inline void printTabbedLine( int num_tabs, char const * data_print ) {
	for( int i = 0; i < num_tabs; i++ ) {
		std::cout << "\t";
	}
	std::cout << data_print << std::endl;
}

inline void printTabbedLine( int num_tabs, std::string & data_print ) {
	for( int i = 0; i < num_tabs; i++ ) {
		std::cout << "\t";
	}
	std::cout << data_print << std::endl;
}

inline void printTabbedLine( int num_tabs, std::ostringstream & data_print ) {
	for( int i = 0; i < num_tabs; i++ ) {
		std::cout << "\t";
	}
	std::cout << data_print.str( ) << std::endl;
}