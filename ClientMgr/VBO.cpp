#include "VBO.h"
#include "Client.h"
#include <iostream>

GLuint const VBO::GeomNumIndsLookup[ ] = {
	1, 2, 3
};

GLuint const VBO::GeomGLTypeLookup[ ] = {
	GL_POINTS, GL_LINES, GL_TRIANGLES
};

VBO::IndexSet::IndexSet( TypeGeometry type, std::string const & str_prog,
	GLuint id_texture, std::vector< GLuint > & list_indices ) :
	type( type ),
	num_eles( 0 ),
	str_prog( str_prog ), id_tex( id_texture ),
	idx_verts( 0 ), //idx_verts_last( 0 ),
	len_verts( 0 ), //len_verts_last( 0 ),
	idx_inds( 0 ), //idx_inds_last( 0 ),
	len_inds( 0 ), //len_inds_last( 0 ),
	list_inds( list_indices ) { }

VBO::IndexSet::~IndexSet( ) { }

VBO::VBO( ) :
	index_end( 0 ) {}

VBO::~VBO( ) { }

void VBO::init( ) { 
	glGenVertexArrays( 1, &id_vao );
	glGenBuffers( 1, &id_vbo );
	glGenBuffers( 1, &id_ibo );

	glBindVertexArray( id_vao );
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );

	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	glEnableVertexAttribArray( 2 );
	glEnableVertexAttribArray( 3 );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 0 ) ); //Vert
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 12 ) ); //Color
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_TRUE, sizeof( Vertex ), BUFFER_OFFSET( 28 ) ); //Norm
	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), BUFFER_OFFSET( 40 ) ); //Uv

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ibo );
	glBindVertexArray( 0 );

	last_size_vbo = 0;
	last_size_ibo = 0;
}

void VBO::push_set( IndexSet & set_idx ) { 
	if( list_sets.size( ) != 0 ) { 
		finalize_set( );
		auto & set_last = list_sets.back( );
		set_idx.idx_inds = set_last.idx_inds + set_last.len_inds;
		set_idx.idx_verts = set_last.idx_verts + set_last.len_verts;
	}
	else { 
		set_idx.idx_inds = 0;
		set_idx.idx_verts = 0;
	}

	list_sets.push_back( set_idx );
}

void VBO::push_data( Vertex const & vert ) { 
	list_verts.emplace_back( vert );
}

void VBO::push_data( std::vector< Vertex > & data_vert ) {
	list_verts.insert( list_verts.end( ), data_vert.begin( ), data_vert.end( ) );
}

void VBO::finalize_set( ) {
	auto & set_last = list_sets.back( );

	if( set_last.list_inds.size( ) % GeomNumIndsLookup[ set_last.type ] != 0 ) {
		std::cout << "ERROR - Missmatched indices length: VBO[ " << id_vbo << " ] Id: " << list_sets.size( ) - 1 << std::endl;
		std::cout << "Expected Indices( mult ): " << GeomNumIndsLookup[ set_last.type ] << " got: " << set_last.list_inds.size( ) << std::endl;
		return;
	}

	int unsigned idx_base = set_last.idx_verts;
	int unsigned idx_max = set_last.idx_verts;
	int unsigned idx_curr = set_last.idx_verts;

	while( idx_max < list_verts.size( ) - 1 ) { 
		for( int i = 0; i < set_last.list_inds.size( ); ++i ) {
			idx_curr = idx_base + set_last.list_inds[ i ];
			if( idx_curr > idx_max ) idx_max = idx_curr;
			list_inds.push_back( idx_curr );
		}

		idx_base = idx_max + 1;
	}

	set_last.len_inds = list_inds.size( ) - set_last.idx_inds;
	set_last.len_verts = list_verts.size( ) - set_last.idx_verts;
	set_last.num_eles = set_last.len_inds / GeomNumIndsLookup[ set_last.type ];
}

void VBO::buffer( ) {
	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	glBufferData( GL_ARRAY_BUFFER, last_size_vbo, nullptr, GL_STATIC_DRAW );
	glBufferData( GL_ARRAY_BUFFER, 
		list_verts.size( ) * sizeof( Vertex ), 
		list_verts.data( ), GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, id_ibo );
	glBufferData( GL_ARRAY_BUFFER, last_size_ibo, nullptr, GL_STATIC_DRAW );
	glBufferData( GL_ARRAY_BUFFER, 
		list_inds.size( ) * sizeof( GLuint ), 
		list_inds.data( ), GL_STATIC_DRAW );

	last_size_vbo = list_verts.size( ) * sizeof( Vertex );
	last_size_ibo = list_inds.size( ) * sizeof( GLuint );
}

void VBO::render( Client & client ) {
	auto iter_set = list_sets.begin( );

	glBindVertexArray( id_vao );

	while( iter_set != list_sets.end( ) ) {
		client.texture_mgr.bind_program( iter_set->str_prog );
		client.texture_mgr.bind_texture( 0, iter_set->id_tex );

		glDrawElements( 
			GeomGLTypeLookup[ iter_set->type ], iter_set->len_inds, 
			GL_UNSIGNED_INT, BUFFER_OFFSET( iter_set->idx_inds * sizeof( GLuint ) ) );

		++iter_set;
	}

	glBindVertexArray( 0 );
}

void VBO::render_range( Client & client, GLuint index, GLuint length ) { 
	auto iter_set = list_sets.begin( ) + index;
	GLuint curr_length = 0;

	glBindVertexArray( id_vao );

	while( iter_set < list_sets.end( ) && curr_length < length ) {
		client.texture_mgr.bind_program( iter_set->str_prog );
		client.texture_mgr.bind_texture( 0, iter_set->id_tex );

		glDrawElements(
			GeomGLTypeLookup[ iter_set->type ], iter_set->len_inds,
			GL_UNSIGNED_INT, BUFFER_OFFSET( iter_set->idx_inds * sizeof( GLuint ) ) );

		++iter_set;
		++curr_length;
	}

	glBindVertexArray( 0 );
}

void VBO::clear( ) { 
	list_inds.clear( );
	list_verts.clear( );
	list_sets.clear( );
	index_end = 0;
}