#include "UpdatedLabelComp.h"

#include "TextureMgr.h"
#include "ResourceMgr.h"

#include "Page.h"
#include "PageComponent.h"

#include "Shapes.h"

UpdatedLabelComp::UpdatedLabelComp( Client & client ) {
	name = "UpdatedLabel";

	func_register = [ &client = client ] ( ) {
		if( client.resource_mgr.reg_pool< UpdatedLabelData >( num_comp_default ) ) {
			return 0;
		}

		return 1;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		auto data = comp->add_data< UpdatedLabelData >( );

		data->text.clear( );
		data->text = "Default";
		data->size_text = 12;
		data->alignment_h = UpdatedLabelData::AlignHorizontal::AH_Right;
		data->alignment_v = UpdatedLabelData::AlignVertical::AV_Top;

		data->func_update = [] ( ) { return ""; };

		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) { 
		auto data = comp->get< UpdatedLabelData >( );
		data->text = data->func_update( );

		return 1;
	};

	func_mesh = [ &client = client ] ( PComp * comp ) {
		comp->page->vbo_mesh.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Triangles,
			"BasicOrtho", client.texture_mgr.get_texture_id( "Fonts" ),
			std::vector< GLuint > { 0, 1, 2, 2, 3, 0 }
		) );

		auto data = comp->get< UpdatedLabelData >( );

		glm::ivec2 dim_text( data->size_text * 2 / 3, data->size_text );
		glm::ivec2 pos_char;
		int unsigned id_subtex = client.texture_mgr.get_texture_layer( "Fonts", "Default/Basic" );

		for( int i = 0; i < data->text.size( ); i++ ) {
			auto & uvs = client.texture_mgr.get_uvs_fonts( data->text.at( i ) - 32 );

			pos_char = comp->pos;

			if( data->alignment_h == UpdatedLabelData::AlignHorizontal::AH_Left ) {
				pos_char.x -= ( int ) data->text.size( ) * dim_text.x;
			}
			else if( data->alignment_h == UpdatedLabelData::AlignHorizontal::AH_Center ) {
				pos_char.x -= ( int ) data->text.size( ) * dim_text.x / 2;
			}

			if( data->alignment_v == UpdatedLabelData::AlignVertical::AV_Bottom ) {
				pos_char.y -= dim_text.y;
			}
			else if( data->alignment_v == UpdatedLabelData::AlignVertical::AV_Center ) {
				pos_char.y -= dim_text.y / 2;
			}

			pos_char.x += dim_text.x * i;

			for( int j = 0; j < 4; ++j ) {
				comp->page->vbo_mesh.push_data( VBO::Vertex {
					{ pos_char.x + Shapes::Verts::quad[ j ].x * dim_text.x,
					pos_char.y + Shapes::Verts::quad[ j ].y * dim_text.y,
					Shapes::Verts::quad[ j ].z },
					{ 0.0f, 0.0f, 0.0f, 1.0f },
					{ 0.0f, 0.0f, 1.0f },
					{ uvs[ j ][ 0 ], uvs[ j ][ 1 ], id_subtex }
				} );
			}
		}

		comp->page->vbo_mesh.finalize_set( );

		return 1;
	};
}

UpdatedLabelComp::~UpdatedLabelComp( ) { }
