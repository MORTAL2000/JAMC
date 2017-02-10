#include "GraphComp.h"
#include "Page.h"


GraphComp::GraphComp( Client & client ) { 
	name = "Graph";

	func_register = [ &client = client ] () {
		if( !client.resource_mgr.reg_pool< GraphData >( num_comp_default ) ) {
			return 1;
		}

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->anchor = { 0.5f, 0.5f };

		auto data = comp->add_data< GraphData >( );
		data->num_entries = 300;
		data->updates = 0;
		data->record = &client.time_mgr.get_record( RecordStrings::RENDER );

		data->id_texture_line = client.texture_mgr.get_texture_id( "Materials" );
		data->id_subtex_line = client.texture_mgr.get_texture_layer( "Materials", "Details/Solid" );

		data->padding = { 4, 4 };

		data->comp_label_title = comp->add_comp( "LabelTitle", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 10.0f + 24, -10.0f };

			data->data_label_title = comp->get_data< LabelComp::LabelData >( );
			data->data_label_title->size_text = 16;
			data->data_label_title->text = RecordStrings::RENDER;
			data->data_label_title->alignment_h = LabelComp::LabelData::AH_Right;
			data->data_label_title->alignment_v = LabelComp::LabelData::AV_Bottom;

			return 0;
		} );

		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get_data< GraphData >( );
		++data->updates;

		//if( data->updates % 4 == 0 )
		comp->page->is_remesh = true;

		return 0;
	};

	func_mesh = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get_data< GraphData >( );
		glm::vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f };
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

		float delta_history = float( comp->dim.x - 2 * data->padding.x ) / ( num_history - 1 );
		float max_history = TIME_FRAME_MILLI;

		for( int i = 0; i < num_history; ++i ) {
			if( data->record->history[ i ] > max_history ) { 
				max_history = data->record->history[ i ];
			}
		}
		
		
		// Draw the grid
		float delta_grid_y;
		int delta_grid_x;

		color = { 0.4f, 0.4f, 0.4f, 1.0f };

		// Draw the y grid - 0.25f
		delta_grid_y = TIME_FRAME_MILLI / 4.0f;
		for( float i = delta_grid_y; i < max_history; i += delta_grid_y ) {
			comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
				{ { data->padding.x + comp->pos.x,
					data->padding.y + comp->pos.y + ( comp->dim.y - data->padding.y * 2 ) * ( i / max_history ),
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

				{ { data->padding.x + comp->pos.x + comp->dim.x - 2 * data->padding.x,
					data->padding.y + comp->pos.y + ( comp->dim.y - data->padding.y * 2 ) * ( i / max_history ),
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } }
			} );
		}

		// Draw the x grid - 1/4 frame
		delta_grid_x = UPDATE_RATE / 4;
		for( float i = 0.0f; i < num_history - delta_grid_x; i += delta_grid_x ) {
			comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
				{ { data->padding.x + comp->pos.x + 
					( comp->dim.x - data->padding.x * 2 ) * ( i / num_history ) + 
					( comp->dim.x - data->padding.x * 2 ) * ( float( data->updates % delta_grid_x ) / num_history ),
					data->padding.y,
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

				{ { data->padding.x + comp->pos.x +
					( comp->dim.x - data->padding.x * 2 ) * ( i / num_history ) +
					( comp->dim.x - data->padding.x * 2 ) * ( float( data->updates % delta_grid_x ) / num_history ),
					data->padding.y + comp->pos.y + comp->dim.y - 2 * data->padding.y,
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } }
			} );
		}

		color = { 0.0f, 0.0f, 0.0f, 1.0f };

		// Draw the y grid - 1.0f
		delta_grid_y = TIME_FRAME_MILLI;
		for( float i = delta_grid_y; i < max_history; i += delta_grid_y ) {
			comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
				{ { data->padding.x + comp->pos.x,
					data->padding.y + comp->pos.y + ( comp->dim.y - data->padding.y * 2 ) * ( i / max_history ),
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

				{ { data->padding.x + comp->pos.x + comp->dim.x - 2 * data->padding.x,
					data->padding.y + comp->pos.y + ( comp->dim.y - data->padding.y * 2 ) * ( i / max_history ),
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } }
			} );
		}

		// Draw the x grid - 1/4 frame
		delta_grid_x = UPDATE_RATE;
		for( float i = ( data->updates % delta_grid_x ); i < num_history; i += delta_grid_x ) {
			comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
				{ { data->padding.x + comp->pos.x +
					( comp->dim.x - data->padding.x * 2 ) * ( i / num_history ), //+
					//( comp->dim.x - data->padding.x * 2 ) * ( float( data->updates % delta_grid_x ) / num_history ),
					data->padding.y,
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

				{ { data->padding.x + comp->pos.x +
					( comp->dim.x - data->padding.x * 2 ) * ( i / num_history ), //+
					//( comp->dim.x - data->padding.x * 2 ) * ( float( data->updates % delta_grid_x ) / num_history ),
					data->padding.y + comp->pos.y + comp->dim.y - 2 * data->padding.y,
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } }
			} );
		}

		// Draw the graph lines
		color = { 0.75f, 0.0f, 0.0f, 1.0f };

		for( int i = 1; i < num_history; ++i ) {
			comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
				{ { data->padding.x + comp->pos.x + int( ( i - 1 ) * delta_history ),
					data->padding.y + comp->pos.y + ( comp->dim.y - 2 * data->padding.y ) * ( data->record->history[ i - 1 ] / max_history ),
					0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

				{ { data->padding.x + comp->pos.x + int( ( i ) * delta_history ),
					data->padding.y + comp->pos.y + ( comp->dim.y - 2 * data->padding.y ) * ( data->record->history[ i ] / max_history ),
					0.0f },	color, norm, { 0.0f, 0.0f, data->id_subtex_line } }
			} );
		}

		// Draw the graph box
		color = { 0.0f, 0.0f, 0.0f, 1.0f };

		comp->page->vbo_mesh.push_data( std::vector< VBO::Vertex > {
			{ { data->padding.x + comp->pos.x,
				data->padding.y + comp->pos.y,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

			{ { data->padding.x + comp->pos.x + comp->dim.x - 2 * data->padding.x,
				data->padding.y + comp->pos.y,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

			{ { data->padding.x + comp->pos.x,
				data->padding.y + comp->pos.y + comp->dim.y - 2 * data->padding.y,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

			{ { data->padding.x + comp->pos.x + comp->dim.x - 2 * data->padding.x,
				data->padding.y + comp->pos.y + comp->dim.y - 2 * data->padding.y,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

			{ { data->padding.x + comp->pos.x,
				data->padding.y + comp->pos.y ,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

			{ { data->padding.x + comp->pos.x,
				data->padding.y + comp->pos.y + comp->dim.y - 2 * data->padding.y,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

			{ { data->padding.x + comp->pos.x + comp->dim.x - 2 * data->padding.x,
				data->padding.y + comp->pos.y,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } },

			{ { data->padding.x + comp->pos.x + comp->dim.x - 2 * data->padding.x,
				data->padding.y + comp->pos.y + comp->dim.y - 2 * data->padding.y,
				0.0f }, color, norm, { 0.0f, 0.0f, data->id_subtex_line } }
		} );

		comp->page->vbo_mesh.finalize_set( );

		return 0;
	};
}


GraphComp::~GraphComp( ) { }
