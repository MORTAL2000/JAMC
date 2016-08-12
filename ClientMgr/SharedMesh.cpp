#include "SharedMesh.h"
#include <iostream>
#include "Client.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"

SharedMesh::SharedMeshGeometrySet::SharedMeshGeometrySet(
	SharedMesh::TypeGeometry type, glm::mat4 const & mat_model,
	GLuint id_prog, GLuint id_tex, std::vector< GLuint > & list_inds ) :
	type( type ), mat_model( mat_model ),
	mat_norm( glm::inverseTranspose( mat_model ) ),
	id_program( id_prog ), id_texture( id_tex ),
	idx_vbo( 0 ), idx_ibo( 0 ),
	cnt_vbo( 0 ), cnt_ibo( 0 ),
	list_inds( list_inds ) {}

SharedMesh::SharedMeshHandle::SharedMeshHandle( ) :
	parent( nullptr ) { }

SharedMesh::SharedMeshHandle::~SharedMeshHandle( ) { }

GLuint SharedMesh::SharedMeshHandle::get_size_vbo( ) { 
	return size_vbo;
}

GLuint SharedMesh::SharedMeshHandle::get_size_ibo( ) { 
	return size_ibo;
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

	if( set_last.cnt_vbo == 0 ) { 
		return;
	}

	GLuint idx_block_ibo;
	GLuint idx_data_ibo;

	GLuint idx_block_vbo;
	GLuint idx_data_vbo;

	GLuint idx_block_vbo_curr;
	GLuint idx_data_vbo_curr;

	GLuint idx_transformed_vbo;

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

	size_vbo += set_last.cnt_vbo;
	size_ibo += set_last.cnt_ibo;

	GLuint idx_block_curr;
	GLuint idx_block_s;
	GLuint idx_block_e;

	GLuint idx_data_s;
	GLuint idx_data_e;

	idx_block_s = set_last.idx_ibo / size_ibo_block;
	idx_data_s = set_last.idx_ibo % size_ibo_block;

	idx_block_e = ( set_last.idx_ibo + set_last.cnt_ibo ) / size_ibo_block;
	idx_data_e = ( set_last.idx_ibo + set_last.cnt_ibo ) % size_ibo_block;

	std::lock_guard< std::mutex > lock( mtx_cmds );

	if( idx_block_s == idx_block_e ) {
		list_cmds.push_back( {
			&set_last,
			DEICommand {
				idx_data_e - idx_data_s, 1,
				list_ibo_blocks[ idx_block_s ].second->index + idx_data_s,
				0, 0
			} 
		} );
	}
	else {
		list_cmds.push_back( {
			&set_last,
			DEICommand {
				size_ibo_block - idx_data_s, 1,
				list_ibo_blocks[ idx_block_s ].second->index + idx_data_s,
				0, 0
			} 
		} );

		for( idx_block_curr = idx_block_s + 1; idx_block_curr < idx_block_e; ++idx_block_curr ) {
			list_cmds.push_back( {
				&set_last,
				DEICommand {
					size_ibo_block, 1,
					list_ibo_blocks[ idx_block_curr ].second->index,
					0, 0
				} 
			} );
		}

		if( idx_data_e != 0 ) {
			list_cmds.push_back( {
				&set_last,
				DEICommand {
					idx_data_e, 1,
					list_ibo_blocks[ idx_block_e ].second->index,
					0, 0
				}
			} );
		}
	}

	list_cmds = list_cmds;
}

void SharedMesh::SharedMeshHandle::buffer_data( Vertex & vert ) {
	auto & set = list_sets.back( );

	GLuint idx_block = ( set.idx_vbo + set.cnt_vbo ) / size_vbo_block;
	if( idx_block >= list_vbo_blocks.size( ) ) { 
		if( !parent->request_vbo_blocks( list_vbo_blocks, idx_block - list_vbo_blocks.size( ) + 1 ) ) {
			//std::cout << "Error! Not enough Vbo blocks in Shared Mesh!" << std::endl;
			return;
		}
	}

	GLuint idx_data = ( set.idx_vbo + set.cnt_vbo ) % size_vbo_block;
	memcpy( list_vbo_blocks[ idx_block ].second->ptr_data + idx_data * sizeof( Vertex ), &vert, sizeof( Vertex ) );

	set.cnt_vbo += 1;
}

