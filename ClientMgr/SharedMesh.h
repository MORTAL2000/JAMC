#pragma once

#include "Globals.h"
#include "glm\glm.hpp"
#include <vector>
#include <queue>
#include <list>
#include <unordered_map>
#include <mutex>

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
		char unsigned * const ptr_data;
		GLuint const index;
		GLsync sync;
	};

	typedef SharedMeshBlock SMBlock;

	// ** Shared Mesh Geometry Set ***

	struct SharedMeshGeometrySet {
		// Geometry Type
		TypeGeometry type;

		// Model Matrix
		glm::mat4 mat_model;
		glm::mat3 mat_norm;

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

		SharedMeshGeometrySet(
			TypeGeometry type, glm::mat4 const & mat_model,
			GLuint id_prog, GLuint id_tex,
			std::vector< GLuint > & list_inds );
	};

	typedef SharedMeshGeometrySet SMGSet;

	// ** End Shared Mesh Geometry Set ***

	// *** Shared Mesh Handle ***

	class SharedMeshHandle {
		friend class SharedMesh;

	private:
		SharedMesh * parent;

		GLuint size_vbo_block;
		GLuint size_ibo_block;

		GLuint size_vbo;
		GLuint size_ibo;

		std::mutex mtx_cmds;

		// Prob need mutex for set data
		std::vector< std::pair< GLuint, SMBlock * > > list_vbo_blocks;
		std::vector< std::pair< GLuint, SMBlock * > > list_ibo_blocks;
		std::vector< SMGSet > list_sets;

		// Command List
		std::vector< std::pair< SMGSet *, DEICommand > > list_cmds;

	public:
		SharedMeshHandle( );
		~SharedMeshHandle( );

	private:
		void clear( );

	public:
		GLuint get_size_vbo( );
		GLuint get_size_ibo( );

		void push_set( SMGSet & set );
		void buffer_data( Vertex & vert );
		void finalize_set( );

		void submit_commands( );

		void release( );
	};

	typedef SharedMeshHandle SMHandle;

	// *** End Shared Mesh Handle ***

	typedef std::pair< GLuint, SMBlock * > SMBPair;

private:
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_ibo;
	GLuint id_cmd;
	GLuint id_mats_model;
	GLuint id_mats_norm;
	
	GLuint num_commands;
	GLuint num_vbo_blocks;
	GLuint num_ibo_blocks;

	GLuint size_vbo_block;
	std::vector< SMBlock > list_vbo_blocks;
	std::queue< std::pair< GLuint, SMBlock * > > queue_vbo_avail;
	std::unordered_map< GLuint, SMBlock * > map_vbo_live;
	std::queue< std::pair< GLuint, SMBlock * > > queue_vbo_release;
	std::list< std::pair< GLuint, SMBlock * > > list_vbo_sync;

	GLuint size_ibo_block;
	std::vector< SMBlock > list_ibo_blocks;
	std::queue< std::pair< GLuint, SMBlock * > > queue_ibo_avail;
	std::unordered_map< GLuint, SMBlock * > map_ibo_live;
	std::queue< std::pair< GLuint, SMBlock * > > queue_ibo_release;
	std::list< std::pair < GLuint, SMBlock * > > list_ibo_sync;

	std::vector< DEICommand > list_commands;
	std::vector< glm::mat4 > list_mats_model;
	std::vector< glm::mat3 > list_mats_norm;

	std::mutex mtx_vbo;
	std::mutex mtx_ibo;
	std::mutex mtx_cmd;

public:
	SharedMesh( );
	~SharedMesh( );

private:

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
	void push_command( glm::mat4 & mat_model, glm::mat3 & mat_norm, DEICommand & command );
	void buffer_commands( );

	void process_released( );

	GLuint size_commands( );
	GLuint size_vbo_live( );
	GLuint size_ibo_live( );
	GLuint size_vbo_avail( );
	GLuint size_ibo_avail( );
	GLuint size_vbo_release( );
	GLuint size_ibo_release( );
	GLuint size_vbo_sync( );
	GLuint size_ibo_sync( );

	GLuint num_primitives( );

	void render( Client & client );
	void render_range( Client & client, GLuint idx_start, GLuint length );
	void render_old( Client & client );

	void unmap( );
};

