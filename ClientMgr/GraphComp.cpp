#include "GraphComp.h"
#include "Page.h"


GraphComp::GraphComp( Client & client ) { 
	name = "Graph";

	func_register = [ &client = client ] () {
		if( !client.resource_mgr.reg_pool< GraphData >( 128 ) ) { 
			return 1;
		}

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->anchor = { 0.5f, 0.5f };

		auto data = comp->add_data< GraphData >( );
		data->num_entries = 256;
		data->updates = 0;
		data->record = &client.time_mgr.get_record( RecordStrings::RENDER );

		data->comp_label_title = comp->add_comp( "LabelTitle", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 5.0f, -5.0f };

			data->data_label_title = comp->get_data< LabelComp::LabelData >( );
			data->padding = { 10, 10 };
			data->data_label_title->size_text = 16;
			data->data_label_title->text = "Default Graph";
			data->data_label_title->alignment_h = LabelComp::LabelData::AH_Right;
			data->data_label_title->alignment_v = LabelComp::LabelData::AV_Bottom;

			return 0;
		} );



		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) {
		comp->page->is_remesh = true;

		return 0;
	};

	func_mesh = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get_data< GraphData >( );
		glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec3 norm = { 0.0f, 0.0f, 1.0f };

		int num_history = data->record->history.size( );

		if( num_history <= 0 ) { 
			return 0;
		}

		if( num_history > data->num_entries ) { 
			num_history = data->num_entries;
		}

		comp->page->vbo_mesh.push_set( VBO::IndexSet(
			VBO::TypeGeometry::TG_Lines,
			"BasicOrtho",
			data->id_texture_line,
			std::vector< GLuint > { 0, 1 }
		) );

		float delta_history = float( comp->dim.x ) / ( num_history - 1 );
		float max_history = TIME_FRAME_MILLI;

		for( int i = 0; i < num_history; ++i ) {
			if( data->record->history[ i ] > max_history ) { 
				max_history = data->record->history[ i ];
			}
		}

		for( int i = 1; i < num_history; ++i ) {
			comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
				{ { comp->pos.x + int( ( i - 1 ) * delta_history ),
					comp->pos.y + comp->dim.y * ( data->record->history[ i - 1 ] / max_history ),
					0.0f }, 
				color, norm, 
				{ 0.0f, 0.0f, data->id_subtex_line } },

				{ { comp->pos.x + int( ( i ) * delta_history ),
					comp->pos.y + comp->dim.y * ( data->record->history[ i ] / max_history ),
					0.0f },	
				color, norm, 
				{ 0.0f, 0.0f, data->id_subtex_line } }
			} );
		}

		comp->page->vbo_mesh.finalize_set( );

		return 0;
	};
}


GraphComp::~GraphComp( ) { }
