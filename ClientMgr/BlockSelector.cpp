#include "BlockSelector.h"
#include "Client.h"
#include "glm/gtc/type_ptr.hpp"

BlockSelector::BlockSelector( Client & client ) :
	client( client ) {}

BlockSelector::~BlockSelector( ) { 
	
}

/*void BlockSelector::make_mesh( ) { 
	std::cout << "Block select errors1: " << checkGlErrors( ) << std::endl;

	handles[ 0 ].push_set( SharedMesh::SMGSet {
		SharedMesh::TypeGeometry::TG_Triangles,
		glm::mat4( 1.0f ),
		client.texture_mgr.get_program( "Basic" )->id_prog,
		client.texture_mgr.id_materials,
		0, 0, 0, 0,
		{ 0, 1, 2, 2, 3, 0 }
	} );

	float size = 50;

	for( GLuint i = 0; i < 10; ++i ) {
		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size, i * size, 0 },
			{ 1, 1, 1, 1 },
			{ 0, 0, 1 },
			{ 0, 0, 0 }
		} );

		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size, i * size, 0 },
			{ 1, 1, 1, 1 },
			{ 0, 0, 1 },
			{ 1, 0, 0 }
		} );

		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size, i * size + size, 0 },
			{ 1, 1, 1, 1 },
			{ 0, 0, 1 },
			{ 1, 1, 0 }
		} );

		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size, i * size + size, 0 },
			{ 1, 1, 1, 1 },
			{ 0, 0, 1 },
			{ 0, 1, 0 }
		} );
	}

	handles[ 0 ].finalize_set( );

	std::cout << "Block select errors2: " << checkGlErrors( ) << std::endl;

	handles[ 1 ].push_set( SharedMesh::SMGSet {
		SharedMesh::TypeGeometry::TG_Triangles,
		glm::translate( glm::mat4( 1.0f ), glm::vec3( size, 0, 0 ) ),
		client.texture_mgr.get_program( "Basic" )->id_prog,
		client.texture_mgr.id_materials,
		0, 0, 0, 0,
		{ 0, 1, 2, 2, 3, 0 }
	} );

	for( GLuint i = 0; i < 10; ++i ) {
		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size, i * size, 0 },
			{ 1, 0, 0, 1 },
			{ 0, 0, 1 },
			{ 0, 0, 0 }
		} );

		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size, i * size, 0 },
			{ 1, 0, 0, 1 },
			{ 0, 0, 1 },
			{ 1, 0, 0 }
		} );

		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size, i * size + size, 0 },
			{ 1, 0, 0, 1 },
			{ 0, 0, 1 },
			{ 1, 1, 0 }
		} );

		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size, i * size + size, 0 },
			{ 1, 0, 0, 1 },
			{ 0, 0, 1 },
			{ 0, 1, 0 }
		} );
	}

	handles[ 1 ].finalize_set( );

	handles[ 1 ].push_set( SharedMesh::SMGSet {
		SharedMesh::TypeGeometry::TG_Triangles,
		glm::translate( glm::mat4( 1.0f ), glm::vec3( size * 2, 0, 0 ) ),
		client.texture_mgr.get_program( "Basic" )->id_prog,
		client.texture_mgr.id_materials,
		0, 0, 0, 0,
		{ 0, 1, 2, 2, 3, 0 }
	} );

	for( GLuint i = 0; i < 10; ++i ) {
		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size, i * size, 0 },
			{ 0, 0, 1, 1 },
			{ 0, 0, 1 },
			{ 0, 0, 0 }
		} );

		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size + size, i * size, 0 },
			{ 0, 0, 1, 1 },
			{ 0, 0, 1 },
			{ 1, 0, 0 }
		} );

		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size + size, i * size + size, 0 },
			{ 0, 0, 1, 1 },
			{ 0, 0, 1 },
			{ 1, 1, 0 }
		} );

		handles[ 1 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size, i * size + size, 0 },
			{ 0, 0, 1, 1 },
			{ 0, 0, 1 },
			{ 0, 1, 0 }
		} );
	}

	handles[ 1 ].finalize_set( );

	handles[ 0 ].push_set( SharedMesh::SMGSet {
		SharedMesh::TypeGeometry::TG_Triangles,
		glm::translate( glm::mat4( 1.0f ), glm::vec3( size * 3, 0, 0 ) ),
		client.texture_mgr.get_program( "Basic" )->id_prog,
		client.texture_mgr.id_materials,
		0, 0, 0, 0,
		{ 0, 1, 2, 2, 3, 0 }
	} );

	for( GLuint i = 0; i < 10; ++i ) {
		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size + size, i * size, 0 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1 },
			{ 0, 0, 0 }
		} );

		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size + size + size, i * size, 0 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1 },
			{ 1, 0, 0 }
		} );

		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size + size + size, i * size + size, 0 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1 },
			{ 1, 1, 0 }
		} );

		handles[ 0 ].buffer_data( SharedMesh::Vertex {
			{ i * size + size + size + size, i * size + size, 0 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1 },
			{ 0, 1, 0 }
		} );
	}

	handles[ 0 ].finalize_set( );

	std::cout << "Block select errors3: " << checkGlErrors( ) << std::endl;
}*/

