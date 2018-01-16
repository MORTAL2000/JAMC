#include "Errors.h"

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