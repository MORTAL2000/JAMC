#pragma once

// Static Libraries
#pragma comment( lib, "glew/lib/Release/x64/glew32" )
#pragma comment( lib, "glut/x64/glut32" )
#pragma comment( lib, "soil/lib/SOIL" )

#ifdef NDEBUG
#pragma comment( lib, "tinyxml2" )

#else
#pragma comment( lib, "tinyxml2_debug" )

#endif

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
class Manager;

class AbstractHandle;
template< class T > class Handle;
class Pool;
template< class T > class ResPool;

class Client;

class Chunk;
struct ChunkNoise;
struct ChunkFile;

class Entity;

struct quad_uvs;

class Page;
class PageComp;

extern int get_refresh( );

// Static Variables
#define PI 3.14159265f
#define BUFFER_OFFSET(i) ((void*)(i))


#define FOR3D( F3DX, F3DXSTART, F3DXEND, F3DY, F3DYSTART, F3DYEND, F3DZ, F3DZSTART, F3DZEND ) for( F3DX = F3DXSTART; F3DX < F3DXEND; ++F3DX ) { for( F3DY = F3DYSTART; F3DY < F3DYEND; ++F3DY ) { for( F3DZ = F3DZSTART; F3DZ < F3DZEND; ++F3DZ ) {
#define FOR3DEND } } }

constexpr float UPDATE_RATE = 60;
constexpr float TIME_MILLISEC = 1000.0f;
constexpr float DELTA_CORRECT = 1.0f / UPDATE_RATE;
constexpr float TIME_FRAME_MILLI = TIME_MILLISEC / UPDATE_RATE;

// Windows Proc
LRESULT CALLBACK WndProc( HWND p_hWnd, UINT p_uiMessage, WPARAM p_wParam, LPARAM p_lParam );

// General Functions
template< class T, int x >
inline void fill_array( T ( & ref_array )[ x ], T data ) {
	for( int i = 0; i < x; i++ ) {
		ref_array[ i ] = data;
	}
}

template< class T, int x, int y >
inline void fill_array( T ( & ref_array )[ x ][ y ], T data ) {
	for( int i = 0; i < x; i++ ) {
		for( int j = 0; j < y; j++ ) {
			ref_array[ i ][ j ] = data;
		}
	}
}

template< class T, int x, int y, int z >
inline void fill_array( T ( & ref_array )[ x ][ y ][ z ], T data ) {
	for( int i = 0; i < x; i++ ) {
		for( int j = 0; j < y; j++ ) {
			for( int k = 0; k < z; k++ ) {
				ref_array[ i ][ j ][ k ] = data;
			}
		}
	}
}

inline void clamp( int & x, int a, int b ) {
	x < a ? x = a : ( x > b ? x = b : x );
}

// Opengl functions
extern inline std::string getGLErrorString( int err );

inline void checkOpenGLError( const char* stmt, const char* fname, int line ) {
	GLenum err;
	while( ( err = glGetError( ) ) != GL_NO_ERROR ) {
		printf( "glError: %s, at %s:%i - %s\n", getGLErrorString( err ).c_str( ), fname, line, stmt );
		//abort( );
	}
}

//#define GL_DEBUG

#if defined( GL_DEBUG ) || defined( _DEBUG )
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define GL_CHECK(stmt) do { \
			stmt; \
			checkOpenGLError(#stmt, __FILENAME__, __LINE__); \
		} while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

// Print formatting
extern inline void printTabbedLine( int num_tabs, char const * data_print );
extern inline void printTabbedLine( int num_tabs, std::string & data_print );
extern inline void printTabbedLine( int num_tabs, std::ostringstream & data_print );