void SharedMesh::SharedMeshHandle::clear( ) {
	std::lock_guard< std::mutex > lock( mtx_cmds );

	list_cmds.clear( );
	list_sets.clear( );
	size_vbo = 0;
	size_ibo = 0;
}

void SharedMesh::SharedMeshHandle::release( ) { 
	clear( );
	if( parent ) {
		parent->release_vbo_all( list_vbo_blocks );
		parent->release_ibo_all( list_ibo_blocks );
	}
}

void SharedMesh::SharedMeshHandle::submit_commands( ) {
	std::lock_guard< std::mutex > lock( mtx_cmds );

	for( auto & cmd : list_cmds ) { 
		parent->push_command( cmd.first->mat_model, cmd.first->mat_norm, cmd.second );
	}
}

SharedMesh::SharedMesh( ) :
	id_vao( 0 ), id_ibo( 0 ), id_vbo( 0 ) { }

SharedMesh::~SharedMesh( ) { }

void SharedMesh::process_released( ) {
	{
		std::lock_guard< std::mutex > lock( mtx_vbo );

		while( !queue_vbo_release.empty( ) ) { 
			queue_vbo_release.front().second->sync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
			list_vbo_sync.push_back( queue_vbo_release.front( ) );
			queue_vbo_release.pop( );
		}

		auto iter_sync = list_vbo_sync.begin( );
		GLenum result = GL_UNSIGNALED;

		while( iter_sync != list_vbo_sync.end( ) ) {
			result = glClientWaitSync( iter_sync->second->sync, 0, 0 );

			if( result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED ) {
				glDeleteSync( iter_sync->second->sync );
				queue_vbo_avail.push( *iter_sync );
				iter_sync = list_vbo_sync.erase( iter_sync );
			}
			else {
				iter_sync++;
			}
		}
	}

	{
		std::lock_guard< std::mutex > lock( mtx_ibo );

		while( !queue_ibo_release.empty( ) ) {
			queue_ibo_release.front( ).second->sync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
			list_ibo_sync.push_back( queue_ibo_release.front( ) );
			queue_ibo_release.pop( );
		}

		auto iter_sync = list_ibo_sync.begin( );
		GLenum result = GL_UNSIGNALED;

		while( iter_sync != list_ibo_sync.end( ) ) {
			result = glClientWaitSync( iter_sync->second->sync, 0, 0 );

			if( result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED ) {
				glDeleteSync( iter_sync->second->sync );
				queue_ibo_avail.push( *iter_sync );
				iter_sync = list_ibo_sync.erase( iter_sync );
			}
			else {
				iter_sync++;
			}
		}
	}
}

