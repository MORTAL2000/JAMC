#include "TextButtonComp.h"

#include "Client.h"
#include "LabelComp.h"

TextButtonComp::TextButtonComp( Client & client ) {
	name = "Button";

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->is_visible = true;
		comp->is_hold = false;

		comp->pos = { 0, 0 };
		comp->dim = { 100, 32 };
		comp->offset = -comp->dim / 2;

		comp->add_data< ButtonData >( client );

		auto & button_data = comp->get_data< ButtonData >( );
		button_data.func_action = func_null;
		button_data.color_down = { 1.0f, 0.5f, 0.5f, 0.75f };
		button_data.color_up = { 1.0f, 1.0f, 1.0f, 0.75f };
		button_data.color = button_data.color_up;
		button_data.id_texture = client.texture_mgr.get_texture_id( "Gui" );
		button_data.id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/PageBG" );

		comp->add_comp( "Label", "Label", func_null );

		PComp * label_comp = comp->get_comp( "Label" );
		button_data.comp_label = label_comp;
		label_comp->anchor = { 0.5f, 0.5f };

		auto & label_data = label_comp->get_data< LabelComp::LabelData >( );
		label_data.size_text = 12;
		label_data.alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Center;
		label_data.alignment_v = LabelComp::LabelData::AlignVertical::AV_Center;

		return 0;
	};

	func_down = [ &client = client ] ( PComp * comp ) {
		auto & button_data = comp->get_data< ButtonData >( );
		button_data.color = button_data.color_down;
		comp->page->is_remesh = true;

		return 1;
	};

	func_hold = [ &client = client ] ( PComp * comp ) {
		return 0;
	};

	func_up = [ ] ( PComp * comp ) {
		auto & button_data = comp->get_data< ButtonData >( );
		button_data.color = button_data.color_up;
		comp->page->is_remesh = true;

		button_data.func_action( comp );

		return 1;
	};

	func_mesh = [ &client = client ] ( PComp * comp ) {
		auto & button_data = comp->get_data< ButtonData >( );
		auto & color = button_data.color;
		glm::vec3 norm = { 0.0f, 0.0f, 1.0f };

		int padding = 8;

		float d_pix = 1.0f / 64.0f;
		float dh_pix = 0.0f;

		float d_uv = d_pix * 8.0f;

		float p_x[ ] = { comp->pos.x, comp->pos.x + padding, comp->pos.x + comp->dim.x - padding, comp->pos.x + comp->dim.x };
		float p_y[ ] = { comp->pos.y, comp->pos.y + padding, comp->pos.y + comp->dim.y - padding, comp->pos.y + comp->dim.y };
		float p_uv[ ] = { dh_pix, dh_pix + d_uv, 1.0f - dh_pix - d_uv, 1.0f - dh_pix };

		int unsigned id_subtex = button_data.id_subtex;

		comp->page->vbo_mesh.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicOrtho",
			client.texture_mgr.get_texture_id( "Gui" ),
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
			// Corners BL/BR/UR/UL
			{ { p_x[ 0 ], p_y[ 0 ], 0.0f }, color, norm, { p_uv[ 0 ], p_uv[ 0 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 0 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 0 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 0 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 0 ], p_uv[ 1 ], id_subtex } },

			{ { p_x[ 2 ], p_y[ 0 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 0 ], id_subtex } },
			{ { p_x[ 3 ], p_y[ 0 ], 0.0f }, color, norm, { p_uv[ 3 ], p_uv[ 0 ], id_subtex } },
			{ { p_x[ 3 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 3 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 1 ], id_subtex } },

			{ { p_x[ 2 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 3 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 3 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 3 ], p_y[ 3 ], 0.0f }, color, norm, { p_uv[ 3 ], p_uv[ 3 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 3 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 3 ], id_subtex } },

			{ { p_x[ 0 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 0 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 3 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 3 ], id_subtex } },
			{ { p_x[ 0 ], p_y[ 3 ], 0.0f }, color, norm, { p_uv[ 0 ], p_uv[ 3 ], id_subtex } },

			// Edges B/R/U/L
			{ { p_x[ 1 ], p_y[ 0 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 0 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 0 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 0 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 1 ], id_subtex } },

			{ { p_x[ 2 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 3 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 3 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 3 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 3 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 2 ], id_subtex } },

			{ { p_x[ 1 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 3 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 3 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 3 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 3 ], id_subtex } },

			{ { p_x[ 0 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 0 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 0 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 0 ], p_uv[ 2 ], id_subtex } },

			// Center
			{ { p_x[ 1 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 1 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 1 ], id_subtex } },
			{ { p_x[ 2 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 2 ], p_uv[ 2 ], id_subtex } },
			{ { p_x[ 1 ], p_y[ 2 ], 0.0f }, color, norm, { p_uv[ 1 ], p_uv[ 2 ], id_subtex } },
		} );

		comp->page->vbo_mesh.finalize_set( );

		return 1;
	};
}


TextButtonComp::~TextButtonComp( ) { }
