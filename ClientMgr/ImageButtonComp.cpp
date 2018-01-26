#include "ImageButtonComp.h"

#include "InputMgr.h"
#include "ResourceMgr.h"

#include "Page.h"

#include "ClickableComp.h"
#include "OverableComp.h"

#include "Directional.h"

ImageButtonComp::ImageButtonComp( Client & client ) { 
	name = "ImageButton";

	func_register = [ &client = client ] () {
		client.resource_mgr.reg_pool< ImageButtonData >( num_comp_default );

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		auto data = comp->add_data< ImageButtonData >( );

		data->color_default = { 1.0f, 1.0f, 1.0f, 1.0f };
		data->color_over = { 0.5f, 0.5f, 0.5f, 1.0f };
		data->color_down = { 1.0f, 0.5f, 0.5f, 1.0f };

		data->comp_border = comp->add_comp( "Border", "BorderImage", [ &client = client, data ] ( PComp * comp ) {
			data->data_border = comp->get< BorderImageComp::BorderImageData >( );
			data->data_border->padding_border = 4;
			
			return 0;
		} );

		auto clickable = data->comp_border->add_comp( "Clickable", "Clickable", [ &client = client, data ] ( PComp * comp ) {
			auto data_clickable = comp->get< ClickableComp::ClickableData >( );

			data_clickable->func_down = [ &client = client, data ] ( PComp * comp ) {
				data->data_border->color = data->color_down;
				comp->page->is_remesh = true;

				return 0;
			};

			data_clickable->func_up = [ &client = client, data ] ( PComp * comp ) {
				data->data_border->color = data->color_default;
				comp->page->is_remesh = true;

				if( Directional::is_point_in_rect(
					client.input_mgr.get_mouse( ),
					comp->page->get_pos( ) + comp->pos,
					comp->page->get_pos( ) + comp->pos + comp->dim ) ) {
				
					data->func_action( comp );
				}

				return 0;
			};

			return 0;
		} );

		auto overable = data->comp_border->add_comp( "Overable", "Overable", [ &client = client, data ] ( PComp * comp ) {
			auto data_overable = comp->get< OverableComp::OverableData >( );

			data_overable->func_enter = [ &client = client, data ] ( PComp * comp ) {
				if( !( data->data_border->color == data->color_down ) ) {
					data->data_border->color = data->color_over;
					comp->page->is_remesh = true;
				}

				return 0;
			};

			data_overable->func_exit = [ &client = client, data ] ( PComp * comp ) {
				if( !( data->data_border->color == data->color_down ) ) {
					data->data_border->color = data->color_default;
					comp->page->is_remesh = true;
				}

				return 0;
			};

			return 0;
		} );

		data->comp_image = comp->add_comp( "Image", "Image", [ &client = client, data ] ( PComp * comp ) {
			data->data_image = comp->get< ImageComp::ImageData >( );

			return 0;
		} );

		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get< ImageButtonComp::ImageButtonData >( );

		data->comp_border->dim = comp->dim;
		data->comp_border->offset = -comp->dim / 2;

		data->comp_image->dim = comp->dim - data->padding_image * 2;
		data->comp_image->offset = -data->comp_image->dim / 2;

		return 0;
	};
}


ImageButtonComp::~ImageButtonComp( ) { }
