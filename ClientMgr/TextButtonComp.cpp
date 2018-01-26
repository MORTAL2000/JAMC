#include "TextButtonComp.h"

#include "ResourceMgr.h"
#include "InputMgr.h"

#include "Page.h"

#include "Directional.h"

TextButtonComp::TextButtonComp( Client & client ) {
	name = "TextButton";

	func_register = [ &client = client ] ( ) {
		if( !client.resource_mgr.reg_pool< TextButtonData >( num_comp_default ) ) {
			return 1;
		}

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->is_visible = true;
		comp->is_hold = false;

		comp->dim = { 100, 32 };
		comp->offset = -comp->dim / 2;

		auto data = comp->add_data< TextButtonData >( );

		data->func_action = func_null;
		data->color_default = { 1.0f, 1.0f, 1.0f, 1.0f };
		data->color_over = { 0.5f, 0.5f, 0.5f, 1.0f };
		data->color_down = { 1.0f, 0.5f, 0.5f, 1.0f };

		// Clickable
		data->comp_clickable = comp->add_comp( "Clickable", "Clickable", [ &client = client, data ] ( PComp * comp ) { 
			data->data_clickable = comp->get < ClickableComp::ClickableData >( );

			data->data_clickable->func_down = [ &client = client, data ] ( PComp * comp ) { 
				data->data_border->color = data->color_down;
				comp->page->is_remesh = true;

				return 0;
			};

			data->data_clickable->func_up = [ &client = client, data ] ( PComp * comp ) {
				if( Directional::is_point_in_rect(
					client.input_mgr.get_mouse( ),
					comp->page->pos + comp->pos,
					comp->page->pos + comp->pos + comp->dim ) ) {

					data->func_action( comp );

					data->data_border->color = data->color_over;
					comp->page->is_remesh = true;
				}
				else { 
					data->data_border->color = data->color_default;
					comp->page->is_remesh = true;
				}

				return 0;
			};

			return 0;
		} );

		// Overable
		data->comp_overable = comp->add_comp( "Overable", "Overable", [ &client = client, data ] ( PComp * comp ) {
			data->data_overable = comp->get < OverableComp::OverableData >( );

			data->data_overable->func_enter = [ &client = client, data ] ( PComp * comp ) {
				if( !( data->data_border->color == data->color_down ) ) {
					data->data_border->color = data->color_over;
					comp->page->is_remesh = true;
				}

				return 0;
			};

			data->data_overable->func_exit = [ &client = client, data ] ( PComp * comp ) {
				if( !( data->data_border->color == data->color_down ) ) {
					data->data_border->color = data->color_default;
					comp->page->is_remesh = true;
				}

				return 0;
			};

			return 0;
		} );

		// Button Border
		data->comp_border = comp->add_comp( "Border", "BorderImage", [ &client = client, data ] ( PComp * comp ) {

			data->data_border = comp->get< BorderImageComp::BorderImageData >( );

			data->data_border->set_texture( client, "Gui", "Default/ButtonBG", 8 );
			data->data_border->padding_border = 4;

			return 0;
		} );

		// Button Label
		data->comp_label = comp->add_comp( "Label", "Label", [ data ] ( PComp * comp ) {
			comp->anchor = { 0.5f, 0.5f };

			data->data_label = comp->get< LabelComp::LabelData >( );

			data->data_label->size_text = 12;
			data->data_label->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Center;
			data->data_label->alignment_v = LabelComp::LabelData::AlignVertical::AV_Center;

			return 0;
		} );

		return 0;
	};

	func_update = [ ] ( PComp * comp ) { 
		auto data = comp->get< TextButtonData >( );

		data->comp_border->dim = comp->dim;
		data->comp_border->offset = -data->comp_border->dim / 2;

		return 0;
	};
}


TextButtonComp::~TextButtonComp( ) { }
