#include "Color4.h"

Color4::Color4( ) :
	Color4( 0, 0, 0, 0 ) { }

Color4::Color4( GLfloat r, GLfloat g, GLfloat b, GLfloat a ) :
	r( r ), g( g ), b( b ), a( a ) { }

Color4::~Color4( ) { }

Color4& Color4::operator=( Color4 &rho ) {
	r = rho.r;
	b = rho.b;
	g = rho.g;
	a = rho.a;
	return *this;
}

void Color4::set( GLfloat r, GLfloat g, GLfloat b, GLfloat a ) { 
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

const Color4 Color4::Red( 1.0f, 0.0f, 0.0f, 1.0f );
const Color4 Color4::Green( 0.0f, 1.0f, 0.0f, 1.0f );
const Color4 Color4::Blue( 0.0f, 0.0f, 1.0f, 1.0f );
const Color4 Color4::Purple( 0.502f, 0.0f, 0.502f, 1.0f );
const Color4 Color4::Yellow( 1.0f, 1.0f, 0.0f, 1.0f );
const Color4 Color4::Orange( 1.0f, 0.647f, 0.0f, 1.0f );
const Color4 Color4::Black( 0.0f, 0.0f, 0.0f, 1.0f );
const Color4 Color4::Gray( 0.502f, 0.502f, 0.502f, 1.0f );
const Color4 Color4::White( 1.0f, 1.0f, 1.0f, 1.0f );