void SharedMesh::init(
	GLuint size_vbo_block, GLuint num_vbo_blocks,
	GLuint size_ibo_block, GLuint num_ibo_blocks ) {

	this->num_vbo_blocks = num_vbo_blocks;
	this->num_ibo_blocks = num_ibo_blocks;

	// Setup Mapping Flags
	GLbitfield flags_map =
		GL_MAP_WRITE_BIT |
		GL_MAP_PERSISTENT_BIT | 
		GL_MAP_COHERENT_BIT | 
		GL_MAP_FLUSH_EXPLICIT_BIT;

	GLbitfield flags_create =
		flags_map |
		GL_DYNAMIC_STORAGE_BIT;

	std::cout << "Begin Shared Mesh: " << checkGlErrors( ) << std::endl;
	// Create Buffers
	glGenVertexArrays( 1, &id_vao );
	glGenBuffers( 1, &id_vbo );
	glGenBuffers( 1, &id_ibo );
	glGenBuffers( 1, &id_cmd );
	glGenBuffers( 1, &id_mats_model );
	glGenBuffers( 1, &id_mats_norm );

	std::cout << "Gen Shared Mesh: " << checkGlErrors( ) << std::endl;
	// Allocate Space
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	glBufferStorage( GL_ARRAY_BUFFER, size_vbo_block * num_vbo_blocks * sizeof( Vertex ), nullptr, flags_create );
	glFlush( );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	glBufferStorage( GL_ELEMENT_ARRAY_BUFFER, size_ibo_block * num_ibo_blocks * sizeof( GLuint ), nullptr, flags_create );
	glFlush( );
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
	glEnableVertexAttribArray( 9 );
	glEnableVertexAttribArray( 10 );
	glEnableVertexAttribArray( 11 );

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
	glVertexAttribPointer( 7, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 8 ) );
	glVertexAttribDivisor( 7, 1 );
	glVertexAttribPointer( 8, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 12 ) );
	glVertexAttribDivisor( 8, 1 );

	glBindBuffer( GL_ARRAY_BUFFER, id_mats_norm );
	glVertexAttribPointer( 9, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 9, BUFFER_OFFSET( 0 ) );
	glVertexAttribDivisor( 9, 1 );
	glVertexAttribPointer( 10, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 9, BUFFER_OFFSET( sizeof( GLfloat ) * 3 ) );
	glVertexAttribDivisor( 10, 1 );
	glVertexAttribPointer( 11, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 9, BUFFER_OFFSET( sizeof( GLfloat ) * 6 ) );
	glVertexAttribDivisor( 11, 1 );

	glBindVertexArray( 0 );
	std::cout << "Setup Pointers Shared Mesh: " << checkGlErrors( ) << std::endl;

	// Setup VBO Blocks
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	char unsigned * const ptr_base_vbo = ( char unsigned * const ) glMapBufferRange( 
		GL_ARRAY_BUFFER, 0, 
		size_vbo_block * num_vbo_blocks * sizeof( Vertex ), 
		flags_map );

	this->size_vbo_block = size_vbo_block;
	list_vbo_blocks.reserve( num_vbo_blocks );
	for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
		list_vbo_blocks.emplace_back( SMBlock {
			ptr_base_vbo + size_vbo_block * i * sizeof( Vertex ),
			size_vbo_block * i
		} );
	}
	list_vbo_blocks.shrink_to_fit( );

	for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
		queue_vbo_avail.push( {
			i,
			&list_vbo_blocks[ i ]
		} );
	}

	std::cout << "Setup VBO blocks Shared Mesh: " << checkGlErrors( ) << std::endl;

	// Setup IBO Blocks
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	char unsigned * const ptr_base_ibo = ( char unsigned * const ) glMapBufferRange( 
		GL_ELEMENT_ARRAY_BUFFER, 0, 
		size_ibo_block * num_ibo_blocks * sizeof( GLuint ), 
		flags_map );

	this->size_ibo_block = size_ibo_block;
	list_ibo_blocks.reserve( num_ibo_blocks );
	for( GLuint i = 0; i < num_ibo_blocks; ++i ) {
		list_ibo_blocks.emplace_back( SMBlock {
			ptr_base_ibo + size_ibo_block * i * sizeof( GLuint ),
			size_ibo_block * i
		} );
	}
	list_ibo_blocks.shrink_to_fit( );

	for( GLuint i = 0; i < num_ibo_blocks; ++i ) {
		queue_ibo_avail.push( {
			i,
			&list_ibo_blocks[ i ]
		} );
	}

	std::cout << "Setup IBO blocks Shared Mesh: " << checkGlErrors( ) << std::endl;
}

void SharedMesh::end( ) {

}

bool SharedMesh::get_handle( SMHandle & handle ) {
	handle.clear( );
	handle.size_vbo_block = size_vbo_block;
	handle.size_ibo_block = size_ibo_block;
	handle.parent = this;

	return true;
}

bool SharedMesh::return_handle( SMHandle & handle ) {
	handle.release( );
	handle.size_vbo_block = 0;
	handle.size_ibo_block = 0;
	handle.parent = nullptr;

	return false;
}

