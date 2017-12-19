#pragma once

#include "Globals.h"
#include "glm\glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <unordered_map>
#include <mutex>
#include <sstream>

template< typename T_vert >
class SharedMeshTerrain {
public:
	enum TypeGeometry {
		TG_Points,
		TG_Lines,
		TG_Triangles,
		TG_Size
	};

	 GLuint constexpr static GeomNumIndsLookup[ TypeGeometry::TG_Size ] = {
		1, 2, 3
	};

	 GLuint constexpr static GeomGLTypeLookup[ TypeGeometry::TG_Size ] = {
		GL_POINTS, GL_LINES, GL_TRIANGLES
	};

	 using Vertex = T_vert;

	struct SharedMeshTerrainCommand {
		GLuint count_vert;
		GLuint count_instace;
		GLuint idx_vert;
		GLuint base_instance;
	};

	typedef SharedMeshTerrainCommand SMTCommand;

	struct SharedMeshTerrainBlock {
		GLuint const index;
	};

	typedef SharedMeshTerrainBlock SMTBlock;

	// ** Shared Mesh Geometry Set ***

	struct SharedMeshTerrainGeometrySet {
		// Geometry Type
		TypeGeometry type;

		// Texture IDs
		GLuint id_program;
		GLuint id_texture;

		// Local Index
		GLuint idx_vbo;

		// Elemnt Count
		GLuint cnt_vbo;

		SharedMeshTerrainGeometrySet( TypeGeometry type, GLuint id_prog, GLuint id_tex ) :
			type( type ), id_program( id_prog ), id_texture( id_tex ),
			idx_vbo( 0 ), cnt_vbo( 0 ) { }
	};

	typedef SharedMeshTerrainGeometrySet SMTGSet;

	// ** End Shared Mesh Geometry Set ***

	// *** Shared Mesh Handle ***

	struct SharedMeshTerrainClientBuffer {
		std::vector< Vertex > list_verts;

		std::vector< std::pair< float, GLuint > > list_sort;
		std::vector< Vertex > list_temp;
	};

	typedef SharedMeshTerrainClientBuffer SMTCBuffer;

	class SharedMeshTerrainHandle {
		friend class SharedMeshTerrain;

	private:
		SharedMeshTerrain * ptr_parent;

		GLuint size_vbo_block;

		GLuint size_vbo;

		std::mutex mtx_cmds;
		std::mutex mtx_sets;

		// Prob need mutex for set data
		std::vector< std::pair< GLuint, SMTBlock * > > list_vbo_blocks;
		std::vector< SMTGSet > list_sets;

		// Command List
		std::vector< std::pair< GLuint, SMTCommand > > list_cmds;

	public:
		SMTCBuffer * ptr_buffer;

		SharedMeshTerrainHandle( ) :
			ptr_parent( nullptr ) { }

		SharedMeshTerrainHandle( SharedMeshTerrainHandle const & rho ) { 
			ptr_parent = rho.ptr_parent;
			ptr_buffer = rho.ptr_buffer;

			size_vbo_block = rho.size_vbo_block;

			size_vbo = rho.size_vbo;

			list_vbo_blocks = rho.list_vbo_blocks;
			list_sets = rho.list_sets;

			list_cmds = rho.list_cmds;
		}

		~SharedMeshTerrainHandle( ) { }

	private:

	public:
		GLuint get_size_vbo( ) { return size_vbo; }

		void push_set( SMTGSet & set ) {
			set.idx_vbo = size_vbo;
			set.cnt_vbo = 0;

			list_sets.push_back( set );
		}

		void finalize_set( ) {
			if( size_vbo == 0 ) {
				list_sets.erase( list_sets.end( ) - 1 );
				return;
			}

			GLuint idx_block_s;
			GLuint idx_block_e;

			auto & set_last = list_sets.back( );

			// Check for enough VBO space
			idx_block_s = set_last.idx_vbo / size_vbo_block;
			idx_block_e = ( set_last.idx_vbo + set_last.cnt_vbo ) / size_vbo_block;

			if( idx_block_e + 1 > list_vbo_blocks.size( ) ) {
				if( !ptr_parent->request_vbo_blocks( list_vbo_blocks, idx_block_e + 1 - ( GLuint ) list_vbo_blocks.size( ) ) ) {
					std::cout << "Error! Not enough VBO blocks!" << std::endl;
					return;
				}
			}
		}

