#pragma once
#include "Globals.h"

#include "Directional.h"

#include <string>
#include <array>
#include <vector>

typedef std::array< glm::vec3, 4 > FaceVerts;
typedef std::array< glm::vec3, 4 > FaceUvs;
typedef std::array< glm::vec3, 4 > FaceNorms;

class Face { 
public:
	glm::vec3 offset;
	glm::vec3 dim;
	glm::vec3 rot;
	glm::vec4 color;

	GLuint occlude;

	std::string subtex;
	GLuint id_subtex;

	FaceVerts verts;
	FaceUvs uvs;
	FaceNorms norms;

	Face( ) :
		offset( 0.0f, 0.0f, 0.0f ),
		dim( 1.0f, 1.0f, 1.0f ),
		rot( 0.0f, 0.0f, 0.0f ),
		color( 1.0f, 1.0f, 1.0f, 1.0f ),
		occlude( FaceDirection::FD_Size ),
		subtex( "Default/Top" ),
		id_subtex( 0 ) { }
};

class Block {
public:
	int id;
	glm::vec4 color;
	std::string name;

	std::vector< Face > faces;

	std::array< std::vector< int >, FaceDirection::FD_Size > occlude_lookup;
	std::vector< int > include_lookup;

	std::string texture;
	GLuint id_texture;

	bool is_trans;
	bool is_coll;

	Block( ) :
		id( 0 ),
		name( "Default" ),
		faces( ),
		texture( "Blocks" ),
		id_texture( 0 ),
		is_trans( false ) { }

	~Block( );

	bool is_visible( Block const & other );
};