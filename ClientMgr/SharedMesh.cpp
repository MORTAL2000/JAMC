#include "SharedMesh.h"
#include <iostream>
#include "Client.h"
#include "glm/gtc/type_ptr.hpp"

SharedMesh::SharedMeshHandle::SharedMeshHandle( ) :
	parent( nullptr ) { 

}

SharedMesh::SharedMeshHandle::~SharedMeshHandle( ) { 

}

void SharedMesh::SharedMeshHandle::push_set( SMGSet & set ) {
	if( list_sets.size( ) == 0 ) { 
		set.idx_vbo = 0;
		set.idx_ibo = 0;
	}
	else { 
		auto & set_last = list_sets.back( );
		set.idx_vbo = set_last.idx_vbo + set_last.cnt_vbo;
		set.idx_ibo = set_last.idx_ibo + set_last.cnt_ibo;
	}

	set.cnt_vbo = 0;
	set.cnt_ibo = 0;
	list_sets.push_back( set );
}

void SharedMesh::SharedMeshHandle::finalize_set( ) { 
	auto & set_last = list_sets.back( );

	if( set_last.list_inds.size( ) % GeomNumIndsLookup[ set_last.type ] != 0 ) {
		std::cout << "ERROR - Missmatched indices length!" << std::endl;
		std::cout << "Expected Indices( mult ): " << GeomNumIndsLookup[ set_last.type ] << " got: " << set_last.list_inds.size( ) << std::endl;
		return;
	}

	if( set_last.list_inds.size( ) > size_ibo_block ) {
		std::cout << "ERROR - Indice pattern too large for indice block!" << std::endl;
		std::cout << "Expected Indice pattern less than or equal to: " << size_ibo_block << " got: " << set_last.list_inds.size( ) << std::endl;
		return;
	}

	GLuint idx_block_ibo;
	GLuint idx_data_ibo;

	GLuint idx_block_vbo;
	GLuint idx_data_vbo;

	GLuint idx_block_vbo_curr;
	GLuint idx_data_vbo_curr;

	GLuint idx_transformed_vbo;
	//GLuint idx_transformed_vbo_last;

	GLuint idx_base_vbo = set_last.idx_vbo;
	GLuint idx_max_vbo = set_last.idx_vbo;
	GLuint idx_curr_vbo = set_last.idx_vbo;

	idx_block_ibo = ( set_last.idx_ibo + set_last.cnt_ibo ) / size_ibo_block;
	if( idx_block_ibo >= list_ibo_blocks.size( ) ) {
		if( !parent->request_ibo_blocks( list_ibo_blocks, idx_block_ibo - list_ibo_blocks.size( ) + 1 ) ) {
			std::cout << "Error! Not enough Ibo blocks in Shared Mesh!:" << std::endl;
			return;
		}
	}
	idx_data_ibo = ( set_last.idx_ibo + set_last.cnt_ibo ) % size_ibo_block;

	idx_block_vbo = set_last.idx_vbo / size_vbo_block;
	idx_data_vbo = set_last.idx_vbo % size_vbo_block;

	while( idx_base_vbo < set_last.idx_vbo + set_last.cnt_vbo - 1 ) {
		if( idx_data_ibo > size_ibo_block - set_last.list_inds.size( ) ) {
			// If we dont have enough space for a full indice set, fill with dummy data.
			while( idx_data_ibo < size_ibo_block ) { 
				// Fill with dummy data.

				memcpy( list_ibo_blocks[ idx_block_ibo ].second->ptr_data + idx_data_ibo * sizeof( GLuint ), &idx_transformed_vbo, sizeof( GLuint ) );

				idx_data_ibo++;
				set_last.cnt_ibo++;
			}
		}
		else {
			// Else we have enough space, lets iterated through the indice pattern
			for( GLuint i = 0; i < set_last.list_inds.size( ); ++i ) { 
				// For each indice, lets get an offset index in the vertex block, and record the max.
				idx_curr_vbo = idx_base_vbo + set_last.list_inds[ i ];
				if( idx_curr_vbo > idx_max_vbo ) idx_max_vbo = idx_curr_vbo;

				// Lets get a temporary offset index in the vertex block.
				idx_block_vbo_curr = idx_block_vbo;
				idx_data_vbo_curr = idx_data_vbo + set_last.list_inds[ i ];

				if( idx_data_vbo_curr >= size_vbo_block ) {
					idx_data_vbo_curr -= size_vbo_block;
					idx_block_vbo_curr++;
				}

				// Lets get the actual global index.
				idx_transformed_vbo = list_vbo_blocks[ idx_block_vbo_curr ].second->index + idx_data_vbo_curr;

				// Make the copy to data!
				memcpy( list_ibo_blocks[ idx_block_ibo ].second->ptr_data + idx_data_ibo * sizeof( GLuint ), &idx_transformed_vbo, sizeof( GLuint ) );

				// Increment the Ibo index
				idx_data_ibo += 1;
			}

			//idx_transformed_vbo_last = idx_transformed_vbo;

			// Increment all the index
			set_last.cnt_ibo += set_last.list_inds.size( );

			idx_data_vbo += idx_max_vbo - idx_base_vbo + 1;
			idx_base_vbo = idx_max_vbo + 1;
		}

		// Check bounds of new index
		if( idx_data_ibo >= size_ibo_block ) { 
			idx_data_ibo -=  size_ibo_block;
			idx_block_ibo++;

			if( idx_block_ibo >= list_ibo_blocks.size( ) ) {
				if( !parent->request_ibo_blocks( list_ibo_blocks, idx_block_ibo - list_ibo_blocks.size( ) + 1 ) ) {
					std::cout << "Error! Not enough Ibo blocks in Shared Mesh!:" << std::endl;
					return;
				}
			}
		}

		if( idx_data_vbo >= size_vbo_block ) { 
			idx_data_vbo -= size_vbo_block;
			idx_block_vbo++;
		}
	}

	GLuint idx_block_curr;
	GLuint idx_block_s;
	GLuint idx_block_e;

	GLuint idx_data_s;
	GLuint idx_data_e;

	idx_block_s = set_last.idx_ibo / size_ibo_block;
	idx_data_s = set_last.idx_ibo % size_ibo_block;

	idx_block_e = ( set_last.idx_ibo + set_last.cnt_ibo ) / size_ibo_block;
	idx_data_e = ( set_last.idx_ibo + set_last.cnt_ibo ) % size_ibo_block;

	if( idx_block_s == idx_block_e ) {
		parent->push_command( set_last.mat_model, DEICommand { 
			idx_data_e - idx_data_s, 1,
			list_ibo_blocks[ idx_block_s ].second->index + idx_data_s,
			0, 0
		} );
	}
	else {
		parent->push_command( set_last.mat_model, DEICommand {
			size_ibo_block - idx_data_s, 1,
			list_ibo_blocks[ idx_block_s ].second->index + idx_data_s,
			0, 0
		} );

		for( idx_block_curr = idx_block_s + 1; idx_block_curr < idx_block_e; ++idx_block_curr ) {
			parent->push_command( set_last.mat_model, DEICommand {
				size_ibo_block, 1,
				list_ibo_blocks[ idx_block_s ].second->index,
				0, 0
			} );
		}

		if( idx_data_e != 0 ) {
			parent->push_command( set_last.mat_model, DEICommand {
				idx_data_e, 1,
				list_ibo_blocks[ idx_block_e ].second->index,
				0, 0
			} );
		}
	}

	/*std::cout << "*** Vertex Buffer Blocks ***" << std::endl;
	for( auto & pair_block : list_vbo_blocks ) { 
		SMBlock & block = *pair_block.second;
		std::cout << "Block index: " << block.index << std::endl;
		for( GLuint i = 0; i < size_vbo_block; ++i ) {
			std::cout << "V[" << i << "]: ";
			for( GLuint j = 0; j < sizeof( Vertex ) / sizeof( GLfloat ); ++j ) {
				std::cout << *( ( GLfloat * ) ( block.ptr_data + i * sizeof( Vertex ) + j * sizeof( GLfloat ) ) ) << " ";
			}
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;

	std::cout << "*** Index Buffer Blocks ***" << std::endl;
	for( auto & pair_block : list_ibo_blocks ) {
		SMBlock & block = *pair_block.second;
		std::cout << "Block index: " << block.index << std::endl;
		for( GLuint i = 0; i < size_ibo_block; ++i ) {
			std::cout << "I[" << i << "]: ";
			std::cout << *( ( GLuint * ) ( block.ptr_data + i * sizeof( GLuint ) ) );
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;*/
}

