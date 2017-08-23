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

/*
template< typename T_data > static GLenum get_GL_type( );

template<> static GLenum get_GL_type< GLbyte >( ) {
	return GL_BYTE;
}

template< > static GLenum get_GL_type< GLubyte >( ) {
	return GL_UNSIGNED_BYTE;
}

template< > static GLenum get_GL_type< GLuint >( ) {
	return GL_UNSIGNED_INT;
}

template< > static GLenum get_GL_type< GLint >( ) {
	return GL_INT;
}

template< > static GLenum get_GL_type< GLfloat >( ) {
	return GL_FLOAT;
}*/

template< typename T_pos, typename T_color, typename T_norm, typename T_uvs >
struct BaseVertex { 
	T_pos pos[ 3 ];
	T_color color[ 4 ];
	T_norm norm[ 3 ];
	T_uvs uv[ 3 ];

	static GLenum get_pos_type( ) {
		return get_GL_type< T_pos >( );
	}

	static GLenum get_color_type( ) {
		return get_GL_type< T_color >( );
	}

	static GLenum get_norm_type( ) {
		return get_GL_type< T_norm >( );
	}

	static GLenum get_uvs_type( ) {
		return get_GL_type< T_uvs >( );
	}
};

template< typename T_vert >
class SharedMesh {
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

	struct SharedMeshCommands {
		GLuint count_inds;
		GLuint count_instace;
		GLuint idx_inds;
		GLuint base_vert;
		GLuint base_instance;
	};

	typedef SharedMeshCommands SMCommand;

	struct SharedMeshBlock {
		GLuint const index;
	};

	typedef SharedMeshBlock SMBlock;

	// ** Shared Mesh Geometry Set ***

	struct SharedMeshGeometrySet {
		// Geometry Type
		TypeGeometry type;

		// Texture IDs
		GLuint id_program;
		GLuint id_texture;

		// Local Index
		GLuint idx_vbo;
		GLuint idx_ibo;

		// Elemnt Count
		GLuint cnt_vbo;
		GLuint cnt_ibo;

		SharedMeshGeometrySet( TypeGeometry type, glm::mat4 const & mat_model, GLuint id_prog, GLuint id_tex ) :
			type( type ), id_program( id_prog ), id_texture( id_tex ),
			idx_vbo( 0 ), idx_ibo( 0 ), cnt_vbo( 0 ), cnt_ibo( 0 ) { }
	};

	typedef SharedMeshGeometrySet SMGSet;

	// ** End Shared Mesh Geometry Set ***

	// *** Shared Mesh Handle ***

	struct SharedMeshClientBuffer {
		std::vector< Vertex > list_verts;
		std::vector< GLuint > list_inds;

		std::vector< std::pair< float, GLuint > > list_sort;
		std::vector< GLuint > list_temp;
	};

	typedef SharedMeshClientBuffer SMCBuffer;

	class SharedMeshHandle {
		friend class SharedMesh;

	private:
		SharedMesh * ptr_parent;

		GLuint size_vbo_block;
		GLuint size_ibo_block;

		GLuint size_vbo;
		GLuint size_ibo;

		std::mutex mtx_cmds;
		std::mutex mtx_sets;

		// Prob need mutex for set data
		std::vector< std::pair< GLuint, SMBlock * > > list_vbo_blocks;
		std::vector< std::pair< GLuint, SMBlock * > > list_ibo_blocks;
		std::vector< SMGSet > list_sets;

		// Command List
		std::vector< std::pair< GLuint, SMCommand > > list_cmds;

	public:
		SMCBuffer * ptr_buffer;

		SharedMeshHandle( ) :
			ptr_parent( nullptr ) { }

		SharedMeshHandle( SharedMeshHandle const & rho ) { 
			ptr_parent = rho.ptr_parent;
			ptr_buffer = rho.ptr_buffer;

			size_vbo_block = rho.size_vbo_block;
			size_ibo_block = rho.size_ibo_block;

			size_vbo = rho.size_vbo;
			size_ibo = rho.size_ibo;

			list_vbo_blocks = rho.list_vbo_blocks;
			list_ibo_blocks = rho.list_ibo_blocks;
			list_sets = rho.list_sets;

			list_cmds = rho.list_cmds;
		}

