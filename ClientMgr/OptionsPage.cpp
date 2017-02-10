#include "OptionsPage.h"

#include "ResizableComp.h"
#include "ImageComp.h"
#include "BorderImageComp.h"
#include "ClickableComp.h"
#include "TextButtonComp.h"
#include "ResizeComp.h"
#include "SliderComp.h"
#include "CheckboxComp.h"

OptionsPage::OptionsPage( Client & client ) { 
	name = "Options";

	func_alloc = [ &client = client ] ( Page * page ) {
		page->is_visible = false;
		page->root->anchor = { 0.5f, 0.5f };
		page->root->offset = { 0, 0 };
		page->root->dim = { 200, 500 };

		auto resizable = page->add_comp( "Resizable", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ parent = comp->parent ] ( PComp * comp ) {
				comp->dim = parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );
		
		auto clickable = page->add_comp( "Clickable", "Clickable", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get_data< ClickableComp::ClickableData >( );
			data->func_hold = [ &client = client ] ( PComp * comp ) { 
				comp->offset += client.input_mgr.get_mouse_delta( );

				return 0;
			};

			return 0;
		} );

		auto border = resizable->add_comp( "Border", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );
			data->set_texture( client, "Gui", "Default/PageBG", 8 );
			data->padding_border = 4;

			return 0;
		} );

		auto icon = page->add_comp( "ImageIcon", "Image", [ &client = client ] ( PComp * comp ) {
			comp->dim = { 24, 24 };
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 6, -6 - comp->dim.y };
			
			auto data = comp->get_data< ImageComp::ImageData >( );
			data->id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/Cog" );

			return 0;
		} );

		auto label_icon = icon->add_comp( "LabelIcon", "Label", [ &client = client, icon ] ( PComp * comp ) {
			comp->anchor = { 1.0f, 0.5f };
			comp->offset = { 6, 0 };

			auto data = comp->get_data< LabelComp::LabelData >( );
			data->text = "Options";
			data->size_text = 20;

			data->alignment_v = LabelComp::LabelData::AV_Center;
			data->alignment_h = LabelComp::LabelData::AH_Right;

			return 0;
		} );

		auto resizer = page->add_comp( "Resize", "Resize", [ &client = client ] ( PComp * comp ) { 

			return 0;
		} );

		auto slider_sun = page->add_comp( "SliderSun", "Slider", [ &client = client ] ( PComp * comp ) { 
			comp->anchor = { 0.0, 1.0f };
			comp->offset = { 12, -36 - 6 - ( 54 ) };

			auto data = comp->get_data < SliderComp::SliderData >( );
			data->data_label_title->text = "Sun Position";
			data->set_bounds( 0, 359 );
			data->set_value( client.chunk_mgr.get_sun_deg( ) );

			data->func_read = [ &client = client, data ] ( PComp * comp ) {
				data->set_value( client.chunk_mgr.get_sun_deg( ) );

				return 0;
			};

			data->func_write = [ &client = client, data ] ( PComp * comp ) {
				client.chunk_mgr.set_sun_deg( data->value );

				return 0;
			};

			return 0;
		} );

		auto slider_fov = page->add_comp( "SliderFov", "Slider", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0, 1.0f };
			comp->offset = { 12, -36 - 6 - 2 * ( 54 + 5 ) };

			auto data = comp->get_data < SliderComp::SliderData >( );
			data->data_label_title->text = "Camera Fov";
			data->set_bounds( 40, 160 );
			data->set_value( client.display_mgr.fov );

			data->func_read = [ &client = client, data ] ( PComp * comp ) {
				data->set_value( client.display_mgr.fov );

				return 0;
			};

			data->func_write = [ &client = client, data ] ( PComp * comp ) {
				client.display_mgr.fov = data->value;
				client.display_mgr.set_proj( );

				return 0;
			};

			return 0;
		} );

		auto checkbox_sun = page->add_comp( "CheckboxSun", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Sun Paused";

			data->func_checked = [ &client = client ] ( PComp * comp ) { 
				client.chunk_mgr.set_sun_pause( true );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.set_sun_pause( false );

				return 0;
			};

			data->set_checked( true );

			return 0;
		} );

		auto checkbox_flatshade = page->add_comp( "CheckboxFlatshade", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 2 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Flatshade";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_flatshade( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_flatshade( );

				return 0;
			};

			data->set_checked( false );

			return 0;
		} );

		auto checkbox_wireframe = page->add_comp( "CheckboxWireframe", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 3 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Wireframe";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_wireframe( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_wireframe( );

				return 0;
			};

			data->set_checked( false );

			return 0;
		} );

		auto checkbox_limiter = page->add_comp( "CheckboxLimiter", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 4 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "FPS Limiter";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.display_mgr.toggle_limiter( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.display_mgr.toggle_limiter( );

				return 0;
			};

			data->set_checked( true );

			return 0;
		} );

		auto checkbox_vsync = page->add_comp( "CheckboxVsync", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 5 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Vsync";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.display_mgr.toggle_vsync( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.display_mgr.toggle_vsync( );

				return 0;
			};

			data->set_checked( false );

			return 0;
		} );

		auto checkbox_render_solid = page->add_comp( "CheckboxRenderSolid", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 6 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Render Solid";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_render_solid( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_render_solid( );

				return 0;
			};

			data->set_checked( true );

			return 0;
		} );

		auto checkbox_render_trans = page->add_comp( "CheckboxRenderTrans", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 7 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Render Transparent";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_render_trans( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_render_trans( );

				return 0;
			};

			data->set_checked( true );

			return 0;
		} );

		auto checkbox_shadow_solid = page->add_comp( "CheckboxShadowSolid", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 8 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Shadow Solids";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_shadow_solid( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_shadow_solid( );

				return 0;
			};

			data->set_checked( true );

			return 0;
		} );

		auto checkbox_shadow_trans = page->add_comp( "CheckboxShadowTrans", "Checkbox", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 8, -36 - 6 - 2 * ( 54 + 5 ) - ( 10 ) - 9 * ( 25 + 5 ) };

			auto data = comp->get_data< CheckboxComp::CheckboxData >( );

			data->data_label->text = "Shadow Transparents";

			data->func_checked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_shadow_trans( );

				return 0;
			};

			data->func_unchecked = [ &client = client ] ( PComp * comp ) {
				client.chunk_mgr.toggle_shadow_trans( );

				return 0;
			};

			data->set_checked( true );

			return 0;
		} );

		auto button = page->add_comp( "CloseButton", "TextButton", [ &client = client ] ( PComp * comp ) { 
			comp->anchor = { 0.0f, 0.0f };
			comp->offset = { 16, 16 };

			auto data = comp->get_data< TextButtonComp::TextButtonData >( );

			data->data_label->text = "Close";

			data->func_action = [ ] ( PComp * comp ) { 
				comp->page->is_visible = false;

				return 0;
			};

			return 0;
		} );
		
		return 0;
	};
}


OptionsPage::~OptionsPage( ) { 

}
