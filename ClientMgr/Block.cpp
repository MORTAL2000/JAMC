#include "Block.h"
#include "TextureMgr.h"

Block::Block( int const id, std::string const & name ) :
	id( id ), name( name ),
	color( 1.0f, 1.0f, 1.0f, 1.0f ), uvs{ },
	is_trans( false ) { }

Block::~Block( ) { }

FaceVerts const Block::verts_front{ { { 0, 0, 1 },{ 1, 0, 1 },{ 1, 1, 1 },{ 0, 1, 1 } } };
FaceVerts const Block::verts_back{ { { 1, 0, 0 },{ 0, 0, 0 },{ 0, 1, 0 },{ 1, 1, 0 } } };
FaceVerts const Block::verts_left{ { { 1, 0, 1 },{ 1, 0, 0 },{ 1, 1, 0 },{ 1, 1, 1 } } };
FaceVerts const Block::verts_right{ { { 0, 0, 0 },{ 0, 0, 1 },{ 0, 1, 1 },{ 0, 1, 0 } } };
FaceVerts const Block::verts_up{ { { 1, 1, 0 },{ 0, 1, 0 },{ 0, 1, 1 },{ 1, 1, 1 } } };
FaceVerts const Block::verts_down{ { { 0, 0, 0 },{ 1, 0, 0 },{ 1, 0, 1 },{ 0, 0, 1 } } };
FaceVerts const Block::verts_null{ { { 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 },{ 0, 0, 0 } } };

glm::vec3 const Block::vert_front_center{ 0.5f, 0.5f, 1.0f };
glm::vec3 const Block::vert_back_center{ 0.5f, 0.5f, 0.0f };
glm::vec3 const Block::vert_left_center{ 1.0f, 0.5f, 0.5f };
glm::vec3 const Block::vert_right_center{ 0.0f, 0.5f, 0.5f };
glm::vec3 const Block::vert_up_center{ 0.5f, 1.0f, 0.5f };
glm::vec3 const Block::vert_down_center{ 0.5f, 0.0f, 0.5f };

std::array< FaceVerts const *, FD_Size > Block::verts{ {
	&Block::verts_front, &Block::verts_back,
	&Block::verts_left, &Block::verts_right,
	&Block::verts_up, &Block::verts_down
} };

std::array< glm::vec3 const *, FD_Size > Block::verts_center{ {
	&Block::vert_front_center, &Block::vert_back_center,
	&Block::vert_left_center, &Block::vert_right_center,
	&Block::vert_up_center, &Block::vert_down_center
} };

FaceVerts const & Block::get_verts( FaceDirection const face ) {
	return *verts[ face ];
}

glm::vec3 const & Block::get_center( FaceDirection const face ) {
	return *verts_center[ face ];
}

FaceUvs & Block::get_uvs( FaceDirection const face ) {
	return uvs[ face ];
}

int Block::get_orientation( FaceDirection const face ) { 
	return orientation[ face ];
}

bool Block::is_visible( Block const & block ) { 
	//if( !is_trans ) { 
	//	if( !block.is_trans ) {
	//		return false;
	//	}

	//	return true;
	//}

	if( !block.is_trans ) {
		return false;
	}

	if( id == block.id ) {
		return false;
	}

	return true;
}