bool SharedMesh::request_vbo_blocks( std::vector< SMBPair > & list_vbo_blocks, GLuint num_blocks ) {
	std::lock_guard< std::mutex > lock( mtx_vbo );

	if( queue_vbo_avail.size( ) < num_blocks ) return false;
	
	for( GLuint i = 0; i < num_blocks; i++ ) { 
		list_vbo_blocks.emplace_back( queue_vbo_avail.front( ) );
		map_vbo_live.insert( queue_vbo_avail.front( ) );
		queue_vbo_avail.pop( );
	}

	return true;
}

bool SharedMesh::request_ibo_blocks( std::vector< SMBPair > & list_ibo_blocks, GLuint num_blocks ) {
	std::lock_guard< std::mutex > lock( mtx_ibo );

	if( queue_ibo_avail.size( ) < num_blocks ) return false;

	for( GLuint i = 0; i < num_blocks; i++ ) {
		list_ibo_blocks.emplace_back( queue_ibo_avail.front( ) );
		map_ibo_live.insert( queue_ibo_avail.front( ) );
		queue_ibo_avail.pop( );
	}

	return true;
}

bool SharedMesh::release_vbo_block( std::vector< SMBPair > & list_vbo_blocks, GLuint idx_block ) {
	std::lock_guard< std::mutex > lock( mtx_vbo );

	if( idx_block >= list_vbo_blocks.size( ) ) return false;

	auto iter = map_vbo_live.find( list_vbo_blocks[ idx_block ].first );
	if( iter == map_vbo_live.end( ) ) return false;

	queue_vbo_release.push( *iter );
	map_vbo_live.erase( iter );
	list_vbo_blocks.erase( list_vbo_blocks.begin( ) + idx_block );

	return true;
}

bool SharedMesh::release_ibo_block( std::vector< SMBPair > & list_ibo_blocks, GLuint idx_block ) {
	std::lock_guard< std::mutex > lock( mtx_ibo );

	if( idx_block >= list_ibo_blocks.size( ) ) return false;

	auto iter = map_ibo_live.find( list_ibo_blocks[ idx_block ].first );
	if( iter == map_ibo_live.end( ) ) return false;

	queue_ibo_release.push( *iter );
	map_ibo_live.erase( iter );
	list_ibo_blocks.erase( list_ibo_blocks.begin( ) + idx_block );

	return true;
}

void SharedMesh::release_vbo_all( std::vector< SMBPair > & list_vbo_blocks ) {
	std::lock_guard< std::mutex > lock( mtx_vbo );

	for( auto & pair_block : list_vbo_blocks ) { 
		auto iter_map = map_vbo_live.find( pair_block.first );
		if( iter_map == map_vbo_live.end( ) ) continue;

		queue_vbo_release.push( *iter_map );
		map_vbo_live.erase( iter_map );
	}

	list_vbo_blocks.clear( );
}

void  SharedMesh::release_ibo_all( std::vector< SMBPair > & list_ibo_blocks ) {
	std::lock_guard< std::mutex > lock( mtx_ibo );

	for( auto & pair_block : list_ibo_blocks ) {
		auto iter_map = map_ibo_live.find( pair_block.first );
		if( iter_map == map_ibo_live.end( ) ) continue;

		queue_ibo_release.push( *iter_map );
		map_ibo_live.erase( iter_map );
	}

	list_ibo_blocks.clear( );
}

void SharedMesh::clear_commands( ) {
	std::lock_guard< std::mutex > lock( mtx_cmd );

	list_mats_model.clear( );
	list_mats_norm.clear( );
	list_commands.clear( );
}

void SharedMesh::push_command( glm::mat4 & mat_model, glm::mat3 & mat_norm, DEICommand & command ) {
	std::lock_guard< std::mutex > lock( mtx_cmd );

	list_mats_model.emplace_back( mat_model );
	list_mats_norm.emplace_back( mat_norm );

	command.base_instance = list_commands.size( );
	list_commands.emplace_back( command );
}