		~SharedMeshHandle( ) { }

	private:

	public:
		GLuint get_size_vbo( ) { return size_vbo; }
		GLuint get_size_ibo( ) { return size_ibo; }

		void push_set( SMGSet & set ) {
			set.idx_vbo = size_vbo;
			set.idx_ibo = size_ibo;

			set.cnt_vbo = 0;
			set.cnt_ibo = 0;

			list_sets.push_back( set );
		}

		void finalize_set( ) {
			if( size_ibo == 0 || size_vbo == 0 ) {
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

			// Check for enough IBO space
			idx_block_s = set_last.idx_ibo / size_ibo_block;
			idx_block_e = ( set_last.idx_ibo + set_last.cnt_ibo ) / size_ibo_block;

			if( idx_block_e + 1 > list_ibo_blocks.size( ) ) {
				if( !ptr_parent->request_ibo_blocks( list_ibo_blocks, idx_block_e + 1 - ( GLuint ) list_ibo_blocks.size( ) ) ) {
					std::cout << "Error! Not enough IBO blocks!" << std::endl;
					return;
				}
			}

			if( ptr_buffer == nullptr ) {
				std::cout << "Error! Handle has a nullptr buffer pointer!" << std::endl;
				return;
			}

			if( ptr_buffer->list_inds.size( ) < set_last.idx_ibo + set_last.cnt_ibo ) {
				std::cout << "Error! Handle's inds count is larger than the buffer size!" << std::endl;
				return;
			}

			for( GLuint i = set_last.idx_ibo; i < set_last.idx_ibo + set_last.cnt_ibo; ++i ) {
				ptr_buffer->list_inds[ i ] =
					ptr_buffer->list_inds[ i ] % size_vbo_block +
					list_vbo_blocks[ ptr_buffer->list_inds[ i ] / size_vbo_block ].second->index;
			}
		}

		void push_verts( std::initializer_list< Vertex > const & verts ) {
			ptr_buffer->list_verts.insert( ptr_buffer->list_verts.end( ), verts );
			list_sets.back( ).cnt_vbo += ( GLuint ) verts.size( );
			size_vbo += ( GLuint ) verts.size( );
		}

		void push_inds( std::initializer_list< GLuint > const & inds ) {
			GLuint padding = size_ibo_block - size_ibo % size_ibo_block;

			if( padding < GeomNumIndsLookup[ list_sets.back( ).type ] || padding < inds.size( ) ) {
				for( GLuint i = 0; i < padding; ++i ) {
					ptr_buffer->list_inds.push_back( 0 );
				}

				list_sets.back( ).cnt_ibo += padding;
				size_ibo += padding;
			}

			ptr_buffer->list_inds.insert( ptr_buffer->list_inds.end( ), inds );
			list_sets.back( ).cnt_ibo += ( GLuint ) inds.size( );
			size_ibo += ( GLuint ) inds.size( );
		}

		bool request_buffer( ) {
			return ptr_parent->request_buffer( ptr_buffer );
		}

		void submit_buffer( ) {
			if( size_ibo == 0 || size_vbo == 0 ) { 
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

			idx_block_s = 0;
			idx_block_e = size_ibo / size_ibo_block;

			idx_data_s = 0;
			idx_data_e = size_ibo % size_ibo_block;

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ptr_parent->id_ibo );

			if( idx_block_s == idx_block_e ) {
				glBufferSubData( GL_ELEMENT_ARRAY_BUFFER,
					( list_ibo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( GLuint ),
					( idx_data_e - idx_data_s ) * sizeof( GLuint ),
					ptr_buffer->list_inds.data( ) + idx_data_s );
			}
			else {
				glBufferSubData( GL_ELEMENT_ARRAY_BUFFER,
					( list_ibo_blocks[ idx_block_s ].second->index + idx_data_s ) * sizeof( GLuint ),
					( size_ibo_block - idx_data_s ) * sizeof( GLuint ),
					ptr_buffer->list_inds.data( ) + idx_data_s );

				for( GLuint i = idx_block_s + 1; i < idx_block_e; ++i ) {
					glBufferSubData( GL_ELEMENT_ARRAY_BUFFER,
						list_ibo_blocks[ i ].second->index * sizeof( GLuint ),
						size_ibo_block * sizeof( GLuint ),
						ptr_buffer->list_inds.data( ) + ( i * size_ibo_block ) );
				}

				glBufferSubData( GL_ELEMENT_ARRAY_BUFFER,
					list_ibo_blocks[ idx_block_e ].second->index * sizeof( GLuint ),
					idx_data_e * sizeof( GLuint ),
					ptr_buffer->list_inds.data( ) + ( idx_block_e * size_ibo_block ) );
			}

			std::lock_guard< std::mutex > lock( mtx_cmds );
			list_cmds.clear( );

			for( GLuint i = 0; i < list_sets.size( ); ++i ) {
				auto & set = list_sets[ i ];

				idx_block_s = set.idx_ibo / size_ibo_block;
				idx_block_e = ( set.idx_ibo + set.cnt_ibo ) / size_ibo_block;

				idx_data_s = set.idx_ibo % size_ibo_block;
				idx_data_e = ( set.idx_ibo + set.cnt_ibo ) % size_ibo_block;

				if( idx_block_s == idx_block_e ) {
					list_cmds.push_back( { i, {
						idx_data_e - idx_data_s, 1,
						list_ibo_blocks[ idx_block_s ].second->index + idx_data_s,
						0, 0
					} } );
				}
				else {
					list_cmds.push_back( { i, {
						size_ibo_block - idx_data_s, 1,
						list_ibo_blocks[ idx_block_s ].second->index + idx_data_s,
						0, 0
					} } );

					for( GLuint i = idx_block_s + 1; i < idx_block_e; ++i ) {
						list_cmds.push_back( { i, {
							size_ibo_block, 1,
							list_ibo_blocks[ i ].second->index,
							0, 0
						} } );
					}

					if( idx_data_e != 0 ) {
						list_cmds.push_back( { i, {
							idx_data_e, 1,
							list_ibo_blocks[ idx_block_e ].second->index,
							0, 0
						} } );
					}
				}
			}
		}

		void release_buffer( ) { 
			ptr_parent->release_buffer( ptr_buffer );
		}

		void submit_commands( glm::mat4 const & mat_model ) {
			std::lock_guard< std::mutex > lock( mtx_cmds );

			for( GLuint i = 0; i < list_cmds.size( ); ++i ) {
				auto & cmd = list_cmds[ i ];
				ptr_parent->push_command( mat_model, glm::mat3( glm::inverseTranspose( mat_model ) ), cmd.second );
			}
		}

		void clear( ) {
			std::lock_guard< std::mutex > lock( mtx_cmds );

			if( ptr_parent ) {
				ptr_parent->release_vbo_all( list_vbo_blocks );
				ptr_parent->release_ibo_all( list_ibo_blocks );
			}

			list_sets.clear( );
			list_cmds.clear( );
			size_vbo = 0;
			size_ibo = 0;
		}


		bool swap_handle( SharedMeshHandle & handle_swap ) {
			if( ptr_parent != handle_swap.ptr_parent ) {
				return false;
			}

			std::swap( ptr_buffer, handle_swap.ptr_buffer );

			std::swap( size_vbo_block, handle_swap.size_vbo_block );
			std::swap( size_ibo_block, handle_swap.size_ibo_block );

			std::swap( size_vbo, handle_swap.size_vbo );
			std::swap( size_ibo, handle_swap.size_ibo );

			std::swap( list_vbo_blocks, handle_swap.list_vbo_blocks );
			std::swap( list_ibo_blocks, handle_swap.list_ibo_blocks );

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

		std::string print_ibo( ) { 
			std::ostringstream out;
			for( GLuint i = 0; i < list_ibo_blocks.size( ); ++i ) {
				out << "Id: " << list_ibo_blocks[ i ].first << " Index: " << list_ibo_blocks[ i ].second->index << "\n";
			}
			return out.str( );
		}

		std::string print_commands( ) { 
			std::ostringstream out;
			for( GLuint i = 0; i < list_cmds.size( ); ++i ) {
				out << "Index: " << list_cmds[ i ].second.idx_inds << " Count: " << list_cmds[ i ].second.count_inds << "\n";
			}
			return out.str( );
		}
	};

	typedef SharedMeshHandle SMHandle;

	// *** End Shared Mesh Handle ***

	typedef std::pair< GLuint, SMBlock * > SMBPair;

	struct SharedMeshServerBuffer { 
		GLuint id_buffer;
		GLuint type_buffer;
		GLuint stride_data;
	};

	typedef SharedMeshServerBuffer SMSBuffer;

	struct SharedMeshAttribute { 
		GLuint idx_buffer;
		GLuint type_data;
		GLuint num_data;
		GLuint offset;
	};

	typedef SharedMeshAttribute SMAttribute;

private:
	// GL id handles
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_ibo;
	GLuint id_cmd;
	GLuint id_mats_model;
	GLuint id_mats_norm;

	// Stride information
	GLuint size_vert;
	GLuint size_ind;

	// Buffer/Attribute containers
	std::vector< SMSBuffer > list_instance_buffers;
	std::vector< SMAttribute > list_vertex_attributes;
	std::vector< SMAttribute > list_instance_attributes;

	// IBO and VBO block data and lists
	GLuint num_vbo_blocks;
	GLuint num_ibo_blocks;

	GLuint size_vbo_block;
	std::vector< SMBlock > list_vbo_blocks;
	std::queue< std::pair< GLuint, SMBlock * > > queue_vbo_avail;
	std::unordered_map< GLuint, SMBlock * > map_vbo_live;

	GLuint size_ibo_block;
	std::vector< SMBlock > list_ibo_blocks;
	std::queue< std::pair< GLuint, SMBlock * > > queue_ibo_avail;
	std::unordered_map< GLuint, SMBlock * > map_ibo_live;

	// Command data and lists
	GLuint num_commands;

	std::vector< SMCommand > list_commands;
	std::vector< glm::mat4 > list_mats_model;
	std::vector< glm::mat3 > list_mats_norm;

	// Buffer data and lists
	GLuint num_buffers;
	GLuint size_buffer_verts;
	GLuint size_buffer_inds;

	std::vector< SMCBuffer > list_buffers;
	std::queue< SMCBuffer * > queue_buffer_avail;
	std::unordered_map< SMCBuffer *, SMCBuffer * > map_buffer_live;

	// Access Mutex
	std::mutex mtx_vbo;
	std::mutex mtx_ibo;
	std::mutex mtx_cmds;
	std::mutex mtx_buffers;

public:
	SharedMesh( ) :
		id_vao( 0 ), id_ibo( 0 ), id_vbo( 0 ) { }

	~SharedMesh( ) { }

private:
	bool request_vbo_blocks( std::vector< SMBPair > & list_vbo_blocks, GLuint num_blocks ) {
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

	bool request_ibo_blocks( std::vector< SMBPair > & list_ibo_blocks, GLuint num_blocks ) {
		std::lock_guard< std::mutex > lock( mtx_ibo );

		if( queue_ibo_avail.size( ) < num_blocks ) {
			return false;
		}

		for( GLuint i = 0; i < num_blocks; i++ ) {
			list_ibo_blocks.emplace_back( queue_ibo_avail.front( ) );
			map_ibo_live.insert( queue_ibo_avail.front( ) );
			queue_ibo_avail.pop( );
		}

		return true;
	}

	bool release_vbo_block( std::vector< SMBPair > & list_vbo_blocks, GLuint idx_block ) {
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

	bool release_ibo_block( std::vector< SMBPair > & list_ibo_blocks, GLuint idx_block ) {
		std::lock_guard< std::mutex > lock( mtx_ibo );

		if( idx_block >= list_ibo_blocks.size( ) ) {
			return false;
		}

		auto iter = map_ibo_live.find( list_ibo_blocks[ idx_block ].first );
		if( iter == map_ibo_live.end( ) ) {
			return false;
		}

		queue_ibo_avail.push( *iter );
		map_ibo_live.erase( iter );
		list_ibo_blocks.erase( list_ibo_blocks.begin( ) + idx_block );

		return true;
	}

	void release_vbo_all( std::vector< SMBPair > & list_vbo_blocks ) {
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

	void release_ibo_all( std::vector< SMBPair > & list_ibo_blocks ) {
		std::lock_guard< std::mutex > lock( mtx_ibo );

		for( auto & pair_block : list_ibo_blocks ) {
			auto iter_map = map_ibo_live.find( pair_block.first );
			if( iter_map == map_ibo_live.end( ) ) {
				continue;
			}

			queue_ibo_avail.push( *iter_map );
			map_ibo_live.erase( iter_map );
		}

		list_ibo_blocks.clear( );
	}

	bool request_buffer( SMCBuffer * & buffer ) {
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
		buffer->list_inds.clear( );

		return true;
	}

	void release_buffer( SMCBuffer * & buffer ) {
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
		buffer->list_inds.resize( size_buffer_inds );
		buffer->list_inds.shrink_to_fit( );
		buffer->list_verts.clear( );

		queue_buffer_avail.push( buffer );
		map_buffer_live.erase( iter_buffers );
		buffer = nullptr;
	}

public:
	void attach_instace_buffer( GLenum type_buffer, GLuint stride_data ) { 
		list_instance_buffers.push_back( { GLuint( ), type_buffer, stride } );
	}

	void attach_vertex_attribute( GLenum type_data, GLuint num_data, GLuint offset ) {
		list_vertex_attributes.push_back( { 0, type_data, num_data, offset } );
	}

	void attach_instance_attribute( GLenum type_data, GLuint num_data, GLuint offset ) { 
		if( !list_isntance_buffers.empty( ) ) {
			list_instance_attributes.push_back( { list_instance_buffers.size( ) - 1, type_data, num_data, offset } );
		}
	}

	void init( 
		GLuint size_vbo_block, GLuint num_vbo_blocks,
		GLuint size_ibo_block, GLuint num_ibo_blocks,
		GLuint num_buffers, GLuint size_buffer_verts, GLuint size_buffer_inds ) {

		this->num_vbo_blocks = num_vbo_blocks;
		this->num_ibo_blocks = num_ibo_blocks;

		this->num_buffers = num_buffers;
		this->size_buffer_verts = size_buffer_verts;
		this->size_buffer_inds = size_buffer_inds;

		list_buffers.resize( num_buffers );
		list_buffers.shrink_to_fit( );

		for( GLuint i = 0; i < num_buffers; ++i ) {
			list_buffers[ i ].list_verts.resize( size_buffer_verts );
			list_buffers[ i ].list_verts.shrink_to_fit( );
			list_buffers[ i ].list_inds.resize( size_buffer_inds );
			list_buffers[ i ].list_inds.shrink_to_fit( );
			queue_buffer_avail.push( &list_buffers[ i ] );
		}

		
		// Create Buffers
		glGenVertexArrays( 1, &id_vao );

		glGenBuffers( 1, &id_vbo );
		glGenBuffers( 1, &id_ibo );
		glGenBuffers( 1, &id_cmd );

		glGenBuffers( 1, &id_mats_model );
		glGenBuffers( 1, &id_mats_norm );

		/*for( GLuint i = 0; i < list_instance_buffers.size( ); ++i ) { 
			glGenBuffers( 1, &list_instance_buffers[ i ].id_buffer );
		}*/

		// Some size vars
		std::cout << "Size of vertex: " << sizeof( Vertex ) << std::endl;
		GLuint size_bytes_vbo = size_vbo_block * num_vbo_blocks * sizeof( Vertex );
		printf( "\nSize vertices buffer: %f\n", size_bytes_vbo / 1024.0f / 1024.0f );
		GLuint size_bytes_ibo = size_ibo_block * num_ibo_blocks * sizeof( GLuint );
		printf( "\nSize indices buffer: %f\n", size_bytes_ibo / 1024.0f / 1024.0f );

		float size_mb_total = ( size_bytes_ibo + size_bytes_vbo ) / 1024.0f / 1024.0f;

		printf( "\nSize total: %f\n", size_mb_total );

		// Allocate Space
		glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
		glBufferData( GL_ARRAY_BUFFER, size_bytes_vbo, nullptr, GL_STATIC_DRAW );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, size_bytes_ibo, nullptr, GL_STATIC_DRAW );

		// Setup the VAO pointers
		glBindVertexArray( id_vao );

		// Enable attributes
		GLuint num_attributes = ( GLuint ) list_vertex_attributes.size( ) + ( GLuint ) list_instance_attributes.size( );
		for( GLuint i = 0; i < 10; ++i ) { 
			glEnableVertexAttribArray( i );
		}

		GLuint offset_byte = 0;
		GLuint idx_attrib = 0;

		// Setup vertex buffer pointers
		glBindBuffer( GL_ARRAY_BUFFER, id_vbo );

		offset_byte = 0;
		for( GLuint i = 0; i < list_vertex_attributes.size( ); ++i ) { 
			auto & attrib = list_vertex_attributes[ i ];

			glVertexAttribIPointer( idx_attrib, attrib.num_data, attrib.type_data,
				sizeof( Vertex ), BUFFER_OFFSET( ( unsigned long long ) offset_byte ) );
			std::cout << "offset: " << offset_byte << std::endl;

			idx_attrib += 1;
			offset_byte += attrib.num_data * attrib.offset;
		}

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );

		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );

		glVertexAttribIPointer( idx_attrib, 1, GL_UNSIGNED_INT, sizeof( SMCommand ), BUFFER_OFFSET( 4 * sizeof( GLuint ) ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;

		glBindBuffer( GL_ARRAY_BUFFER, id_mats_model );
		glVertexAttribPointer( idx_attrib, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( 0 ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;
		glVertexAttribPointer( idx_attrib, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 4 ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;
		glVertexAttribPointer( idx_attrib, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 8 ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;
		glVertexAttribPointer( idx_attrib, 4, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 16, BUFFER_OFFSET( sizeof( GLfloat ) * 12 ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;

		glBindBuffer( GL_ARRAY_BUFFER, id_mats_norm );
		glVertexAttribPointer( idx_attrib, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 9, BUFFER_OFFSET( 0 ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;
		glVertexAttribPointer( idx_attrib, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 9, BUFFER_OFFSET( sizeof( GLfloat ) * 3 ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;
		glVertexAttribPointer( idx_attrib, 3, GL_FLOAT, GL_FALSE, sizeof( GLfloat ) * 9, BUFFER_OFFSET( sizeof( GLfloat ) * 6 ) );
		glVertexAttribDivisor( idx_attrib, 1 );
		idx_attrib += 1;

		std::cout << "idx_attrib:" << idx_attrib << std::endl;
		
		glBindVertexArray( 0 );

		// Setup VBO Blocks
		this->size_vbo_block = size_vbo_block;
		list_vbo_blocks.reserve( num_vbo_blocks );
		for( GLuint i = 0; i < num_vbo_blocks; ++i ) {
			list_vbo_blocks.emplace_back( SMBlock {
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

		// Setup IBO Blocks
		this->size_ibo_block = size_ibo_block;
		list_ibo_blocks.reserve( num_ibo_blocks );
		for( GLuint i = 0; i < num_ibo_blocks; ++i ) {
			list_ibo_blocks.emplace_back( SMBlock {
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
	}

	void end( ) { }

	bool request_handle( SMHandle & handle ) {
		if( handle.ptr_parent ) {
			return false;
		}

		handle.clear( );
		handle.size_vbo_block = size_vbo_block;
		handle.size_ibo_block = size_ibo_block;
		handle.ptr_parent = this;

		return true;
	}

	bool release_handle( SMHandle & handle ) {
		if( !handle.ptr_parent ) {
			return false;
		}

		handle.clear( );
		handle.size_vbo_block = 0;
		handle.size_ibo_block = 0;
		release_buffer( handle.ptr_buffer );
		handle.ptr_parent = nullptr;

		return true;
	}

	void clear_commands( ) {
		std::lock_guard< std::mutex > lock( mtx_cmds );

		list_mats_model.clear( );
		list_mats_norm.clear( );
		list_commands.clear( );

		num_commands = 0;
	}

	void push_command( glm::mat4 const & mat_model, glm::mat3 const & mat_norm, SMCommand & command ) {
		std::lock_guard< std::mutex > lock( mtx_cmds );

		list_mats_model.emplace_back( mat_model );
		list_mats_norm.emplace_back( mat_norm );

		command.base_instance = ( GLuint ) list_commands.size( );
		list_commands.emplace_back( command );

		num_commands++;
	}

	void buffer_commands( ) {
		std::lock_guard< std::mutex > lock( mtx_cmds );

		//glBindVertexArray( id_vao );

		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );
		glBufferData( GL_DRAW_INDIRECT_BUFFER, num_commands * sizeof( SMCommand ), list_commands.data( ), GL_STATIC_DRAW );
		//glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );

		glBindBuffer( GL_ARRAY_BUFFER, id_mats_model );
		glBufferData( GL_ARRAY_BUFFER, num_commands * sizeof( glm::mat4 ), list_mats_model.data( ), GL_STATIC_DRAW );
		//glBindBuffer( GL_ARRAY_BUFFER, 0 );

		glBindBuffer( GL_ARRAY_BUFFER, id_mats_norm );
		glBufferData( GL_ARRAY_BUFFER, num_commands * sizeof( glm::mat3 ), list_mats_norm.data( ), GL_STATIC_DRAW );
		//glBindBuffer( GL_ARRAY_BUFFER, 0 );

		//glBindVertexArray( 0 );
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

	GLuint size_ibo_avail( ) {
		return ( GLuint ) queue_ibo_avail.size( );
	}

	GLuint size_ibo_live( ) {
		return ( GLuint ) map_ibo_live.size( );

	}

	GLuint size_buffer_avail( ) { 
		return ( GLuint ) queue_buffer_avail.size( );
	}

	GLuint size_buffer_live( ) { 
		return ( GLuint ) map_buffer_live.size( );
	}

	GLuint num_primitives( ) { 
		GLuint total = 0;

		for( auto & cmd : list_commands ) {
			total += cmd.count_inds / 3;
		}

		return total;
	}

	void render( Client & client ) { 
		glBindVertexArray( id_vao );
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );

		glMultiDrawElementsIndirect(
			GL_TRIANGLES, GL_UNSIGNED_INT,
			BUFFER_OFFSET( ( GLuint ) 0 ),
			num_commands,
			sizeof( SMCommand ) );

		glBindVertexArray( 0 );
	}

	void render_range( Client & client, GLuint idx_start, GLuint length ) { 
		glBindVertexArray( id_vao );
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, id_cmd );

		glMultiDrawElementsIndirect(
			GL_TRIANGLES, GL_UNSIGNED_INT,
			BUFFER_OFFSET( idx_start * sizeof( SMCommand ) ),
			length,
			sizeof( SMCommand ) );

		glBindVertexArray( 0 );
	}
};