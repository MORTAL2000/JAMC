#include "GraphPage.h"
#include "ResizableComp.h"
#include "BorderImageComp.h"
#include "ClickableComp.h"
#include "GraphComp.h"
#include "ImageComp.h"


GraphPage::GraphPage( Client & client ) { 
	name = "Graph";

	func_register = [ &client = client ] ( ) {
		client.resource_mgr.reg_pool< GraphPageData >( 1024 );

		return 0;
	};

	func_alloc = [ &client = client ] ( Page * page ) {
		page->is_visible = false;
		page->set_dim( glm::vec2( 500, 250 ) );

		auto data_page = page->add_data< GraphPageData >( );

		auto clickable = page->add_comp( "Clickable", "Clickable", [ &client = client, data_page ] ( PComp * comp ) {
			auto data = comp->get_data< ClickableComp::ClickableData >( );

			data->func_hold = [ &client = client ] ( PComp * comp ) {
				comp->offset += client.input_mgr.get_mouse_delta( );

				return 0;
			};

			data->func_up = [ &client = client, data_page ] ( PComp * comp ) {
				if( client.input_mgr.is_mouse_up( 1 ) &&
					Directional::is_point_in_rect(
						client.input_mgr.get_mouse( ),
						comp->page->pos + comp->pos,
						comp->page->pos + comp->pos + comp->dim ) ) {

					data_page->comp_menu->offset = client.input_mgr.get_mouse( ) - comp->page->pos - comp->pos - comp->dim / 2;
					data_page->comp_menu->is_visible = true;
					data_page->comp_menu->reposition( );
				}

				return 0;
			};

			return 0;
		} );

		auto resizable = page->add_comp( "Resizable", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		resizable->add_comp( "Border", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;

			return 0;
		} );

		auto graph = resizable->add_comp( "Graph", "Graph", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< GraphComp::GraphData >( );

			return 0;
		} );

		auto icon = page->add_comp( "ImageIcon", "Image", [ &client = client ] ( PComp * comp ) {
			comp->dim = { 24, 24 };
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -8 - comp->dim.y };

			auto data = comp->get_data< ImageComp::ImageData >( );
			data->id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/Graph" );

			return 0;
		} );

		data_page->comp_menu = page->add_comp( "Menu", "Menu", PageComponentLoader::func_null );
		data_page->data_menu = data_page->comp_menu->get_data< MenuComp::MenuData >( );

		data_page->comp_menu->is_visible = false;
		data_page->comp_menu->anchor = { 0.5f, 0.5f };

		data_page->data_menu->add_entry( client, "Close", [ &client = client ] ( PComp * comp ) {
			//comp->page->is_visible = false;
			client.gui_mgr.remove_page( comp->page->name );

			return 0;
		} );

		for( auto & name_record : { RecordStrings::FRAME, RecordStrings::UPDATE, RecordStrings::TASK_MAIN,  RecordStrings::RENDER, RecordStrings::SLEEP } ) { 
			data_page->data_menu->add_entry( client, name_record, [ &client = client, graph, name_record ] ( PComp * comp ) {
				auto data = graph->get_data< GraphComp::GraphData >( );
				data->data_label_title->text = name_record;
				data->record = &client.time_mgr.get_record( name_record );

				return 0;
			} );
		}

		auto resizer = page->add_comp( "Resizer", "Resize", [ &client = client ] ( PComp * comp ) {

			return 0;
		} );

		return 0;
	};
}


GraphPage::~GraphPage( ) { }
