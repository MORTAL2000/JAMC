#include "SharedMesh.h"

SharedMesh::SharedMesh( ) :
	id_vao( 0 ), id_ibo( 0 ), id_vbo( 0 ) { }

SharedMesh::~SharedMesh( ) { }

void SharedMesh::init(
	GLuint size_vbo_block, GLuint num_vbo_blocks,
	GLuint size_ibo_block, GLuint num_ibo_blocks ) {

	// Setup Mapping Flags
	GLbitfield flags =
		GL_MAP_WRITE_BIT |
		GL_MAP_PERSISTENT_BIT |
		GL_MAP_COHERENT_BIT;

	// Create Buffers
	glGenVertexArrays( 1, &id_vao );
	glGenBuffers( 1, &id_vbo );
	glGenBuffers( 1, &id_ibo );

	// Allocate Space
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	glBufferStorage( GL_ARRAY_BUFFER, size_vbo_block * num_vbo_blocks, nullptr, flags );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	glBufferStorage( GL_ELEMENT_ARRAY_BUFFER, size_ibo_block * num_ibo_blocks, nullptr, flags );

	// Setup the VAO pointers
	glBindVertexArray( id_vao );
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );

	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 0 ) ); //Vert
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 12 ) ); //Color
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_TRUE, sizeof( Vertex ), BUFFER_OFFSET( 28 ) ); //Norm
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 40 ) ); //Uv

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	glBindVertexArray( 0 );

	// Setup VBO Blocks
	list_vbo_blocks.reserve( num_vbo_blocks );
	for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
		list_vbo_blocks.emplace_back( SMVBlock {
			glMapBufferRange( GL_ARRAY_BUFFER, size_vbo_block * i, size_vbo_block, flags ),
			size_vbo_block * i,
			0
		} );
	}
	list_vbo_blocks.shrink_to_fit( );

	list_vbo_avail.reserve( num_vbo_blocks );
	for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
		list_vbo_avail.emplace_back( GLuint{ 0 } );
		list_vbo_avail[ i ] = i;
	}
	list_vbo_avail.shrink_to_fit( );

	// Setup IBO Blocks
	list_ibo_blocks.reserve( num_ibo_blocks );
	for( GLuint i = 0; i < num_ibo_blocks; ++i ) {
		list_ibo_blocks.emplace_back( SMIBlock {
			glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, size_ibo_block * i, size_ibo_block, flags ),
			size_ibo_block * i,
			0
		} );
	}
	list_ibo_blocks.shrink_to_fit( );

	list_ibo_avail.reserve( num_ibo_blocks );
	for( GLuint i = 0; i < num_ibo_blocks; ++i ) {
		list_ibo_avail.emplace_back( GLuint { 0 } );
		list_ibo_avail[ i ] = i;
	}
	list_ibo_avail.shrink_to_fit( );
}

void SharedMesh::end( ) {

}