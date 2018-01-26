#pragma once

#include "glew\include\GL\glew.h"
#include "glew\include\GL\wglew.h"

#include <string>

// Opengl functions
extern inline std::string getGLErrorString( int err );

static inline void checkOpenGLError( const char* stmt, const char* fname, int line ) {
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