		void push_verts( std::initializer_list< Vertex > const & verts ) {
			/*GLuint padding = size_vbo_block - size_vbo % size_vbo_block;

			if( padding < verts.size( ) ) {
				for( GLuint i = 0; i < padding; ++i ) {
					ptr_buffer->list_verts.push_back( Vertex { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } );
				}

				list_sets.back( ).cnt_vbo += padding;
				size_vbo += padding;
			}*/

			ptr_buffer->list_verts.insert( ptr_buffer->list_verts.end( ), verts );
			list_sets.back( ).cnt_vbo += ( GLuint ) verts.size( );
			size_vbo += ( GLuint ) verts.size( );
		}

		bool request_buffer( ) {
			return ptr_parent->request_buffer( ptr_buffer );
		}

		void submit_buffer( ) {
			if( size_vbo == 0 ) { 
				return;
			}

			GLuint idx_block_s;
			GLuint idx_block_e;

			GLuint idx_data_s;
			GLuint idx_data_e;

			idx_block_s = 0;
			idx_block_e = size_vbo / size_vbo_block;

			idx_data_s = 0;
			idx_data_e = size_vbo % size_vbo_block;

			if( idx_block_s >= ( GLuint ) list_vbo_blocks.size( ) ) {
				idx_block_s = ( GLuint ) list_vbo_blocks.size( ) - 1;
				std::cout << "You dun goofed on your buffer size bra." << std::endl;
				return;
			}

			if( idx_block_e >= ( GLuint ) list_vbo_blocks.size( ) ) {
				idx_block_e = ( GLuint ) list_vbo_blocks.size( ) - 1;
				std::cout << "You dun goofed on your buffer size bra." << std::endl;
				return;
			}

			glBindBuffer( GL_ARRAY_BUFFER, ptr_parent->id_vbo );

			if( idx_block_s == idx_block_e ) {
				glBufferSubData( GL_ARRAY_BUFFER,
					( list_vbo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( Vertex ),
					( idx_data_e - idx_data_s ) * sizeof( Vertex ),
					ptr_buffer->list_verts.data( ) + idx_data_s );
			}
			else {
				glBufferSubData( GL_ARRAY_BUFFER,
					( list_vbo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( Vertex ),
					( size_vbo_block - idx_data_s ) * sizeof( Vertex ),
					ptr_buffer->list_verts.data( ) + idx_data_s );

				for( GLuint i = idx_block_s + 1; i < idx_block_e; ++i ) {
					glBufferSubData( GL_ARRAY_BUFFER,
						list_vbo_blocks[ i ].second->index * sizeof( Vertex ),
						size_vbo_block * sizeof( Vertex ),
						ptr_buffer->list_verts.data( ) + ( i * size_vbo_block ) );
				}

				glBufferSubData( GL_ARRAY_BUFFER,
					list_vbo_blocks[ idx_block_e ].second->index * sizeof( Vertex ),
					idx_data_e * sizeof( Vertex ),
					ptr_buffer->list_verts.data( ) + ( idx_block_e * size_vbo_block ) );
			}

			std::lock_guard< std::mutex > lock( mtx_cmds );
			list_cmds.clear( );

			for( GLuint i = 0; i < list_sets.size( ); ++i ) {
				auto & set = list_sets[ i ];

				idx_block_s = set.idx_vbo / size_vbo_block;
				idx_block_e = ( set.idx_vbo + set.cnt_vbo ) / size_vbo_block;

				idx_data_s = set.idx_vbo % size_vbo_block;
				idx_data_e = ( set.idx_vbo + set.cnt_vbo ) % size_vbo_block;

				if( idx_block_s == idx_block_e ) {
					list_cmds.push_back( { i, {
						idx_data_e - idx_data_s, 1,
						list_vbo_blocks[ idx_block_s ].second->index + idx_data_s,
						0
					} } );
				}
				else {
					list_cmds.push_back( { i, {
						size_vbo_block - idx_data_s, 1,
						list_vbo_blocks[ idx_block_s ].second->index + idx_data_s,
						0
					} } );

					for( GLuint i = idx_block_s + 1; i < idx_block_e; ++i ) {
						list_cmds.push_back( { i, {
							size_vbo_block, 1,
							list_vbo_blocks[ i ].second->index,
							0
						} } );
					}

					if( idx_data_e != 0 ) {
						list_cmds.push_back( { i, {
							idx_data_e, 1,
							list_vbo_blocks[ idx_block_e ].second->index,
							0
						} } );
					}
				}
			}
		}

		void release_buffer( ) { 
			ptr_parent->release_buffer( ptr_buffer );
		}

		void submit_commands( glm::vec3 const & vec_model ) {
			std::lock_guard< std::mutex > lock( mtx_cmds );

			for( GLuint i = 0; i < list_cmds.size( ); ++i ) {
				auto & cmd = list_cmds[ i ];
				ptr_parent->push_command( vec_model, cmd.second );
			}
		}

		GLuint num_cmd_primitives( ) {
			int unsigned cnt = 0;

			for( GLuint i = 0; i < list_cmds.size( ); ++i ) {
				cnt += list_cmds[ i ].second.count_vert;
			}

			return cnt;
		}

		GLuint num_vbo_blocks( ) { 
			return list_vbo_blocks.size( );
		}

		void clear( ) {
			std::lock_guard< std::mutex > lock( mtx_cmds );

			if( ptr_parent ) {
				ptr_parent->release_vbo_all( list_vbo_blocks );
			}

			list_sets.clear( );
			list_cmds.clear( );

			size_vbo = 0;
		}

		bool swap_handle( SharedMeshTerrainHandle & handle_swap ) {
			if( ptr_parent != handle_swap.ptr_parent ) {
				return false;
			}

			std::swap( ptr_buffer, handle_swap.ptr_buffer );

			std::swap( size_vbo_block, handle_swap.size_vbo_block );
			std::swap( size_vbo, handle_swap.size_vbo );

			std::swap( list_vbo_blocks, handle_swap.list_vbo_blocks );

			std::swap( list_sets, handle_swap.list_sets );
			std::swap( list_cmds, handle_swap.list_cmds );

			return true;
		}

		std::string print_vbo( ) { 
			std::ostringstream out;
			for( GLuint i = 0; i < list_vbo_blocks.size( ); ++i ) {
				out << "Id: " << list_vbo_blocks[ i ].first << " Index: " << list_vbo_blocks[ i ].second->index << "\n";
			}
			return out.str( );
		}

		std::string print_commands( ) { 
			std::ostringstream out;
			for( GLuint i = 0; i < list_cmds.size( ); ++i ) {
				out << "Index: " << list_cmds[ i ].second.idx_vert << " Count: " << list_cmds[ i ].second.count_vert << "\n";
			}
			return out.str( );
		}
	};

	typedef SharedMeshTerrainHandle SMTHandle;

	// *** End Shared Mesh Handle ***

	typedef std::pair< GLuint, SMTBlock * > SMTBPair;

private:
	// GL id handles
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_cmd;
	GLuint id_vecs_model;

	// Stride information
	GLuint size_vert;

	// IBO and VBO block data and lists
	GLuint num_vbo_blocks;

	GLuint size_vbo_block;
	std::vector< SMTBlock > list_vbo_blocks;
	std::queue< std::pair< GLuint, SMTBlock * > > queue_vbo_avail;
	std::unordered_map< GLuint, SMTBlock * > map_vbo_live;

	//Total Size data
	GLuint size_bytes_vbo;

	// Command data and lists
	GLuint num_commands;

	std::vector< SMTCommand > list_commands;
	std::vector< glm::vec3 > list_vecs_model;

	// Buffer data and lists
	GLuint num_buffers;
	GLuint size_buffer_verts;

	std::vector< SMTCBuffer > list_buffers;
	std::queue< SMTCBuffer * > queue_buffer_avail;
	std::unordered_map< SMTCBuffer *, SMTCBuffer * > map_buffer_live;

	// Access Mutex
	std::mutex mtx_vbo;
	std::mutex mtx_cmds;
	std::mutex mtx_buffers;

public:
	SharedMeshTerrain( ) :
		id_vao( 0 ), id_vbo( 0 ) { }

	~SharedMeshTerrain( ) { }

private:
	bool request_vbo_blocks( std::vector< SMTBPair > & list_vbo_blocks, GLuint num_blocks ) {
		std::lock_guard< std::mutex > lock( mtx_vbo );

		if( queue_vbo_avail.size( ) < num_blocks ) {
			return false;
		}

		for( GLuint i = 0; i < num_blocks; i++ ) {
			list_vbo_blocks.emplace_back( queue_vbo_avail.front( ) );
			map_vbo_live.insert( queue_vbo_avail.front( ) );
			queue_vbo_avail.pop( );
		}

		return true;
	}

	bool release_vbo_block( std::vector< SMTBPair > & list_vbo_blocks, GLuint idx_block ) {
		std::lock_guard< std::mutex > lock( mtx_vbo );

		if( idx_block >= list_vbo_blocks.size( ) ) {
			return false;
		}

		auto iter = map_vbo_live.find( list_vbo_blocks[ idx_block ].first );
		if( iter == map_vbo_live.end( ) ) {
			return false;
		}

		queue_vbo_avail.push( *iter );
		map_vbo_live.erase( iter );
		list_vbo_blocks.erase( list_vbo_blocks.begin( ) + idx_block );

		return true;
	}

	void release_vbo_all( std::vector< SMTBPair > & list_vbo_blocks ) {
		std::lock_guard< std::mutex > lock( mtx_vbo );

		for( auto & pair_block : list_vbo_blocks ) {
			auto iter_map = map_vbo_live.find( pair_block.first );
			if( iter_map == map_vbo_live.end( ) ) {
				continue;
			}

			queue_vbo_avail.push( *iter_map );
			map_vbo_live.erase( iter_map );
		}

		list_vbo_blocks.clear( );
	}

	bool request_buffer( SMTCBuffer * & buffer ) {
		if( buffer != nullptr ) {
			return true;
		}

		std::lock_guard< std::mutex > lock( mtx_buffers );

		if( queue_buffer_avail.empty( ) ) {
			buffer = nullptr;
			return false;
		}

		buffer = queue_buffer_avail.front( );
		map_buffer_live.insert( { queue_buffer_avail.front( ), queue_buffer_avail.front( ) } );
		queue_buffer_avail.pop( );

		buffer->list_verts.clear( );

		return true;
	}

	void release_buffer( SMTCBuffer * & buffer ) {
		if( !buffer ) {
			return;
		}

		std::lock_guard< std::mutex > lock( mtx_buffers );

		auto iter_buffers = map_buffer_live.find( buffer );
		if( iter_buffers == map_buffer_live.end( ) ) {
			buffer = nullptr;
			return;
		}

		buffer->list_verts.resize( size_buffer_verts );
		buffer->list_verts.shrink_to_fit( );
		buffer->list_verts.clear( );

		queue_buffer_avail.push( buffer );
		map_buffer_live.erase( iter_buffers );
		buffer = nullptr;
	}

public:
	void init( 
		GLuint size_vbo_block, GLuint num_vbo_blocks,
		GLuint num_buffers, GLuint size_buffer_verts ) {

		this->num_vbo_blocks = num_vbo_blocks;

		this->num_buffers = num_buffers;
		this->size_buffer_verts = size_buffer_verts;

		list_buffers.resize( num_buffers );
		list_buffers.shrink_to_fit( );

		for( GLuint i = 0; i < num_buffers; ++i ) {
			list_buffers[ i ].list_verts.resize( size_buffer_verts );
			list_buffers[ i ].list_verts.shrink_to_fit( );
			queue_buffer_avail.push( &list_buffers[ i ] );
		}

		// Create Buffers
		glGenVertexArrays( 1, &id_vao );

		glGenBuffers( 1, &id_vbo );
		glGenBuffers( 1, &id_cmd );

		glGenBuffers( 1, &id_vecs_model );

		// Some size vars
		//std::cout << "Size of vertex: " << sizeof( Vertex ) << std::endl;
		size_bytes_vbo = size_vbo_block * num_vbo_blocks * sizeof( Vertex );
		//printf( "\nSize vertices buffer: %f\n", size_bytes_vbo / 1024.0f / 1024.0f );

		//float size_mb_total = size_bytes_vbo / 1024.0f / 1024.0f;

		//printf( "\nSize total: %f\n", size_mb_total );

		// Allocate Space
		glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
		glBufferData( GL_ARRAY_BUFFER, size_bytes_vbo, nullptr, GL_STATIC_DRAW );

		// Setup the VAO pointers
		glBindVertexArray( id_vao );

		// Enable attributes
		for( GLuint i = 0; i < 4; ++i ) { 
			glEnableVertexAttribArray( i );
		}

		GLuint offset_byte = 0;
		GLuint idx_attrib = 0;

		// Setup vertex buffer pointers
		glBindBuffer( GL_ARRAY_BUFFER, id_vbo );

		glVertexAttribIPointer( 0, 1, GL_UNSIGNED_INT, sizeof( Vertex ), BUFFER_OFFSET( 0 ) );
		glVertexAttribIPointer( 1, 1, GL_UNSIGNED_INT, sizeof( Vertex ), BUFFER_OFFSET( 4 ) );

		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );

		glVertexAttribIPointer( 2, 1, GL_UNSIGNED_INT, sizeof( SMTCommand ), BUFFER_OFFSET( 3 * sizeof( GLuint ) ) );
		glVertexAttribDivisor( 2, 1 );

		glBindBuffer( GL_ARRAY_BUFFER, id_vecs_model );
		glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 3, BUFFER_OFFSET( 0 ) );
		glVertexAttribDivisor( 3, 1 );

