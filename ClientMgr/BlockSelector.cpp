#include "BlockSelector.h"
#include "Client.h"
#include "glm/gtc/type_ptr.hpp"

BlockSelector::BlockSelector( Client & client ) :
	client( client ){

}

BlockSelector::~BlockSelector( ) { 
	
}

void BlockSelector::init( ) {
	vbo.init( );
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

	glEnable( GL_CULL_FACE );
}

void BlockSelector::mesh( ) {
	if( !is_dirty ) {
		return;
	}

	FaceVerts const * verts;
	Color4 * color;
	glm::vec3 const * normal;
	FaceUvs const * uvs;
	Block * block;
	FaceDirection dir;

	vbo.clear( );

	block = &client.chunk_mgr.get_block_data( id_block );

	vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
		"Selector", client.texture_mgr.id_terrain,
		std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
	) );

	for( int i = 0; i < FaceDirection::FD_Size; ++i ) {
		dir = ( FaceDirection ) i;
		verts = &Block::get_verts( dir );
		color = &block->color;
		normal = &Directional::get_vec_dir_f( dir );
		uvs = &block->get_uvs( dir );

		for( int j = 0; j < 4; ++j ) {
			vbo.push_data( VBO::Vertex {
				( *verts )[ j ][ 0 ],
				( *verts )[ j ][ 1 ],
				( *verts )[ j ][ 2 ],

				color->r, color->g, color->b, color->a,
				normal->x, normal->y, normal->z,
				( *uvs )[ j ][ 0 ], ( *uvs )[ j ][ 1 ], ( *uvs )[ j ][ 2 ]
			} );
		}
	}

	for( int k = num_hist; k >= 1; --k ) {
		block = &client.chunk_mgr.get_block_data( ( id_block + k ) % client.chunk_mgr.get_num_blocks( ) );

		vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
			"Selector", client.texture_mgr.id_terrain,
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		for( int i = 0; i < FaceDirection::FD_Size; ++i ) {
			dir = ( FaceDirection ) i;
			verts = &Block::get_verts( dir );
			color = &block->color;
			normal = &Directional::get_vec_dir_f( dir );
			uvs = &block->get_uvs( dir );

			for( int j = 0; j < 4; ++j ) {
				vbo.push_data( VBO::Vertex {
					( *verts )[ j ][ 0 ],
					( *verts )[ j ][ 1 ],
					( *verts )[ j ][ 2 ],

					color->r, color->g, color->b, color->a,
					normal->x, normal->y, normal->z,
					( *uvs )[ j ][ 0 ], ( *uvs )[ j ][ 1 ], ( *uvs )[ j ][ 2 ]
				} );
			}
		}

		block = &client.chunk_mgr.get_block_data( ( id_block - k + client.chunk_mgr.get_num_blocks( ) ) % client.chunk_mgr.get_num_blocks( ) );

		vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
			"Selector", client.texture_mgr.id_terrain,
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		for( int i = 0; i < FaceDirection::FD_Size; ++i ) {
			dir = ( FaceDirection ) i;
			verts = &Block::get_verts( dir );
			color = &block->color;
			normal = &Directional::get_vec_dir_f( dir );
			uvs = &block->get_uvs( dir );

			for( int j = 0; j < 4; ++j ) {
				vbo.push_data( VBO::Vertex {
					( *verts )[ j ][ 0 ],
					( *verts )[ j ][ 1 ],
					( *verts )[ j ][ 2 ],

					color->r, color->g, color->b, color->a,
					normal->x, normal->y, normal->z,
					( *uvs )[ j ][ 0 ], ( *uvs )[ j ][ 1 ], ( *uvs )[ j ][ 2 ]
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