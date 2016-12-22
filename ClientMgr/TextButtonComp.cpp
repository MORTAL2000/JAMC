#include "TextButtonComp.h"

#include "LabelComp.h"

TextButtonComp::TextButtonComp( Client & client ) {
	name = "Button";

	func_register = [ &client = client ] ( ) {
		if( client.resource_mgr.reg_pool< ButtonData >( 128 ) ) {
			return 0;
		}

		return 1;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->is_visible = true;
		comp->is_hold = false;

		comp->pos = { 0, 0 };
		comp->dim = { 100, 32 };
		comp->offset = -comp->dim / 2;

		// Button Border
		auto comp_border = comp->add_comp( "Border", "BorderImage", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );
			data->set_texture( client, "Gui", "Default/ButtonBG", 8 );
			data->func_resize = [ ] ( PComp * comp ) { 
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		// Button Label
		auto comp_label = comp->add_comp( "Label", "Label", [ ] ( PComp * comp ) { 
			comp->anchor = { 0.5f, 0.5f };

			auto data = comp->get_data< LabelComp::LabelData >( );
			data->size_text = 12;
			data->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Center;
			data->alignment_v = LabelComp::LabelData::AlignVertical::AV_Center;

			return 0;
		} );

		// Button Data
		auto data_button = comp->add_data< ButtonData >( client );
		data_button->func_action = func_null;
		data_button->color_down = { 1.0f, 0.5f, 0.5f, 1.0f };
		data_button->color_up = { 1.0f, 1.0f, 1.0f, 1.0f };
		data_button->comp_border = comp_border;
		data_button->comp_label = comp_label;
		data_button->data_border = comp_border->get_data< BorderImageComp::BorderImageData >( );

		return 0;
	};

	func_down = [ &client = client ] ( PComp * comp ) {
		auto data_button = comp->get_data< ButtonData >( );
		data_button->data_border->color = data_button->color_down;
		comp->page->is_remesh = true;

		return 1;
	};

	func_hold = [ &client = client ] ( PComp * comp ) {
		return 0;
	};

	func_up = [ ] ( PComp * comp ) {
		auto data_button = comp->get_data< ButtonData >( );
		data_button->data_border->color = data_button->color_up;
		comp->page->is_remesh = true;

		data_button->func_action( comp );

		return 1;
	};
}


TextButtonComp::~TextButtonComp( ) { }
