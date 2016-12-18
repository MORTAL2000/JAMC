#include "TestPage.h"

TestPage::TestPage( Client & client ) {
	name = "Test";

	func_alloc = [ &client = client ] ( Page * page ) { 
		page->is_visible = true;
		page->is_remesh = true;
		page->is_hold = false;

		page->anchor = { 0.5f, 0.5f };
		page->offset = { 0, 0 };
		page->dim = { 100, 100 };

		page->add_data< TestData >( client );
		auto & color = page->get_data< TestData >( ).color;
		color = { 1.0f, 1.0f, 1.0f, 0.5f };

		/*
		page->add_comp( "Test", "Test", [ ] ( PComp * comp ) {
			comp->anchor = { 0.5f, 0.5f };
			return 0;
		} );
		*/

		return 0;
	};

	func_release = func_null;

	func_mesh = [ 
		id_subtex1 = client.texture_mgr.get_texture_layer( "Materials", "Details/Solid" ),
		id_subtex2 = client.texture_mgr.get_texture_layer( "Gui", "Default/PageBG" )
	] ( Page * page ) {
		page->vbo_mesh.clear( );

		glm::vec4 & color = page->get_data< TestData >( ).color;
		glm::vec3 norm = { 0.0f, 0.0f, 1.0f };

		/*
		{
			page->vbo_mesh.push_set( VBO::IndexSet(
				VBO::TypeGeometry::TG_Triangles,
				"BasicOrtho",
				page->client->texture_mgr.get_texture_id( "Materials" ),
				std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
			) );

			page->vbo_mesh.push_data( std::vector< VBO::Vertex >{
				{ { 0, 0, 0 },
				color,
				norm,
				{ 0.0f, 0.0f, id_subtex1 }
				},

				{ { page->dim.x,	0,				0 },
				color,
				norm,
				{ 0.0f, 0.0f, id_subtex1 } },

				{ { page->dim.x,	page->dim.y,	0 },
				color,
				norm,
				{ 0.0f, 0.0f, id_subtex1 } },

				{ { 0,				page->dim.y,	0 },
				color,
				norm,
				{ 0.0f, 0.0f, id_subtex1 } }
			} );

			page->vbo_mesh.finalize_set( );
		}
		*/

		{
			int padding = 18;

			float d_pix = 1.0f / 64.0f;
			float dh_pix = d_pix / 2.0f;

			float d_uv = d_pix * 9.0f;

			page->vbo_mesh.push_set( VBO::IndexSet(
				VBO::TypeGeometry::TG_Triangles,
				"BasicOrtho",
				page->client->texture_mgr.get_texture_id( "Gui" ),
				std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
			) );

			page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
				// Corners BL/BR/UR/UL
				{ { 0.0f,						0.0f, 0.0f },						color, norm, { dh_pix,					dh_pix,						id_subtex2 } },
				{ { 0.0f + padding,				0.0f, 0.0f },						color, norm, { dh_pix + d_uv,			dh_pix,						id_subtex2 } },
				{ { 0.0f + padding,				0.0f + padding, 0.0f },				color, norm, { dh_pix + d_uv,			dh_pix + d_uv,				id_subtex2 } },
				{ { 0.0f,						0.0f + padding, 0.0f },				color, norm, { dh_pix,					dh_pix + d_uv,				id_subtex2 } },

				{ { page->dim.x - padding,		0.0f, 0.0f },						color, norm, { 1.0f - dh_pix - d_uv,	dh_pix,						id_subtex2 } },
				{ { page->dim.x,				0.0f, 0.0f },						color, norm, { 1.0f - dh_pix,			dh_pix,						id_subtex2 } },
				{ { page->dim.x,				0.0f + padding, 0.0f },				color, norm, { 1.0f - dh_pix,			dh_pix + d_uv,				id_subtex2 } },
				{ { page->dim.x - padding,		0.0f + padding, 0.0f },				color, norm, { 1.0f - dh_pix - d_uv,	dh_pix + d_uv,				id_subtex2 } },

				{ { page->dim.x - padding,		page->dim.y - padding, 0.0f },		color, norm, { 1.0f - dh_pix - d_uv,	1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { page->dim.x,				page->dim.y - padding, 0.0f },		color, norm, { 1.0f - dh_pix,			1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { page->dim.x,				page->dim.y, 0.0f },				color, norm, { 1.0f - dh_pix,			1.0f - dh_pix,				id_subtex2 } },
				{ { page->dim.x - padding,		page->dim.y, 0.0f },				color, norm, { 1.0f - dh_pix - d_uv,	1.0f - dh_pix,				id_subtex2 } },

				{ { 0.0f,						page->dim.y - padding, 0.0f },		color, norm, { dh_pix,					1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { 0.0f + padding,				page->dim.y - padding, 0.0f },		color, norm, { dh_pix + d_uv,			1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { 0.0f + padding,				page->dim.y, 0.0f },				color, norm, { dh_pix + d_uv,			1.0f - dh_pix,				id_subtex2 } },
				{ { 0.0f,						page->dim.y, 0.0f },				color, norm, { dh_pix,					1.0f - dh_pix,				id_subtex2 } },

				// Edges B/R/U/L
				{ { padding,					0.0f, 0.0f },						color, norm, { dh_pix + d_uv,			dh_pix,						id_subtex2 } },
				{ { page->dim.x - padding,		0.0f, 0.0f },						color, norm, { 1.0f - dh_pix - d_uv,	dh_pix,						id_subtex2 } },
				{ { page->dim.x - padding,		padding, 0.0f },					color, norm, { 1.0f - dh_pix - d_uv,	dh_pix + d_uv,				id_subtex2 } },
				{ { padding,					padding, 0.0f },					color, norm, { dh_pix + d_uv,			dh_pix + d_uv,				id_subtex2 } },

				{ { page->dim.x - padding,		padding, 0.0f },					color, norm, { 1.0f - dh_pix - d_uv,	dh_pix + d_uv,				id_subtex2 } },
				{ { page->dim.x,				padding, 0.0f },					color, norm, { 1.0f - dh_pix,			dh_pix + d_uv,				id_subtex2 } },
				{ { page->dim.x,				page->dim.y - padding, 0.0f },		color, norm, { 1.0f - dh_pix,			1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { page->dim.x - padding,		page->dim.y - padding, 0.0f },		color, norm, { 1.0f - dh_pix - d_uv,	1.0f - dh_pix - d_uv,		id_subtex2 } },

				{ { padding,					page->dim.y - padding, 0.0f },		color, norm, { dh_pix + d_uv,			1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { page->dim.x - padding,		page->dim.y - padding, 0.0f },		color, norm, { 1.0f - dh_pix - d_uv,	1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { page->dim.x - padding,		page->dim.y, 0.0f },				color, norm, { 1.0f - dh_pix - d_uv,	1.0f - dh_pix,				id_subtex2 } },
				{ { padding,					page->dim.y, 0.0f },				color, norm, { dh_pix + d_uv,			1.0f - dh_pix,				id_subtex2 } },

				{ { 0.0f,						padding, 0.0f },					color, norm, { dh_pix,					dh_pix + d_uv,				id_subtex2 } },
				{ { padding,					padding, 0.0f },					color, norm, { dh_pix + d_uv,			dh_pix + d_uv,				id_subtex2 } },
				{ { padding,					page->dim.y - padding, 0.0f },		color, norm, { dh_pix + d_uv,			1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { 0.0f,						page->dim.y - padding, 0.0f },		color, norm, { dh_pix,					1.0f - dh_pix - d_uv,		id_subtex2 } },

				// Center
				{ { padding,					padding, 0.0f },					color, norm, { dh_pix + d_uv,			dh_pix + d_uv,				id_subtex2 } },
				{ { page->dim.x - padding,		padding, 0.0f },					color, norm, { 1.0f - dh_pix - d_uv,	dh_pix + d_uv,				id_subtex2 } },
				{ { page->dim.x - padding,		page->dim.y - padding, 0.0f },		color, norm, { 1.0f - dh_pix - d_uv,	1.0f - dh_pix - d_uv,		id_subtex2 } },
				{ { padding,					page->dim.y - padding, 0.0f },		color, norm, { dh_pix + d_uv,			1.0f - dh_pix - d_uv,		id_subtex2 } },
			} );

			page->vbo_mesh.finalize_set( );
		}

		PageLoader::func_mesh_comps( page );

		page->vbo_mesh.buffer( );

		return 0;
	};

	func_update = [ ] ( Page * page ) {
		page->mat_model = glm::translate( glm::mat4( ), glm::vec3( page->pos, 0 ) );

		PageLoader::func_update_comps( page );

		return 0;
	};
}


TestPage::~TestPage( ) { }
