#include "ConsolePage.h"

#include "ResizableComp.h"
#include "ClickableComp.h"
#include "TextFieldComp.h"
#include "LabelComp.h"
#include "BorderImageComp.h"
#include "ResizeComp.h"

ConsolePage::ConsolePage( Client & client ) { 
	name = "Console";

	func_alloc = [ &client = client ] ( Page * page ) {
		page->set_dim( { 400, 250 } );
		page->set_anchor( { 0, 0 } );

		auto data_console = page->add_data< ConsoleData >( );
		data_console->console = page;
		data_console->padding = 4;
		data_console->dy_input = 24;
		data_console->size_text = 16;
		data_console->num_visible = 0;
		data_console->id_labels = 0;

		auto resizable_root = page->add_comp( "ResizableRoot", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto border_root = resizable_root->add_comp( "BorderBG", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;

			return 0;
		} );

		auto clickable_root = border_root->add_comp( "ClickableRoot", "Clickable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ClickableComp::ClickableData >( );
			data->func_hold = [ &client = client ] ( PComp * comp ) {
				comp->page->root->offset += client.input_mgr.get_mouse_delta( );

				return 0;
			};

			return 0;
		} );

		auto resizable_text = page->add_comp( "ResizableText", "Resizable", [ &client = client, data_console ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client, data_console ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->dim.x -= data_console->padding * 2;
				comp->dim.y -= data_console->dy_input + data_console->padding * 3;

				comp->offset = -comp->dim / 2;
				comp->offset.y += ( ( data_console->dy_input + data_console->padding ) / 2 );

				return 0;
			};

			return 0;
		} );
		
		data_console->comp_border_text = resizable_text->add_comp( "BorderText", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;

			return 0;
		} );

		auto resizable_input = page->add_comp( "ResizableInput", "Resizable", [ &client = client, data_console ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );

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
			data_console->data_input = comp->get_data< TextFieldComp::TextFieldData >( );

			return 0;
		} );
		//textfield_input->dim 

		auto resizer = page->add_comp( "ResizerRoot", "Resize", PageComponentLoader::func_null );

		return 0;
	};
}


ConsolePage::~ConsolePage( ) { }
