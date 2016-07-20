#pragma once

#include "Globals.h"
#include <vector>

enum TypeGeometry {
	TG_Points,
	TG_Lines,
	TG_Triangles,
	TG_Size
};

struct Vertex {
	GLfloat pos[ 3 ];
	GLfloat color[ 4 ];
	GLfloat norm[ 3 ];
	GLfloat uv[ 3 ];
};

struct DrawElementsIndirectCommand { 
	GLuint count;
	GLuint count_instace;
	GLuint idx_first;
	GLuint base_vert;
	GLuint base_instance;
};

typedef DrawElementsIndirectCommand DEICommand;

struct SharedMeshVboBlock {
	void * const ptr_data;
	GLuint const idx_first;
	GLuint count;
};

typedef SharedMeshVboBlock SMVBlock;

struct SharedMeshIboBlock { 
	void * const ptr_data;
	GLuint const idx_first;
	GLuint count;
};

typedef SharedMeshIboBlock SMIBlock;

struct SharedMeshGeometrySet {
	TypeGeometry type;
	GLuint idx_first;
	GLuint cnt;
};

class SharedMeshHandle {
private:
	std::vector< GLuint > list_vbo_blocks;
	std::vector< GLuint > list_ibo_blocks;
	std::vector< DEICommand > list_commands;
	std::vector< 

public:

private:

public:

};

typedef SharedMeshHandle SMHandle;

class SharedMesh {
private:
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_ibo;

	std::vector< SMVBlock > list_vbo_blocks;
	std::vector< GLuint > list_vbo_avail;
	std::vector< GLuint > list_vbo_live;

	std::vector< SMIBlock > list_ibo_blocks;
	std::vector< GLuint > list_ibo_avail;
	std::vector< GLuint > list_ibo_live;

public:
	SharedMesh( );
	~SharedMesh( );

private:

public:
	void init( 
		GLuint size_vbo_block, GLuint num_vbo_blocks,
		GLuint size_ibo_block, GLuint num_ibo_blocks );
	void end( );

	bool request_block( GLuint & id_block );
	bool release_block( GLuint id_block );
};