void BlockSelector::init( ) {
	vbo.init( );


	std::cout << "Block select errors0: " << checkGlErrors( ) << std::endl;

	//shared_mesh.init( 8192, 100, 12288, 100 );
	//shared_mesh.get_handle( handles[ 0 ] );
	//shared_mesh.get_handle( handles[ 1 ] );

	//make_mesh( );

	//shared_mesh.unmap( );

	id_block = 0;
	is_dirty = true;
}

void BlockSelector::set_dirty( ) {
	is_dirty = true;
}

void BlockSelector::set_id_block( int id_block ) {
	this->id_block = id_block;
	set_dirty( );
}

int BlockSelector::get_id_block( ) { 
	return id_block;
}

void BlockSelector::change_id_block( int delta_id_block ) {
	id_block += delta_id_block;
	while( id_block >= client.chunk_mgr.get_num_blocks( ) ) id_block -= client.chunk_mgr.get_num_blocks( );
	while( id_block < 0 ) id_block += client.chunk_mgr.get_num_blocks( );
	set_dirty( );
}

void BlockSelector::update( ) {
	if( client.input_mgr.get_wheel_delta( ) != 0 ) {
		change_id_block( client.input_mgr.get_wheel_delta( ) );
	}

	mesh( );
}

void BlockSelector::render( ) {
	client.texture_mgr.bind_program( "Selector" );
	static GLuint prog_model = glGetUniformLocation( client.texture_mgr.id_prog, "mat_model" );

	auto & dim_window = client.display_mgr.get_window( );
	int size_active;
	float rx = 200.0f;
	float ry = 50.0f;
	float dx, dy, dz; 

	glDisable( GL_CULL_FACE );

	mat_rotate = glm::rotate( glm::mat4( 1.0f ),
		glm::radians( client.time_mgr.get_time( TimeStrings::GAME ) / 1000.0f * 180.0f / 3.0f ),
		glm::vec3( 0, 1, 0 ) );
	mat_rotate = glm::rotate( mat_rotate,
		glm::radians( 30.0f ),
		glm::vec3( 1, 0, 0 ) );

	for( int i = 0; i < num_hist; ++i ) {
		size_active = size_min + int( ( size_max - size_min ) * ( float( i ) / num_hist ) );

		dx = glm::cos( glm::radians( -50 + 140.0f / num_hist * i ) ) * rx;
		dy = ry - glm::sin( glm::radians( -50 + 140.0f / num_hist * i ) ) * ry;
		dz = -( num_hist - i + 1 ) * size_max;

		mat_scale = glm::scale( glm::mat4( 1.0f ), glm::vec3( size_active, size_active, size_active ) );
		mat_scale = glm::translate( mat_scale, glm::vec3( -0.5f, -0.5f, -0.5f ) );

		mat_translate = glm::translate(
			glm::mat4( 1.0f ),
			glm::vec3(
				dim_window.x - size_max - rx + dx,
				size_max + dy,
				dz
			) 
		);
		
		glUniformMatrix4fv( prog_model, 1, GL_FALSE, glm::value_ptr( mat_translate * mat_rotate * mat_scale ) );
		vbo.render_range( client, 1 + ( i * 2 ), 1 );

		mat_translate = glm::translate(
			glm::mat4( 1.0f ),
			glm::vec3(
				dim_window.x - size_max - rx - dx,
				size_max + dy,
				dz
			)
		);

		glUniformMatrix4fv( prog_model, 1, GL_FALSE, glm::value_ptr( mat_translate * mat_rotate * mat_scale ) );
		vbo.render_range( client, 1 + ( i * 2 ) + 1, 1 );
	}

	mat_scale = glm::scale( glm::mat4( 1.0f ), glm::vec3( size_max, size_max, size_max ) );
	mat_scale = glm::translate( mat_scale, glm::vec3( -0.5f, -0.5f, -0.5f ) );

	mat_rotate = glm::rotate( glm::mat4( 1.0f ), 
		glm::radians( client.time_mgr.get_time( TimeStrings::GAME ) / 1000.0f * 180.0f ), 
		glm::vec3( 0, 1, 0 ) );
	mat_rotate = glm::rotate( mat_rotate, glm::radians( 30.0f ), glm::vec3( 1, 0, 0 ) );

	mat_translate = glm::translate(
		glm::mat4( 1.0f ),
		glm::vec3(
			dim_window.x - size_max - rx,
			size_max,
			-size_max
		)
	);

	glUniformMatrix4fv( prog_model, 1, GL_FALSE, glm::value_ptr( mat_translate * mat_rotate * mat_scale ) );
	vbo.render_range( client, 0, 1 );

	//handles[ 0 ].render( client );
	//handles[ 1 ].render( client );
	//shared_mesh.render( client );

	glEnable( GL_CULL_FACE );
}

