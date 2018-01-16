#pragma once

#pragma comment( lib, "glew/lib/Release/x64/glew32" )
#pragma comment( lib, "glut/x64/glut32" )
#pragma comment( lib, "soil/lib/SOIL" )

#ifdef NDEBUG
#pragma comment( lib, "tinyxml2" )

#else
#pragma comment( lib, "tinyxml2_debug" )

#endif

#include <stdlib.h>

// Soil Includes
#include "soil\src\SOIL.h"

// OpenGL Includes
#include "glew\include\GL\glew.h"
#include "glew\include\GL\wglew.h"
#include "glut\glut.h"