#include "Chunk.h"

glm::ivec3 const Chunk::vec_size( Chunk::size_x, Chunk::size_y, Chunk::size_z );

Chunk::Chunk( ) :
	ptr_adj { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
	ptr_buffer { nullptr },
	ptr_noise { nullptr } { 
}

Chunk::~Chunk( ) { }