		std::cout << "idx_attrib:" << idx_attrib << std::endl;
		
		glBindVertexArray( 0 );

		// Setup VBO Blocks
		this->size_vbo_block = size_vbo_block;
		list_vbo_blocks.reserve( num_vbo_blocks );
		for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
			list_vbo_blocks.emplace_back( SMTBlock {
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
	}

	void end( ) { }

	bool request_handle( SMTHandle & handle ) {
		if( handle.ptr_parent ) {
			return false;
		}

		handle.clear( );
		handle.size_vbo_block = size_vbo_block;
		handle.ptr_parent = this;

		return true;
	}

	bool release_handle( SMTHandle & handle ) {
		if( !handle.ptr_parent ) {
			return false;
		}

		handle.clear( );
		handle.size_vbo_block = 0;
		release_buffer( handle.ptr_buffer );
		handle.ptr_parent = nullptr;

		return true;
	}

	void clear_commands( ) {
		std::lock_guard< std::mutex > lock( mtx_cmds );

		list_vecs_model.clear( );
		list_commands.clear( );

		num_commands = 0;
	}

	void push_command( glm::vec3 const & vec_model, SMTCommand & command ) {
		std::lock_guard< std::mutex > lock( mtx_cmds );

		list_vecs_model.emplace_back( vec_model );

		command.base_instance = ( GLuint ) list_commands.size( );
		list_commands.emplace_back( command );

		num_commands++;
	}

	void buffer_commands( ) {
		std::lock_guard< std::mutex > lock( mtx_cmds );

		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );
		glBufferData( GL_DRAW_INDIRECT_BUFFER, num_commands * sizeof( SMTCommand ), list_commands.data( ), GL_STATIC_DRAW );

		glBindBuffer( GL_ARRAY_BUFFER, id_vecs_model );
		glBufferData( GL_ARRAY_BUFFER, num_commands * sizeof( glm::vec3 ), list_vecs_model.data( ), GL_STATIC_DRAW );
	}


