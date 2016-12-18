#include "TestComp.h"

TestComp::TestComp( Client & client ) {
	name = "Test";

	func_alloc = [ 
		&client = client 
	] ( PComp * comp ) {
		comp->is_visible = true;
		comp->is_hold = false;

		comp->pos = { 0, 0 };
		comp->dim = { 50, 50 };

		comp->add_data< CompData >( client );
		auto & comp_data = comp->get_data< CompData >( );

		comp_data.color = { 0.5f, 0.5f, 1.0f, 0.5f };

		return 0;
	};

	func_down = [ ] ( PComp * comp ) { 
		printf( "Mouse down on component: %s in Page: %s\n", comp->name.c_str( ), comp->page->name.c_str( ) );

		comp->is_hold = true;

		return 1;
	};

	func_hold = [
		&client = client
	] ( PComp * comp ) { 
		if( comp->is_hold ) {
			glm::ivec2 & pos_mouse = client.input_mgr.get_mouse( );
			glm::ivec2 new_pos = comp->pos + client.input_mgr.get_mouse_delta( );

			if( new_pos.x < 0 ) new_pos.x = 0;
			if( new_pos.x + comp->dim.x > comp->page->dim.x ) new_pos.x = comp->page->dim.x - comp->dim.x;
			if( new_pos.y < 0 ) new_pos.y = 0;
			if( new_pos.y + comp->dim.y > comp->page->dim.y ) new_pos.y = comp->page->dim.y - comp->dim.y;

			comp->offset.x = new_pos.x - comp->page->dim.x * comp->anchor.x;
			comp->offset.y = new_pos.y - comp->page->dim.y * comp->anchor.y;

			if( pos_mouse.x < comp->page->pos.x ) pos_mouse.x = comp->page->pos.x;
			if( pos_mouse.x > comp->page->pos.x + comp->page->dim.x ) pos_mouse.x = comp->page->pos.x + comp->page->dim.x;

			if( pos_mouse.y < comp->page->pos.y ) pos_mouse.y = comp->page->pos.y;
			if( pos_mouse.y > comp->page->pos.y + comp->page->dim.y ) pos_mouse.y = comp->page->pos.y + comp->page->dim.y;

			client.input_mgr.set_mouse( pos_mouse );

			comp->page->is_remesh = true;

			return 1;
		}

		return 0;
	};

	func_up = [ ] ( PComp * comp ) {
		printf( "Mouse up on component: %s in Page: %s\n", comp->name.c_str( ), comp->page->name.c_str( ) );

		comp->is_hold = false;

		return 1;
	};

	func_mesh = [ 
		&client = client,
		id_texture = client.texture_mgr.get_texture_id( "Materials" ),
		id_subtex = client.texture_mgr.get_texture_layer( "Materials", "Details/Solid" )
	] ( PComp * comp ) {
		auto & color = comp->get_data< CompData >( ).color;

		comp->page->vbo_mesh.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicOrtho",
			id_texture,
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex >{
			{ { comp->pos.x,				comp->pos.y,					0 },
			color,
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, id_subtex }
			},

			{ { comp->pos.x + comp->dim.x,	comp->pos.y,					0 },
			color,
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, id_subtex } },

			{ { comp->pos.x + comp->dim.x,	comp->pos.y + comp->dim.y,		0 },
			color,
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, id_subtex } },

			{ { comp->pos.x,				comp->pos.y + comp->dim.y,		0 },
			color,
			{ 0.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, id_subtex } }
		} );

		comp->page->vbo_mesh.finalize_set( );

		return 0;
	};
}


TestComp::~TestComp( ) { }
