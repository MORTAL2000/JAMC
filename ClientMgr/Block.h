#pragma once
#include "Globals.h"

#include "Color4.h"
#include "Directional.h"

#include <string>
#include <array>

typedef std::array< float, 3 > UV;
typedef std::array< float, 3 > Vert;

typedef std::array< UV, 4 > FaceUvs;
typedef std::array< Vert, 4 > FaceVerts;

class Block {
private:
	static std::array< FaceVerts const *, FaceDirection::FD_Size > verts;
	static std::array< glm::vec3 const *, FaceDirection::FD_Size > verts_center;

private:
	static FaceVerts const
		verts_front, verts_back, 
		verts_left, verts_right, 
		verts_up, verts_down,
		verts_null;

	static glm::vec3 const
		vert_front_center, vert_back_center, 
		vert_left_center, vert_right_center,
		vert_up_center, vert_down_center;

public:
	static FaceVerts const & get_verts( FaceDirection const face );
	static glm::vec3 const & get_center( FaceDirection const face );

public:
	int const id;
	Color4 color;
	std::array< FaceUvs, FaceDirection::FD_Size > uvs;
	std::array< int, FaceDirection::FD_Size > orientation;
	std::string const name;
	bool is_trans;

	Block( int const id, std::string const & name );
	~Block( );

public:
	FaceUvs & get_uvs( FaceDirection const face );
	int get_orientation( FaceDirection const face );

	bool is_visible( Block const & block );
};

