#include "Globals.h"

#include "Client.h"

#include <iostream>
#include <sstream>

int get_refresh( ) {
	DEVMODE dev_mode;
	EnumDisplaySettings( nullptr, ENUM_CURRENT_SETTINGS, &dev_mode );
	return dev_mode.dmDisplayFrequency;
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	Client* winptr = ( Client* ) GetWindowLongPtr( hwnd, GWLP_USERDATA );

	if( winptr == NULL ) {
		return DefWindowProc( hwnd, message, wParam, lParam );
	}
	else {
		return winptr->WndProc( hwnd, message, wParam, lParam );
	}
}

inline std::string getGLErrorString( int error ) {
	switch( error ) {
		case GL_NO_ERROR:
		return "NO GLErrors!";
		case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM!";
		case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION!";
		case GL_STACK_OVERFLOW:
		return "GL_STACK_downFLOW!";
		case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW!";
		case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY!";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION!";
		case GL_TABLE_TOO_LARGE:
		return "GL_TABLE_TOO_LARGE!";
		default:
		return "UNKNOWN ERROR!";
	}
}

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
	std::cout << data_print.str() << std::endl;
}