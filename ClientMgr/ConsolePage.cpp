#include "ConsolePage.h"

#include "ResizableComp.h"
#include "ClickableComp.h"
#include "TextFieldComp.h"
#include "LabelComp.h"
#include "BorderImageComp.h"
#include "ResizeComp.h"

//#include <stdlib.h>
#include <algorithm>

ConsolePage::ConsolePage( Client & client ) { 
	name = "Console";

	func_alloc = [ &client = client ] ( Page * page ) {
		page->set_dim( { 600, 250 } );
		page->set_anchor( { 0, 0 } );

		auto data_console = page->add_data< ConsoleData >( );
		data_console->console = page;

		data_console->padding = 4;
		data_console->padding_text = 0;
		data_console->size_text = 16;
		data_console->dy_input = 24;

		data_console->idx_visible = 0;
		data_console->num_visible = 0;

		data_console->idx_labels = 0;
		data_console->size_labels = 0;

		data_console->idx_history = 0;
		data_console->idx_history_recall = 0;
		data_console->list_history.resize( ConsoleData::num_history_max );

		data_console->size_title = 24;
		data_console->size_text_title = 18;

		auto resizable_root = page->add_comp( "ResizableRoot", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto border_root = resizable_root->add_comp( "BorderBG", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;

			return 0;
		} );

		auto clickable_root = border_root->add_comp( "ClickableRoot", "Clickable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ClickableComp::ClickableData >( );
			data->func_hold = [ &client = client ] ( PComp * comp ) {
				comp->page->root->offset += client.input_mgr.get_mouse_delta( );

				return 0;
			};

			return 0;
		} );

		auto label_title = border_root->add_comp( "LabelTitle", "Label", [ &client = client, data_console ] ( PComp * comp ) {
			auto data = comp->get< LabelComp::LabelData >( );
			data->text = "Console";
			data->size_text = data_console->size_text_title;
			data->alignment_v = LabelComp::LabelData::AlignVertical::AV_Center;

			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { data_console->padding * 2, -data_console->padding - data_console->size_title / 2  };

			return 0;
		} );

		auto resizable_text = border_root->add_comp( "ResizableText", "Resizable", [ &client = client, data_console ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client, data_console ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->dim.x -= data_console->padding * 2;
				comp->dim.y -= data_console->dy_input + data_console->padding * 3;
				comp->dim.y -= data_console->size_title;

				comp->offset = -comp->dim / 2;
				comp->offset.y += ( ( data_console->dy_input + data_console->padding ) / 2 );
				comp->offset.y -= data_console->size_title / 2;

				data_console->check_visibles( );
				data_console->reposition_labels( );

				return 0;
			};

			return 0;
		} );
		
		data_console->comp_border_text = resizable_text->add_comp( "BorderText", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;
			data->color *= 0.75f;
			return 0;
		} );

		for( unsigned int i = 0; i < ConsoleData::num_text_max; ++i ) { 
			data_console->list_labels.push_back( data_console->comp_border_text->add_comp( "Label" + std::to_string( i ), "Label", [ &client = client , data_console, i ] ( PComp * comp ) {
				comp->is_visible = false;
				auto data = comp->get< LabelComp::LabelData >( );
				data->text = comp->name;

				return 0;
			} ) );
		}

		auto resizable_scroll = data_console->comp_border_text->add_comp( "ResizableScrollBar", "Resizable", [ &client = client, data_console ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );
			data->func_resize = [ &client = client, data_console ] ( PComp * comp ) {
				comp->dim = { 24, comp->parent->dim.y };
				comp->offset = { -comp->dim.x, -comp->dim.y / 2 };

				return 0;
			};

			return 0;
		} );

		data_console->comp_slider = resizable_scroll->add_comp( "Slider", "SliderV", [ &client = client, data_console ] ( PComp * comp ) {
			comp->anchor = { 1.0f, 0.5f };

			data_console->data_slider = comp->get< SliderVComp::SliderVData >( );
			data_console->data_slider->set_bounds( 0.0f, 1.0f );
			data_console->data_slider->set_value( 0.0f );
			data_console->data_slider->set_labels_visible( false );

			data_console->data_slider->func_write = [ &client = client, data_console ] ( PComp * comp ) {
				std::lock_guard< std::recursive_mutex > lock( client.gui_mgr.mtx_console );

				data_console->set_idx_visible( std::round(
					data_console->data_slider->ratio *
					std::max( 0, data_console->size_labels - data_console->num_visible ) 
				) );

				return 0;
			};

			return 0;
		} );

		auto resizable_input = page->add_comp( "ResizableInput", "Resizable", [ &client = client, data_console ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client, data_console ] ( PComp * comp ) {
				comp->dim.y = data_console->dy_input;
				comp->dim.x = comp->parent->dim.x - data_console->padding * 2;
				 
				comp->offset.x = -comp->dim.x / 2;
				comp->offset.y = -comp->parent->dim.y / 2 + data_console->padding;

				return 0;
			};

			return 0;
		} );

		auto textfield_input = resizable_input->add_comp( "TextFieldInput", "TextField", [ &client = client, data_console ] ( PComp * comp ) {
			data_console->data_input = comp->get< TextFieldComp::TextFieldData >( );
			data_console->data_input->text = "";
			data_console->data_input->data_border->color *= 0.75f;

			return 0;
		} );

		auto resizer = page->add_comp( "ResizerRoot", "Resize", PageComponentLoader::func_null );

		data_console->set_idx_visible( 0 );

		return 0;
	};
}


ConsolePage::~ConsolePage( ) { }
