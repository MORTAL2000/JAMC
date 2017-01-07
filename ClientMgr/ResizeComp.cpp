#include "ResizeComp.h"

#include "ResizableComp.h"

ResizeComp::ResizeComp( Client & client ) {
	name = "Resize";

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->dim = { 16, 16 };
		comp->anchor = { 1.0f, 1.0f };
		comp->offset = -comp->dim;

		auto data_resize = comp->add_data< ResizeData >( );
		data_resize->color_exit = { 0.0f, 0.0f, 0.0f, 0.0f };
		data_resize->color_enter = { 1.0f, 1.0f, 1.0f, 1.0f };

		auto resizable = comp->add_comp( "Resizable", "Resizable", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) { 
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto comp_image = resizable->add_comp( "Image", "Image", [ &client = client, data_resize ] ( PComp * comp ) {
			auto data = comp->get_data< ImageComp::ImageData >( );
			data->color = data_resize->color_exit;
			data->id_texture = client.texture_mgr.get_texture_id( "Gui" );
			data->id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/Resize" );

			return 0;
		} );

		data_resize->comp_image = comp_image;
		data_resize->data_image = comp_image->get_data< ImageComp::ImageData >( );

		return 0;
	};

	func_is_over = [ ] ( PComp * comp ) { 
		return 1;
	};	
	
	func_is_down = [ ] ( PComp * comp ) {
		return 1;
	};

	func_hold = [ &client = client ] ( PComp * comp ) {
		if( comp->parent ) { 
			comp->parent->dim += client.input_mgr.get_mouse_delta( );
			if( comp->parent->dim.x < 0 ) { 
				comp->parent->dim.x = 0;
			}
			if( comp->parent->dim.y < 0 ) {
				comp->parent->dim.y = 0;
			}
		}

		comp->page->is_remesh = true;

		return 1;
	};

	func_enter = [ ] ( PComp * comp ) { 
		auto data = comp->get_data< ResizeData >( );
		data->data_image->color = data->color_enter;

		comp->page->is_remesh = true;

		return 1;
	};

	func_exit = [ ] ( PComp * comp ) {
		auto data = comp->get_data< ResizeData >( );
		data->data_image->color = data->color_exit;

		comp->page->is_remesh = true;

		return 1;
	};
}


ResizeComp::~ResizeComp( ) { 

}
