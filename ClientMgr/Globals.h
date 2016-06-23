#pragma once

// Static Libraries
#pragma comment( lib, "glew/lib/Release/x64/glew32" )
#pragma comment( lib, "glut/x64/glut32" )
#pragma comment( lib, "soil/lib/SOIL" )

// General Includes
#include <stdlib.h>
#include <string>

// Soil Includes
#include "soil\src\SOIL.h"

// OpenGL Includes
#include "glew\include\GL\glew.h"
#include "glew\include\GL\wglew.h"
#include "glut\glut.h"

// Forward Declarations
class Runnable;
class Manager;

class Client;
class ClientMgr;

class AbstractHandle;
template< class T > class Handle;
class Pool;
template< class T > class ResPool;

template< class T > class Vect2;
template< class T > class Vect3;

class Block;

class Chunk;
struct ChunkNoise;
struct ChunkFile;
class Color4;

class Entity;

struct quad_uvs;

class Page;
class PageComp;

extern int get_refresh( );

// Static Variables
#define PI 3.14159265f
#define BUFFER_OFFSET(i) ((void*)(i))

static const float UPDATE_RATE = get_refresh( );
static const float TIME_MILLISEC = 1000.0f;
static const float DELTA_CORRECT = 1.0f / UPDATE_RATE;
static const float TIME_FRAME_MILLI = TIME_MILLISEC / UPDATE_RATE;

// Windows Proc
LRESULT CALLBACK WndProc( HWND p_hWnd, UINT p_uiMessage, WPARAM p_wParam, LPARAM p_lParam );

// General Functions
template< class T, int x >
static inline void fill_array( T ( & ref_array )[ x ], T data ) {
	for( int i = 0; i < x; i++ ) {
		ref_array[ i ] = data;
	}
}

template< class T, int x, int y >
static inline void fill_array( T ( & ref_array )[ x ][ y ], T data ) {
	for( int i = 0; i < x; i++ ) {
		for( int j = 0; j < y; j++ ) {
			ref_array[ i ][ j ] = data;
		}
	}
}

template< class T, int x, int y, int z >
static inline void fill_array( T ( & ref_array )[ x ][ y ][ z ], T data ) {
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

// Opengl functions
extern inline std::string checkGlErrors( );

// Print formatting
extern inline void printTabbedLine( int num_tabs, char const * data_print );
extern inline void printTabbedLine( int num_tabs, std::string & data_print );
extern inline void printTabbedLine( int num_tabs, std::ostringstream & data_print );