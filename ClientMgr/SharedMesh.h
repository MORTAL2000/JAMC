#pragma once

#include "Globals.h"
#include "glm\glm.hpp"
#include <vector>
#include <queue>
#include <list>
#include <unordered_map>
#include <mutex>
#include <sstream>

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

		SharedMeshGeometrySet(
			TypeGeometry type, glm::mat4 const & mat_model,
			GLuint id_prog, GLuint id_tex );
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
		std::vector< std::pair< SMGSet, SMCommand > > list_cmds;

	public:
		SMCBuffer * ptr_buffer;

		SharedMeshHandle( );
		~SharedMeshHandle( );

	private:

	public:
		GLuint get_size_vbo( );
		GLuint get_size_ibo( );

		void push_set( SMGSet & set );
		void finalize_set( );

		void push_verts( std::initializer_list< Vertex > const & verts );
		void push_inds( std::initializer_list< GLuint > const & inds );

		bool request_buffer( );
		void submit_buffer( );
		void release_buffer( );

		void submit_commands( );

		void clear( );

		bool swap_handle( SharedMeshHandle & handle_swap );

		std::string print_vbo( );
		std::string print_ibo( );
		std::string print_commands( );
	};

	typedef SharedMeshHandle SMHandle;

	// *** End Shared Mesh Handle ***

	typedef std::pair< GLuint, SMBlock * > SMBPair;

private:
	// GL id handles
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_ibo;
	GLuint id_cmd;
	GLuint id_mats_model;
	GLuint id_mats_norm;

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
	SharedMesh( );
	~SharedMesh( );

private:
	bool request_vbo_blocks( std::vector< SMBPair > & list_vbo_blocks, GLuint num_blocks );
	bool request_ibo_blocks( std::vector< SMBPair > & list_ibo_blocks, GLuint num_blocks );

	bool release_vbo_block( std::vector< SMBPair > & list_vbo_blocks, GLuint idx_block );
	bool release_ibo_block( std::vector< SMBPair > & list_ibo_blocks, GLuint idx_block );

	void release_vbo_all( std::vector< SMBPair > & list_vbo_blocks );
	void release_ibo_all( std::vector< SMBPair > & list_ibo_blocks );

	bool request_buffer( SMCBuffer * & buffer );
	void release_buffer( SMCBuffer * & buffer );

public:
	void init( 
		GLuint size_vbo_block, GLuint num_vbo_blocks,
		GLuint size_ibo_block, GLuint num_ibo_blocks,
		GLuint num_buffers,
		GLuint size_buffer_verts, GLuint size_buffer_inds );

	void end( );

	bool request_handle( SMHandle & handle );
	bool release_handle( SMHandle & handle );

	void clear_commands( );
	void push_command( glm::mat4 & mat_model, glm::mat3 & mat_norm, SMCommand & command );
	void buffer_commands( );

	GLuint size_commands( );

	GLuint size_vbo_avail( );
	GLuint size_vbo_live( );

	GLuint size_ibo_avail( );
	GLuint size_ibo_live( );

	GLuint size_buffer_avail( );
	GLuint size_buffer_live( );

	GLuint num_primitives( );

	void render( Client & client );
	void render_range( Client & client, GLuint idx_start, GLuint length );

};