void SharedMesh::SharedMeshHandle::buffer_data( Vertex & vert ) {
	auto & set = list_sets.back( );

	GLuint idx_block = ( set.idx_vbo + set.cnt_vbo ) / size_vbo_block;
	if( idx_block >= list_vbo_blocks.size( ) ) { 
		if( !parent->request_vbo_blocks( list_vbo_blocks, idx_block - list_vbo_blocks.size( ) + 1 ) ) {
			std::cout << "Error! Not enough Vbo blocks in Shared Mesh!" << std::endl;
			return;
		}
	}

	GLuint idx_data = ( set.idx_vbo + set.cnt_vbo ) % size_vbo_block;
	memcpy( list_vbo_blocks[ idx_block ].second->ptr_data + idx_data * sizeof( Vertex ), &vert, sizeof( Vertex ) );

	set.cnt_vbo += 1;
}

void SharedMesh::SharedMeshHandle::clear( ) { 
	list_sets.clear( );
}

void SharedMesh::SharedMeshHandle::release( ) { 
	clear( );
	parent->release_vbo_all( list_vbo_blocks );
	parent->release_ibo_all( list_ibo_blocks );
}

void SharedMesh::SharedMeshHandle::push_command_id( GLuint id_set ) { }

void SharedMesh::SharedMeshHandle::push_command_all( ) { }