void BlockSelector::mesh( ) {
	if( !is_dirty ) {
		return;
	}

	static bool is_test = true;
	if( is_test == true ) { 
		
		is_test = false;
	}

	Block * block;
	glm::vec3 const * vert;
	glm::vec4 color;
	glm::vec3 const * norm;
	glm::vec3 const * uv;

	block = &client.chunk_mgr.get_block_data( id_block );

	vbo.clear( );

	vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
		"Selector", client.texture_mgr.get_texture_id( "Blocks" ),
		std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
	) );

	for( auto & face : block->faces ) { 
		for( int j = 0; j < 4; ++j ) {
			vert = &face.verts[ j ];
			color = block->color * face.color;
			norm = &face.norms[ j ];
			uv = &face.uvs[ j ];

			vbo.push_data( VBO::Vertex {
				vert->x, vert->y, vert->z,
				color.r, color.g, color.b, color.a,
				norm->x, norm->y, norm->z,
				uv->x, uv->y, uv->z
			} );
		}
	}

	for( int k = num_hist; k >= 1; --k ) {
		id_temp = id_block + k;
		while( id_temp >= client.chunk_mgr.get_num_blocks( ) ) id_temp -= client.chunk_mgr.get_num_blocks( );
		block = &client.chunk_mgr.get_block_data( id_temp );

		vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
			"Selector", client.texture_mgr.get_texture_id( "Blocks" ),
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		for( auto & face : block->faces ) {
			for( int j = 0; j < 4; ++j ) {
				vert = &face.verts[ j ];
				color = block->color * face.color;
				norm = &face.norms[ j ];
				uv = &face.uvs[ j ];

				vbo.push_data( VBO::Vertex {
					vert->x, vert->y, vert->z,
					color.r, color.g, color.b, color.a,
					norm->x, norm->y, norm->z,
					uv->x, uv->y, uv->z
				} );
			}
		}

		id_temp = id_block - k;
		while( id_temp < 0 ) id_temp += client.chunk_mgr.get_num_blocks( );
		block = &client.chunk_mgr.get_block_data( id_temp );

		vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
			"Selector", client.texture_mgr.get_texture_id( "Blocks" ),
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		for( auto & face : block->faces ) {
			for( int j = 0; j < 4; ++j ) {
				vert = &face.verts[ j ];
				color = block->color * face.color;
				norm = &face.norms[ j ];
				uv = &face.uvs[ j ];

				vbo.push_data( VBO::Vertex {
					vert->x, vert->y, vert->z,
					color.r, color.g, color.b, color.a,
					norm->x, norm->y, norm->z,
					uv->x, uv->y, uv->z
				} );
			}
		}
	}

	vbo.finalize_set( );

	client.thread_mgr.task_main( 5, [ & ] ( ) { 
		vbo.buffer( );
	} );

	is_dirty = false;
}