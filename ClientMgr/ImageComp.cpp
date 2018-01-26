#include "ImageComp.h"

#include "ResourceMgr.h"
#include "TextureMgr.h"

#include "Page.h"

ImageComp::ImageComp( Client & client ) {
	name = "Image";

	func_register = [ &client = client ] ( ) {
		if( client.resource_mgr.reg_pool< ImageData >( num_comp_default ) ) {
			return 0;
		}

		return 1;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->anchor = { 0.5f, 0.5f };
		comp->dim = { 50, 50 };
		comp->offset = -comp->dim / 2;

		auto data = comp->add_data< ImageData >( );
		data->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		data->id_texture = client.texture_mgr.get_texture_id( "Gui" );
		data->id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/Image" );

		return 0;
	};

	func_mesh = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get< ImageData >( );
		auto & color = data->color;
		glm::vec3 norm = { 0.0f, 0.0f, 1.0f };

		comp->page->vbo_mesh.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicOrtho",
			data->id_texture,
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
			{ { comp->pos.x,					comp->pos.y, 0.0f },					color, norm, { 0.0f, 0.0f, data->id_subtex } },
			{ { comp->pos.x + comp->dim.x,		comp->pos.y, 0.0f },					color, norm, { 1.0f, 0.0f, data->id_subtex } },
			{ { comp->pos.x + comp->dim.x,		comp->pos.y + comp->dim.y, 0.0f },		color, norm, { 1.0f, 1.0f, data->id_subtex } },
			{ { comp->pos.x,					comp->pos.y + comp->dim.y, 0.0f },		color, norm, { 0.0f, 1.0f, data->id_subtex } },
		} );

		comp->page->vbo_mesh.finalize_set( );

		return 0;
	};
}


ImageComp::~ImageComp( ) { }
