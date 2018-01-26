#pragma once

#include "Globals.h"

#include "glew\include\GL\glew.h"
#include "glm\glm.hpp"

#include <vector>

class VBO {
public:
	enum TypeGeometry {
		TG_Points,
		TG_Lines,
		TG_Triangles,
		TG_Size
	};

	static GLuint const GeomNumIndsLookup[ VBO::TypeGeometry::TG_Size ];
	static GLuint const GeomGLTypeLookup[ VBO::TypeGeometry::TG_Size ];

	struct Vertex {
		glm::vec3 pos;
		glm::vec4 color;
		glm::vec3 norm;
		glm::vec3 uv;
	};

	class IndexSet {
	private:
	public:
		TypeGeometry type;

		std::string const str_prog;
		GLuint id_tex;

		GLuint num_eles;

		GLuint idx_verts;
		GLuint len_verts;

		GLuint idx_inds;
		GLuint len_inds;

		std::vector< GLuint > list_inds;

	private:
	public:
		IndexSet( TypeGeometry type, std::string const & str_prog,
			GLuint id_tex, std::vector< GLuint > & list_inds ); 
		~IndexSet( );

	};

private:
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_ibo;
	
	GLuint size_vbo;
	GLuint size_ibo;
	GLuint last_size_vbo;
	GLuint last_size_ibo;

	GLuint index_end;
	std::vector< Vertex > list_verts;
	std::vector< GLuint > list_inds;
	std::vector< IndexSet > list_sets;

private:

public:
	VBO( );
	~VBO( );

	void init( );
	void push_set( IndexSet & set_idx );
	void push_data( Vertex const & vert );
	void push_data( std::vector< Vertex > & list_vert );
	void finalize_set( );

	void buffer( );
	void render( Client & client, bool is_tex_array );
	void render_range( Client & client, bool is_tex_array, GLuint index, GLuint length );

	void clear( );
};

