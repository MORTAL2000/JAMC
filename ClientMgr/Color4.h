#pragma once

#include "Globals.h"

class Color4 {
public:
	GLfloat r, g, b, a;

	Color4();
	Color4( GLfloat r, GLfloat g, GLfloat b, GLfloat a );
	~Color4();

	Color4& operator=( Color4 &other_color );

	void set( GLfloat r, GLfloat g, GLfloat b, GLfloat a );

	static const Color4 Red;
	static const Color4 Green;
	static const Color4 Blue;
	static const Color4 Purple;
	static const Color4 Yellow;
	static const Color4 Orange;
	static const Color4 Black;
	static const Color4 Gray;
	static const Color4 White;
};
