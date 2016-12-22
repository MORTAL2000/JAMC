#include "ResizeComp.h"

#include "ImageComp.h"

ResizeComp::ResizeComp( Client & client ) {
	name = "Resize";

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->dim = { 16, 16 };
		comp->anchor = { 1.0f, 1.0f };
		comp->offset = -comp->dim;

		comp->is_visible = false;

		auto comp_image = comp->add_comp( "Image", "Image", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ImageComp::ImageData >( );
			data->id_texture = client.texture_mgr.get_texture_id( "Gui" );
			data->id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/Resize" );
			data->func_resize = [ ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		return 0;
	};

	func_down = [ ] ( PComp * comp ) { 
		comp->is_hold = true;

		return 1;
	};

	func_hold = [ &client = client ] ( PComp * comp ) {
		if( comp->is_hold ) { 
			if( comp->parent ) { 
				comp->parent->dim += client.input_mgr.get_mouse_delta( );
				if( comp->parent->dim.x < 0 ) { 
					comp->parent->dim.x = 0;
				}
				if( comp->parent->dim.y < 0 ) {
					comp->parent->dim.y = 0;
				}
			}
			else {
				comp->page->dim += client.input_mgr.get_mouse_delta( );
				if( comp->page->dim.x < 0 ) {
					comp->page->dim.x = 0;
				}
				if( comp->page->dim.y < 0 ) {
					comp->page->dim.y = 0;
				}
			}
			comp->page->is_remesh = true;
			return 1;
		}

		return 0;
	};

	func_up = [ ] ( PComp * comp ) {
		comp->is_hold = false;

		return 1;
	};

	func_enter = [ ] ( PComp * comp ) { 
		comp->is_visible = true;
		comp->page->is_remesh = true;

		return 1;
	};

	func_exit = [ ] ( PComp * comp ) {
		comp->is_visible = false;
		comp->page->is_remesh = true;

		return 1;
	};
}


ResizeComp::~ResizeComp( ) { 

}
