#include "Page.h"

#include "Client.h"

Page::Page( ) :
	client( nullptr ),
	vec_pos( 0, 0 ),
	vec_dim( 0, 0 ),
	is_hold( false ),
	is_edit( false ),
	is_visibile( true ) {
}

const glm::vec3 verts_page[ 4 ] = { 
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 } 
};

Page::~Page( ) { }

void Page::add_comp( std::string & str_name,
	FuncComp func_alloc, FuncComp func_custom ) { 
	Handle< PageComp > handle_comp;

	if( client->resource_mgr.allocate( handle_comp ) ) { 
		auto & comp = handle_comp.get();
		comp.parent = this;
		comp.client = client;
		comp.str_name = str_name;

		if( func_alloc( comp ) &&
			func_custom( comp ) ) {
			list_comps.push_back( handle_comp );
			map_comps.insert( { str_name, &comp } );
		}
	}
}

PageComp & Page::get_comp( std::string & str_name ) { 
	return *map_comps[ str_name ];
}

bool Page::on_down( int button ) {
	auto pos_mouse = client->input_mgr.get_mouse_down( button );
	glm::ivec2 pos_local =  glm::ivec2( pos_mouse.x, pos_mouse.y ) - vec_pos;
	bool is_handled = false;
	int i = list_comps.size( ) - 1;

	while( !is_handled && i >= 0 ) { 
		auto & comp = list_comps[ i ].get();

		if( comp.is_visible ) {
			if( Directional::is_point_in_rect( pos_local, comp.vec_pos, comp.vec_pos + comp.vec_dim ) ) {
				if( comp.func_on_down ) {
					is_handled = comp.func_on_down( comp );
				}
			}
		}

		i--;
	}

	return true;
}

bool Page::on_hold( int button ) { 
	bool is_handled = false;
	int i = list_comps.size( ) - 1;

	while( !is_handled && i >= 0 ) {
		auto & comp = list_comps[ i ].get( );

		if( comp.is_visible ) {
			if( comp.func_on_hold ) {
				is_handled = comp.func_on_hold( comp );
			}
		}

		i--;
	}

	return false;
}

bool Page::on_up( int  button ) {
	bool is_handled = false;
	int i = list_comps.size( ) - 1;

	while( !is_handled && i >= 0 ) {
		auto & comp = list_comps[ i ].get( );

		if( comp.is_visible ) {
			if( comp.func_on_up ) {
				is_handled = comp.func_on_up( comp );
			}
		}

		i--;
	}

	return false;
}

void Page::update( ) { 
	if( ++cnt_update % 4 == 0 ) is_dirty = true;

	mat_model = glm::translate( glm::mat4( 1.0f ), glm::vec3( vec_pos.x, vec_pos.y, 0.0f ) );

	for( int i = 0; i < list_comps.size( ); i++ ) {
		auto & comp = list_comps[ i ].get( );

		if( comp.func_update ) {
			comp.func_update( comp );
		}
	}

	if( is_dirty ) { 
		vbo.clear( );

		vbo.push_set( VBO::IndexSet( 
			VBO::TypeGeometry::TG_Triangles,
			"Basic", client->texture_mgr.id_materials,
			std::vector< GLuint > { 0, 1, 2, 2, 3, 0 }
		) );

		auto & norm = Directional::get_vec_dir_f( FaceDirection::FD_Front );

		for( int i = 0; i < 4; ++i ) {
			vbo.push_data( VBO::Vertex {
				verts_page[ i ].x * vec_dim.x,
				verts_page[ i ].y * vec_dim.y,
				verts_page[ i ].z,
				color.r, color.g, color.b, color.a,
				norm.x, norm.y, norm.z,
				0, 0, 0
			} );
		}

		for( int i = 0; i < list_comps.size( ); ++i ) {
			auto & comp = list_comps[ i ].get( );
			if( !comp.is_visible ) {
				continue;
			}

			for( int j = 0; j < 4; ++j ) {
				vbo.push_data( VBO::Vertex {
					comp.vec_pos.x + verts_page[ j ].x * comp.vec_dim.x,
					comp.vec_pos.y + verts_page[ j ].y * comp.vec_dim.y,
					verts_page[ j ].z,
					comp.color.r, comp.color.g, comp.color.b, comp.color.a,
					norm.x, norm.y, norm.z,
					0, 0, 0
				} );
			}
		}

		vbo.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"Basic", client->texture_mgr.id_fonts,
			std::vector< GLuint > { 0, 1, 2, 2, 3, 0 }
		) );

		auto & map_data = get_map_data< PCDTextField >( );
		auto iter_data = map_data.begin( );

		for( iter_data; iter_data != map_data.end( ); iter_data++ ) {
			auto & data_text = ( ( Handle< PCDTextField > * ) iter_data->second )->get( );
			glm::ivec2 vec_size_text( data_text.size_text * 2 / 3, data_text.size_text );
			glm::ivec2 vec_pos_char;

			for( int i = 0; i < data_text.ptr_str->size( ); i++ ) {
				auto & uvs = client->texture_mgr.get_uvs_fonts( data_text.ptr_str->at( i ) - 32 );
				vec_pos_char = *data_text.ptr_vec_pos + data_text.vec_offset + glm::ivec2( vec_size_text.x * i, 0 );

				if( vec_pos_char.x > vec_dim.x - vec_size_text.x - 5 ) break;

				for( int j = 0; j < 4; ++j ) { 
					vbo.push_data( VBO::Vertex { 
						vec_pos_char.x + verts_page[ j ].x * vec_size_text.x,
						vec_pos_char.y + verts_page[ j ].y * vec_size_text.y,
						verts_page[ j ].z,
						0.0f, 0.0f, 0.0f, 1.0f,
						norm.x, norm.y, norm.z,
						uvs[ j ][ 0 ], uvs[ j ][ 1 ], 0
					} );
				}
			}
		}

		vbo.finalize_set( );

		client->thread_mgr.task_main( 5, [ & ] ( ) { 
			vbo.buffer( );
		} );

		is_dirty = false;
	}
}

void Page::resize( ) { 
	for( int i = 0; i < list_comps.size( ); i++ ) { 
		list_comps[ i ].get( ).position( );
	}
}