void SharedMesh::SharedMeshHandle::render( Client & client ) { 
	if( !list_sets.size( ) ) return;

	auto iter_set = list_sets.begin( );

	GLuint idx_block_curr;
	GLuint idx_block_s;
	GLuint idx_block_e;

	GLuint idx_data_s;
	GLuint idx_data_e;

	glBindVertexArray( parent->id_vao );

	while( iter_set != list_sets.end( ) ) {
		client.texture_mgr.bind_program( iter_set->id_program );
		client.texture_mgr.bind_texture( 0, iter_set->id_texture );

		glm::mat4 model = glm::translate( glm::mat4( 1.0f ), glm::vec3( 100, 100, 0 ) );

		static GLuint idx_model = glGetUniformLocation( client.texture_mgr.id_prog, "mat_model" );
		glUniformMatrix4fv( idx_model, 1, GL_FALSE, glm::value_ptr( model ) );

		idx_block_s = iter_set->idx_ibo / size_ibo_block;
		idx_data_s = iter_set->idx_ibo % size_ibo_block;

		idx_block_e = ( iter_set->idx_ibo + iter_set->cnt_ibo ) / size_ibo_block;
		idx_data_e = ( iter_set->idx_ibo + iter_set->cnt_ibo ) % size_ibo_block;

		if( idx_block_s == idx_block_e ) {
			/*auto & out = client.display_mgr.out;
			out.str( "" );
			out << "Render same block: block_s: " << idx_block_s << 
				" block_e: " << idx_block_e << 
				" data_s: " << idx_data_s << 
				" data_e: " << idx_data_e << 
				" offset: " << ( list_ibo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( GLuint );

			client.gui_mgr.print_to_console( out.str( ) );*/

			glDrawElements(
				GeomGLTypeLookup[ iter_set->type ], idx_data_e - idx_data_s,
				GL_UNSIGNED_INT, BUFFER_OFFSET( ( list_ibo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( GLuint ) ) 
			);
		}
		else {
			/*auto & out = client.display_mgr.out;
			out.str( "" );
			out << "Render same block: block_s: " << idx_block_s <<
				" block_e: " << idx_block_e <<
				" data_s: " << idx_data_s <<
				" data_e: " << idx_data_e <<
				" offset: " << ( list_ibo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( GLuint );

			std::cout << out.str( ) << std::endl;
			client.gui_mgr.print_to_console( out.str( ) );*/

			glDrawElements(
				GeomGLTypeLookup[ iter_set->type ], size_ibo_block - idx_data_s,
				GL_UNSIGNED_INT, BUFFER_OFFSET( ( list_ibo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( GLuint ) )
			);

			for( idx_block_curr = idx_block_s + 1; idx_block_curr < idx_block_e; ++idx_block_curr ) {
				glDrawElements(
					GeomGLTypeLookup[ iter_set->type ], size_ibo_block,
					GL_UNSIGNED_INT, BUFFER_OFFSET( ( list_ibo_blocks[ idx_block_curr ].second->index ) * sizeof( GLuint ) )
				);
			}
			
			if( idx_data_e != 0 ) {
				glDrawElements(
					GeomGLTypeLookup[ iter_set->type ], idx_data_e,
					GL_UNSIGNED_INT, BUFFER_OFFSET( ( list_ibo_blocks[ idx_block_e ].second->index ) * sizeof( GLuint ) )
				);
			}
		}

		++iter_set;
	}

	glBindVertexArray( 0 );
}

SharedMesh::SharedMesh( ) :
	id_vao( 0 ), id_ibo( 0 ), id_vbo( 0 ) { }

SharedMesh::~SharedMesh( ) { }

void SharedMesh::init(
	GLuint size_vbo_block, GLuint num_vbo_blocks,
	GLuint size_ibo_block, GLuint num_ibo_blocks ) {

	//list_data_vbo.resize( size_vbo_block * sizeof( Vertex ) * num_vbo_blocks );
	//list_data_ibo.resize( size_ibo_block * sizeof( GLuint ) * num_ibo_blocks );

	// Setup Mapping Flags
	GLbitfield flags =
		GL_MAP_WRITE_BIT |
		GL_MAP_PERSISTENT_BIT |
		GL_MAP_COHERENT_BIT;

	std::cout << "Begin Shared Mesh: " << checkGlErrors( ) << std::endl;
	// Create Buffers
	glGenVertexArrays( 1, &id_vao );
	glGenBuffers( 1, &id_vbo );
	glGenBuffers( 1, &id_ibo );
	glGenBuffers( 1, &id_cmd );
	glGenBuffers( 1, &id_mats_model );

	std::cout << "Gen Shared Mesh: " << checkGlErrors( ) << std::endl;
	// Allocate Space
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	glBufferStorage( GL_ARRAY_BUFFER, size_vbo_block * num_vbo_blocks * sizeof( Vertex ), nullptr, flags );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	glBufferStorage( GL_ELEMENT_ARRAY_BUFFER, size_ibo_block * num_ibo_blocks * sizeof( GLuint ), nullptr, flags );
	std::cout << "Alloc Shared Mesh: " << checkGlErrors( ) << std::endl;

	// Setup the VAO pointers
	glBindVertexArray( id_vao );
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );

	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );
	glEnableVertexAttribArray( 4 );
	glEnableVertexAttribArray( 5 );
	glEnableVertexAttribArray( 6 );
	glEnableVertexAttribArray( 7 );
	glEnableVertexAttribArray( 8 );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 0 ) ); //Vert
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 12 ) ); //Color
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_TRUE, sizeof( Vertex ), BUFFER_OFFSET( 28 ) ); //Norm
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 40 ) ); //Uv

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );

	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );

	glVertexAttribIPointer( 4, 1, GL_UNSIGNED_INT, sizeof( DEICommand ), BUFFER_OFFSET( 4 * sizeof( GLuint ) ) );
	glVertexAttribDivisor( 4, 1 );

	glBindBuffer( GL_ARRAY_BUFFER, id_mats_model );
	glVertexAttribPointer( 5, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( 0 ) );
	glVertexAttribDivisor( 5, 1 );
	glVertexAttribPointer( 6, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 4 ) );
	glVertexAttribDivisor( 6, 1 );
	glVertexAttribPointer( 7, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 4 * 2 ) );
	glVertexAttribDivisor( 7, 1 );
	glVertexAttribPointer( 8, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 4 * 3 ) );
	glVertexAttribDivisor( 8, 1 );

	glBindVertexArray( 0 );
	std::cout << "Setup Pointers Shared Mesh: " << checkGlErrors( ) << std::endl;

	// Setup VBO Blocks
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	char * const ptr_base_vbo = ( char * const ) glMapBufferRange( GL_ARRAY_BUFFER, 0, size_vbo_block * num_vbo_blocks * sizeof( Vertex ), flags );
	this->size_vbo_block = size_vbo_block;
	list_vbo_blocks.reserve( num_vbo_blocks );
	for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
		list_vbo_blocks.emplace_back( SMBlock {
			//list_data_vbo.data( ) + size_vbo_block * sizeof( Vertex ) * i,
			ptr_base_vbo + size_vbo_block * i * sizeof( Vertex ),
			size_vbo_block * i
		} );
		//std::cout << "Mapping: " << i << ": " << checkGlErrors( ) << std::endl;
	}
	list_vbo_blocks.shrink_to_fit( );

	for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
		map_vbo_avail.insert( {
			i,
			&list_vbo_blocks[ i ]
		} );
	}

	std::cout << "Setup VBO blocks Shared Mesh: " << checkGlErrors( ) << std::endl;

	// Setup IBO Blocks
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	char * const ptr_base_ibo = ( char * const ) glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, size_ibo_block * num_ibo_blocks * sizeof( GLuint ), flags );
	this->size_ibo_block = size_ibo_block;
	list_ibo_blocks.reserve( num_ibo_blocks );
	for( GLuint i = 0; i < num_ibo_blocks; ++i ) {
		list_ibo_blocks.emplace_back( SMBlock {
			//list_data_ibo.data( ) + size_ibo_block * sizeof( GLuint ) * i,
			ptr_base_ibo + size_ibo_block * i * sizeof( GLuint ),
			size_ibo_block * i
		} );
		//std::cout << "Mapping: " << i << ": " << checkGlErrors( ) << std::endl;
	}
	list_ibo_blocks.shrink_to_fit( );

	for( GLuint i = 0; i < num_ibo_blocks; ++i ) {
		map_ibo_avail.insert( { 
			i,
			&list_ibo_blocks[ i ]
		} );
	}

	std::cout << "Setup IBO blocks Shared Mesh: " << checkGlErrors( ) << std::endl;
}