void SharedMesh::buffer_commands( ) { 
	std::lock_guard< std::mutex > lock( mtx_cmd );
	num_commands = list_commands.size( );

	std::cout << "SM In: " << checkGlErrors( ) << std::endl;

	glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );
	glBufferData( GL_DRAW_INDIRECT_BUFFER, num_commands * sizeof( DEICommand ), list_commands.data( ), GL_STATIC_DRAW );

	//std::cout << "Cmds: " << checkGlErrors( ) << std::endl;

	glBindBuffer( GL_ARRAY_BUFFER, id_mats_model );
	glBufferData( GL_ARRAY_BUFFER, num_commands * sizeof( glm::mat4 ), list_mats_model.data( ), GL_STATIC_DRAW );

	//std::cout << "Models: " << checkGlErrors( ) << std::endl;

	glBindBuffer( GL_ARRAY_BUFFER, id_mats_norm );
	glBufferData( GL_ARRAY_BUFFER, num_commands * sizeof( glm::mat3 ), list_mats_norm.data( ), GL_STATIC_DRAW );

	//std::cout << "Norms: " << checkGlErrors( ) << std::endl;

	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	glFlushMappedBufferRange( GL_ARRAY_BUFFER, 0, num_vbo_blocks * size_vbo_block * sizeof( Vertex ) );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	glFlushMappedBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, num_ibo_blocks * size_ibo_block * sizeof( GLuint ) );

	std::cout << "SM Out: " << checkGlErrors( ) << std::endl;

	//for( auto & cmd : list_commands ) { 
	//	glFlushMappedBufferRange( GL_ELEMENT_ARRAY_BUFFER, cmd.idx_inds * sizeof( GLuint ), cmd.count_inds * sizeof( GLuint ) );
	//}
}

GLuint SharedMesh::size_commands( ) { 
	return num_commands;
}

GLuint SharedMesh::size_vbo_live( ) {
	return map_vbo_live.size( );
}

GLuint SharedMesh::size_ibo_live( ) {
	return map_ibo_live.size( );
}

GLuint SharedMesh::size_vbo_avail( ) {
	return queue_vbo_avail.size( );
}

GLuint SharedMesh::size_ibo_avail( ) {
	return queue_ibo_avail.size( );
}

GLuint SharedMesh::size_vbo_release( ) {
	return queue_vbo_release.size( );
}

GLuint SharedMesh::size_ibo_release( ) {
	return queue_ibo_release.size( );
}

GLuint SharedMesh::size_vbo_sync( ) {
	return list_vbo_sync.size( );
}

GLuint SharedMesh::size_ibo_sync( ) {
	return list_ibo_sync.size( );
}

GLuint SharedMesh::num_primitives( ) {
	GLuint total = 0;
	for( auto & cmd : list_commands ) { 
		total += cmd.count_inds / 3;
	}

	return total;
}

void SharedMesh::render( Client & client ) {
	glBindVertexArray( id_vao );

	glMultiDrawElementsIndirect( 
		GL_TRIANGLES, GL_UNSIGNED_INT,
		BUFFER_OFFSET( 0 ),
		num_commands,
		0 );

	glBindVertexArray( 0 );
}

void SharedMesh::render_range( Client & client, GLuint idx_start, GLuint length ) { 
	glBindVertexArray( id_vao );

	glMultiDrawElementsIndirect(
		GL_TRIANGLES, GL_UNSIGNED_INT,
		BUFFER_OFFSET( idx_start * sizeof( DEICommand ) ),
		length,
		0 );

	glBindVertexArray( 0 );
}

void SharedMesh::render_old( Client & client ) {
	glBindVertexArray( id_vao );

	for( auto & cmd : list_commands ) { 
		glDrawElements( GL_TRIANGLES, cmd.count_inds, GL_UNSIGNED_INT, BUFFER_OFFSET( cmd.idx_inds ) );
	}

	glBindVertexArray( 0 );
}

void SharedMesh::unmap( ) {
	glUnmapBuffer( id_vbo );
	glUnmapBuffer( id_ibo );
}