	GLuint size_commands( ) {
		return num_commands;
	}

	GLuint size_vbo_avail( ) {
		return ( GLuint ) queue_vbo_avail.size( );
	}

	GLuint size_vbo_live( ) {
		return ( GLuint ) map_vbo_live.size( );
	}

	GLuint size_buffer_avail( ) { 
		return ( GLuint ) queue_buffer_avail.size( );
	}

	GLuint size_buffer_live( ) { 
		return ( GLuint ) map_buffer_live.size( );
	}

	GLuint size_bytes_total( ) { 
		return size_bytes_vbo;
	}

	GLuint num_cmd_primitives( ) { 
		GLuint total = 0;

		for( auto & cmd : list_commands ) {
			total += cmd.count_vert;
		}

		return total;
	}

	void render( Client & client ) { 
		glBindVertexArray( id_vao );
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );

		glMultiDrawArraysIndirect(
			GL_POINTS,
			BUFFER_OFFSET( ( GLuint ) 0 ),
			num_commands,
			sizeof( SMTCommand ) );

		glBindVertexArray( 0 );
	}

	void render_range( Client & client, GLuint idx_start, GLuint length ) { 
		glBindVertexArray( id_vao );
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );

		glMultiDrawArraysIndirect(
			GL_POINTS,
			BUFFER_OFFSET( idx_start * sizeof( SMTCommand ) ),
			length,
			sizeof( SMTCommand ) );

		glBindVertexArray( 0 );
	}
};