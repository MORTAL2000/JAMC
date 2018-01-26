#include "BorderImageComp.h"

#include "ResizableComp.h"

#include "Page.h"
#include "PageComponent.h"

#include "ResourceMgr.h"

BorderImageComp::BorderImageComp( Client & client ) { 
	name = "BorderImage";

	func_register = [ &client = client ] ( ) {
		if( client.resource_mgr.reg_pool< BorderImageData >( num_comp_default ) ) {
			return 0;
		}

		return 1;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) { 
		comp->anchor = { 0.5f, 0.5f };
		comp->dim = { 100, 100 };
		comp->offset = -comp->dim / 2;

		auto data = comp->add_data< BorderImageData >( );
		data->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		data->padding_border = 8;
		data->set_texture( client, "Gui", "Default/PageBG", 8 );

		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) {

		return 0;
	};

	func_mesh = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get< BorderImageData >( );
		auto & color = data->color;
		glm::vec3 norm = { 0.0f, 0.0f, 1.0f };

		float p_x[ ] = { 
			( float ) comp->pos.x, comp->pos.x + data->padding_border,
			( float ) comp->pos.x + comp->dim.x - data->padding_border, ( float ) comp->pos.x + comp->dim.x };

		float p_y[ ] = { 
			( float ) comp->pos.y, comp->pos.y + data->padding_border,
			( float ) comp->pos.y + comp->dim.y - data->padding_border, ( float ) comp->pos.y + comp->dim.y };

		float p_uv[ ] = { 
			data->dh_pix, data->dh_pix + data->d_uv,
			1.0f - data->dh_pix - data->d_uv, 1.0f - data->dh_pix };

		int unsigned id_subtex = data->id_subtex;

		comp->page->vbo_mesh.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicOrtho",
			data->id_texture,
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

		return 0;
	};
}


BorderImageComp::~BorderImageComp( ) { }
