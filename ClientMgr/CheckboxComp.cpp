#include "CheckboxComp.h"

#include "BorderImageComp.h"
#include "ImageComp.h"

CheckboxComp::CheckboxComp( Client & client ) { 
	name = "Checkbox";

	func_register = [ &client = client ]( ) { 
		client.resource_mgr.reg_pool< CheckboxData >( 128 );

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->dim = { 25, 25 };
		comp->anchor = { 0.5f, 0.5f };

		auto data_checkbox = comp->add_data< CheckboxData >( client );
		data_checkbox->is_checked = false;
		data_checkbox->id_texture = client.texture_mgr.get_texture_id( "Gui" );
		data_checkbox->id_subtex_unchecked = client.texture_mgr.get_texture_layer( "Gui", "Default/Unchecked" );
		data_checkbox->id_subtex_checked = client.texture_mgr.get_texture_layer( "Gui", "Default/Checked" );
		data_checkbox->func_checked = func_null;
		data_checkbox->func_unchecked = func_null;

		auto comp_border = comp->add_comp( "Border", "BorderImage", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );

			data->set_texture( client, "Gui", "Default/CheckboxBG", 8 );
			data->padding_border = 4;
			data->func_resize = [ ] ( PComp * comp ) { 
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto comp_image = comp->add_comp( "Image", "Image", [ &client = client, data_checkbox ] ( PComp * comp ) {
			auto data = comp->get_data< ImageComp::ImageData >( );

			data->id_texture = data_checkbox->id_texture;
			data->id_subtex = data_checkbox->id_subtex_unchecked;

			data->func_resize = [ ] ( PComp * comp ) {
				comp->dim.x = ( int ) comp->parent->dim.x * 0.75f;
				comp->dim.y = ( int ) comp->parent->dim.y * 0.75f;

				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		data_checkbox->comp_border = comp_border;
		data_checkbox->comp_image = comp_image;
		data_checkbox->data_image = comp_image->get_data< ImageComp::ImageData >( );

		return 0;
	};

	func_down = [ ] ( PComp * comp ) { 
		comp->is_hold = true;

		return 1;
	};

	func_hold = [ ] ( PComp * comp ) {
		if( comp->is_hold ) { 
			return 1;
		}

		return 0;
	};

	func_up = [ ] ( PComp * comp ) {
		auto data = comp->get_data< CheckboxData >( );

		comp->is_hold = false;

		if( !data->is_checked ) { 
			data->func_checked( comp );
			data->data_image->id_subtex = data->id_subtex_checked;
			comp->page->is_remesh = true;
		}
		else { 
			data->func_unchecked( comp );
			data->data_image->id_subtex = data->id_subtex_unchecked;
			comp->page->is_remesh = true;
		}

		data->is_checked = !data->is_checked;

		return 1;
	};
}


CheckboxComp::~CheckboxComp( ) { }
