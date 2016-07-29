#pragma once

#include "Globals.h"
#include "glm\glm.hpp"
#include <vector>
#include <map>

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

	struct Vertex {
		GLfloat pos[ 3 ];
		GLfloat color[ 4 ];
		GLfloat norm[ 3 ];
		GLfloat uv[ 3 ];
	};

	struct DrawElementsIndirectCommand {
		GLuint count_inds;
		GLuint count_instace;
		GLuint idx_inds;
		GLuint base_vert;
		GLuint base_instance;
	};

	typedef DrawElementsIndirectCommand DEICommand;

	struct SharedMeshBlock {
		char * const ptr_data;
		GLuint const index;
	};

	typedef SharedMeshBlock SMBlock;

	struct SharedMeshGeometrySet {
		// Geometry Type
		TypeGeometry type;

		// Model Matrix
		glm::mat4 mat_model;

		// Texture IDs
		GLuint id_program;
		GLuint id_texture;

		// Local Index
		GLuint idx_vbo;
		GLuint idx_ibo;

		// Elemnt Count
		GLuint cnt_vbo;
		GLuint cnt_ibo;

		// Indice Pattern
		std::vector< GLuint > list_inds;
	};

	typedef SharedMeshGeometrySet SMGSet;

	class SharedMeshHandle {
		friend class SharedMesh;

	private:
		SharedMesh * parent;

		GLuint size_vbo_block;
		GLuint size_ibo_block;

		std::vector< std::pair< GLuint, SMBlock * > > list_vbo_blocks;
		std::vector< std::pair< GLuint, SMBlock * > > list_ibo_blocks;
		std::vector< SMGSet > list_sets;

	public:
		SharedMeshHandle( );
		~SharedMeshHandle( );

	private:

	public:
		void push_set( SMGSet & set );
		void finalize_set( );

		void buffer_data( Vertex & vert );

		template< int unsigned num >
		void buffer_data( Vertex( &vert_array )[ num ] ) { 
			
		}

		void clear( );
		void release( );

		void push_command_id( GLuint id_set );
		void push_command_all( );

		void render( Client & client );
	};

	typedef SharedMeshHandle SMHandle;

	typedef std::pair< GLuint, SMBlock * > SMBPair;

private:
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_ibo;
	GLuint id_cmd;
	GLuint id_mats_model;

	GLuint num_blocks;

	GLuint size_vbo_block;
	std::vector< SMBlock > list_vbo_blocks;
	std::map< GLuint, SMBlock * > map_vbo_avail;
	std::map< GLuint, SMBlock * > map_vbo_live;

	GLuint size_ibo_block;
	std::vector< SMBlock > list_ibo_blocks;
	std::map< GLuint, SMBlock * > map_ibo_avail;
	std::map< GLuint, SMBlock * > map_ibo_live;

	std::vector< glm::mat4 > list_mats_model;
	std::vector< DEICommand > list_commands;

public:
	std::vector< char > list_data_vbo;
	std::vector< char > list_data_ibo;

	SharedMesh( );
	~SharedMesh( );

public:
	void init( 
		GLuint size_vbo_block, GLuint num_vbo_blocks,
		GLuint size_ibo_block, GLuint num_ibo_blocks );

	void end( );

	bool get_handle( SMHandle & handle );
	bool return_handle( SMHandle & handle );

	bool request_vbo_blocks( std::vector< SMBPair > & list_vbo_blocks, GLuint num_blocks );
	bool request_ibo_blocks( std::vector< SMBPair > & list_ibo_blocks, GLuint num_blocks );

	bool release_vbo_block( std::vector< SMBPair > & list_vbo_blocks, GLuint idx_block );
	bool release_ibo_block( std::vector< SMBPair > & list_ibo_blocks, GLuint idx_block );

	void release_vbo_all( std::vector< SMBPair > & list_vbo_blocks );
	void release_ibo_all( std::vector< SMBPair > & list_ibo_blocks );

	void clear_commands( );
	void push_command( glm::mat4 & mat_model, DEICommand & command );

	void render( Client & client );

	void unmap( );
};

