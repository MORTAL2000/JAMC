#include "CheckboxComp.h"

#include "ResizableComp.h"
#include "OverableComp.h"
#include "ImageComp.h"
#include "ClickableComp.h"

CheckboxComp::CheckboxComp( Client & client ) { 
	name = "Checkbox";

	func_register = [ &client = client ]( ) { 
		client.resource_mgr.reg_pool< CheckboxData >( num_comp_default );

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->dim = { 25, 25 };
		comp->anchor = { 0.5f, 0.5f };

		auto data = comp->add_data< CheckboxData >( );
		data->is_checked = false;
		data->id_texture = client.texture_mgr.get_texture_id( "Gui" );
		data->id_subtex_unchecked = client.texture_mgr.get_texture_layer( "Gui", "Default/Unchecked" );
		data->id_subtex_checked = client.texture_mgr.get_texture_layer( "Gui", "Default/Checked" );
		data->func_checked = func_null;
		data->func_unchecked = func_null;

		auto resizable = comp->add_comp( "Resizable", "Resizable", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		data->comp_border = resizable->add_comp( "Border", "BorderImage", [ &client = client, data ] ( PComp * comp ) {
			data->data_border = comp->get_data< BorderImageComp::BorderImageData >( );

			data->data_border->set_texture( client, "Gui", "Default/CheckboxBG", 8 );
			data->data_border->padding_border = 4;

			comp->add_comp( "Clickable", "Clickable", [ &client = client, data ] ( PComp * comp ) { 
				auto data_clickable = comp->get_data< ClickableComp::ClickableData >( );

				data_clickable->func_up = [ &client = client, data ] ( PComp * comp ) {
					if( Directional::is_point_in_rect(
						client.input_mgr.get_mouse( ),
						comp->page->get_pos( ) + comp->pos,
						comp->page->get_pos( ) + comp->pos + comp->dim ) ) {

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
					}

					return 0;
				};

				return 0;
			} );

			return 0;
		} );

		auto overable = comp->add_comp( "Overable", "Overable", [ &client = client, data ] ( PComp * comp ) {
			auto data_overable = comp->get_data< OverableComp::OverableData >( );

			data_overable->func_enter = [ &client = client, data ] ( PComp * comp ) { 
				data->data_border->color = { 0.5f, 0.5f, 0.5f, 1.0f };
				comp->page->is_remesh = true;

				return 0;
			};

			data_overable->func_exit = [ &client = client, data ] ( PComp * comp ) {
				data->data_border->color = { 1.0f, 1.0f, 1.0f, 1.0f };
				comp->page->is_remesh = true;

				return 0;
			};

			return 0;
		} );

		auto resizable_half = comp->add_comp( "ResizableHalf", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim / 2;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		data->comp_image = resizable_half->add_comp( "Image", "Image", [ &client = client, data ] ( PComp * comp ) {
			data->data_image = comp->get_data< ImageComp::ImageData >( );

			data->data_image->id_texture = data->id_texture;
			data->data_image->id_subtex = data->id_subtex_unchecked;

			return 0;
		} );

		data->comp_label = comp->add_comp( "Label", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 1.0f, 0.5f };
			comp->offset = { 5.0f, 0.0f };

			data->data_label = comp->get_data< LabelComp::LabelData >( );
			data->data_label->text = "Checkbox";
			data->data_label->alignment_v = LabelComp::LabelData::AlignVertical::AV_Center;

			return 0;
		} );

		return 0;
	};
}


CheckboxComp::~CheckboxComp( ) { }
