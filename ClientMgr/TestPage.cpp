#include "TestPage.h"

#include "LabelComp.h"
#include "TextButtonComp.h"
#include "BorderImageComp.h"

TestPage::TestPage( Client & client ) {
	name = "Test";

	func_alloc = [ &client = client ] ( Page * page ) { 
		page->is_visible = true;
		page->is_remesh = true;
		page->is_hold = false;

		page->anchor = { 0.5f, 0.5f };
		page->offset = { 0, 0 };
		page->dim = { 500, 500 };

		page->add_data< TestData >( client );
		auto & color = page->get_data< TestData >( )->color;
		color = { 1.0f, 1.0f, 1.0f, 0.5f };

		page->add_comp( "Border", "BorderImage", [ ] ( PComp * comp ) { 
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );

			data->func_resize = [ ] ( PComp * comp ) {
				comp->dim = comp->page->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		page->add_comp( "TestButton", "Button", [ ] ( PComp * comp ) {
			comp->anchor = { 0.5f, 0.5f };

			auto button_data = comp->get_data< TextButtonComp::ButtonData >( );
			auto label_data = button_data->comp_label->get_data< LabelComp::LabelData >( );

			label_data->text = "TestButton";

			comp->page->is_remesh = true;

			return 0;
		} );

		page->add_comp( "TitleLabel", "Label", [ ] ( PComp * comp ) { 
			auto data = comp->get_data< LabelComp::LabelData >( );

			data->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;

			data->text = comp->page->name + " Page";
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 10, -10 };
			comp->page->is_remesh = true;

			return 0;
		} );

		page->add_comp( "TestImage", "Image", [ ] ( PComp * comp ) { 
			comp->anchor = { 1.0f, 1.0f };
			comp->offset = -comp->dim - glm::ivec2( 10, 10 );

			return 0;
		} );

		page->add_comp( "TestCheck", "Checkbox", [ ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 0.0f };
			comp->offset = { 25, 25 };

			return 0;
		} );

		
		page->add_comp( "TestResize", "Resize", [ ] ( PComp * comp ) { return 0; } );

		return 0;
	};

	func_release = func_null;

	func_mesh = [] ( Page * page ) {
		/*
		glm::vec4 & color = page->get_data< TestData >( ).color;
		glm::vec3 norm = { 0.0f, 0.0f, 1.0f };

		int padding = 8;

		float d_pix = 1.0f / 64.0f;
		float dh_pix = 0.0f;

		float d_uv = d_pix * 8.0f;

		float p_x[ ] = { 0.0f, padding, page->dim.x - padding, page->dim.x };
		float p_y[ ] = { 0.0f, padding, page->dim.y - padding, page->dim.y };
		float p_uv[ ] = { dh_pix, dh_pix + d_uv, 1.0f - dh_pix - d_uv, 1.0f - dh_pix };

		page->vbo_mesh.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicOrtho",
			page->client->texture_mgr.get_texture_id( "Gui" ),
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
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

		page->vbo_mesh.finalize_set( );
		*/

		return 0;
	};

	func_update = [ ] ( Page * page ) {
		page->mat_model = glm::translate( glm::mat4( ), glm::vec3( page->pos, 0 ) );

		return 0;
	};
}


TestPage::~TestPage( ) { }