void SharedMesh::end( ) {

}

bool SharedMesh::get_handle( SMHandle & handle ) {
	handle.parent = this;
	handle.size_vbo_block = size_vbo_block;
	handle.size_ibo_block = size_ibo_block;

	return true;
}

bool SharedMesh::return_handle( SMHandle & handle ) {
	handle.release( );
	handle.parent = nullptr;
	handle.size_vbo_block = 0;
	handle.size_ibo_block = 0;

	return false;
}

bool SharedMesh::request_vbo_blocks( std::vector< SMBPair > & list_vbo_blocks, GLuint num_blocks ) {
	if( map_vbo_avail.size( ) < num_blocks ) return false;
	
	for( GLuint i = 0; i < num_blocks; i++ ) { 
		list_vbo_blocks.emplace_back( *map_vbo_avail.begin( ) );
		map_vbo_live.insert( *map_vbo_avail.begin( ) );
		map_vbo_avail.erase( map_vbo_avail.begin( ) );
	}

	return true;
}

bool SharedMesh::request_ibo_blocks( std::vector< SMBPair > & list_ibo_blocks, GLuint num_blocks ) {
	if( map_ibo_avail.size( ) < num_blocks ) return false;

	for( GLuint i = 0; i < num_blocks; i++ ) {
		list_ibo_blocks.emplace_back( *map_ibo_avail.begin( ) );
		map_ibo_live.insert( *map_ibo_avail.begin( ) );
		map_ibo_avail.erase( map_ibo_avail.begin( ) );
	}

	return true;
}

