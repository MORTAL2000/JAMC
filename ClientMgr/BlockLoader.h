#pragma once
#include "Globals.h"

#include "Directional.h"

#include <string>
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

	int unsigned occlude;

	std::string subtex;
	int unsigned id_subtex;

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

class BlockLoader {
public:
	int id;
	glm::vec4 color;
	std::string name;

	std::vector< Face > faces;

	std::array< std::vector< int >, FaceDirection::FD_Size > occlude_lookup;
	std::vector< int > include_lookup;

	std::string texture;
	int unsigned id_texture;

	bool is_trans;
	bool is_coll;

	BlockLoader( ) :
		id( 0 ),
		name( "Default" ),
		faces( ),
		texture( "Blocks" ),
		id_texture( 0 ),
		is_trans( false ) { }

	~BlockLoader( );

	bool is_visible( BlockLoader const & other ) const;
};