bool SharedMesh::release_vbo_block( std::vector< SMBPair > & list_vbo_blocks, GLuint idx_block ) {
	if( idx_block >= list_vbo_blocks.size( ) ) return false;

	auto iter = map_vbo_live.find( list_vbo_blocks[ idx_block ].first );
	if( iter == map_vbo_live.end( ) ) return false;

	map_vbo_avail.insert( *iter );
	map_vbo_live.erase( iter );
	list_vbo_blocks.erase( list_vbo_blocks.begin( ) + idx_block );

	return true;
}

bool SharedMesh::release_ibo_block( std::vector< SMBPair > & list_ibo_blocks, GLuint idx_block ) {
	if( idx_block >= list_ibo_blocks.size( ) ) return false;

	auto iter = map_ibo_live.find( list_ibo_blocks[ idx_block ].first );
	if( iter == map_ibo_live.end( ) ) return false;

	map_ibo_avail.insert( *iter );
	map_ibo_live.erase( iter );
	list_ibo_blocks.erase( list_ibo_blocks.begin( ) + idx_block );

	return true;
}

void SharedMesh::release_vbo_all( std::vector< SMBPair > & list_vbo_blocks ) {
	for( auto & pair_block : list_vbo_blocks ) { 
		auto iter_map = map_vbo_live.find( pair_block.first );
		if( iter_map == map_vbo_live.end( ) ) continue;

		map_vbo_avail.insert( *iter_map );
		map_vbo_live.erase( iter_map );
	}

	list_vbo_blocks.clear( );
}

void  SharedMesh::release_ibo_all( std::vector< SMBPair > & list_ibo_blocks ) {
	for( auto & pair_block : list_ibo_blocks ) {
		auto iter_map = map_ibo_live.find( pair_block.first );
		if( iter_map == map_ibo_live.end( ) ) continue;

		map_ibo_avail.insert( *iter_map );
		map_ibo_live.erase( iter_map );
	}

	list_ibo_blocks.clear( );
}

void SharedMesh::clear_commands( ) { 
	list_mats_model.clear( );
	list_commands.clear( );
}

void SharedMesh::push_command( glm::mat4 & mat_model, DEICommand & command ) {
	list_mats_model.emplace_back( mat_model );

	command.base_instance = list_commands.size( );
	list_commands.emplace_back( command );
}

void SharedMesh::render( Client & client ) {
	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );
	glBufferData( GL_DRAW_INDIRECT_BUFFER, list_commands.size( ) * sizeof( DEICommand ), list_commands.data( ), GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, id_mats_model );
	glBufferData( GL_ARRAY_BUFFER, list_mats_model.size( ) * sizeof( glm::mat4 ), list_mats_model.data( ), GL_STATIC_DRAW );

	client.texture_mgr.bind_program( "SMBasic" );
	client.texture_mgr.bind_texture( 0, client.texture_mgr.id_materials );

	glBindVertexArray( id_vao );

	glMultiDrawElementsIndirect( 
		GL_TRIANGLES, GL_UNSIGNED_INT,
		BUFFER_OFFSET( 0 ), 
		list_commands.size( ),
		sizeof( DEICommand ) );

	glBindVertexArray( 0 );
}

void SharedMesh::unmap( ) {
	glUnmapBuffer( id_vbo );
	glUnmapBuffer( id_ibo );
}