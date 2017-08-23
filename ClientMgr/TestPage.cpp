#include "TestPage.h"

#include "ResizableComp.h"
#include "LabelComp.h"
#include "TextButtonComp.h"
#include "BorderImageComp.h"
#include "SliderComp.h"
#include "SliderVComp.h"
#include "ClickableComp.h"
#include "TextFieldComp.h"


TestPage::TestPage( Client & client ) {
	name = "Test";

	func_alloc = [ &client = client ] ( Page * page ) { 
		page->is_visible = true;
		page->is_remesh = true;

		page->root->anchor = { 0.5f, 0.5f };
		page->root->offset = { 0, 0 };
		page->root->dim = { 500, 500 };

		auto comp_resize = page->add_comp( "RootResizable", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto comp_border = comp_resize->add_comp( "Border", "BorderImage", [ ] ( PComp * comp ) {

			return 0;
		} );

		auto clickable = comp_border->add_comp( "ClickableBorder", "Clickable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ClickableComp::ClickableData >( );
			data->func_hold = [ &client = client ] ( PComp * comp ) {
				comp->page->root->offset += client.input_mgr.get_mouse_delta( );

				return 0;
			};

			return 0;
		} );

		comp_border->add_comp( "TestButton", "TextButton", [ ] ( PComp * comp ) {
			comp->anchor = { 0.5f, 0.5f };

			auto button_data = comp->get< TextButtonComp::TextButtonData >( );
			auto label_data = button_data->comp_label->get< LabelComp::LabelData >( );

			label_data->text = "TestButton";

			comp->page->is_remesh = true;

			return 0;
		} );

		comp_border->add_comp( "TitleLabel", "Label", [ ] ( PComp * comp ) {
			auto data = comp->get< LabelComp::LabelData >( );

			data->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;

			data->text = comp->page->name + " Page";
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 10, -10 };
			comp->page->is_remesh = true;

			return 0;
		} );

		comp_border->add_comp( "TestImage", "Image", [ ] ( PComp * comp ) {
			comp->anchor = { 1.0f, 1.0f };
			comp->offset = -comp->dim - glm::ivec2( 10, 10 );

			return 0;
		} );

		comp_border->add_comp( "TestCheck", "Checkbox", [ ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 0.0f };
			comp->offset = { 25, 25 };

			return 0;
		} );

		
		comp_border->add_comp( "TestSlider", "Slider", [ &client = client ] ( PComp * comp ) {
			comp->offset += glm::ivec2{ 0, 100 };

			auto data = comp->get< SliderComp::SliderData >( );
			data->set_bounds( 0.0f, 359.0f );
			data->set_value( 180 );

			data->func_read = [ &client = client, data ] ( PComp * comp ) { 
				data->set_value( client.chunk_mgr.get_sun_deg( ) );

				return 0;
			}; 

			data->func_write = [ &client = client, data ] ( PComp * comp ) {
				client.chunk_mgr.set_sun_deg( data->value );

				return 0;
			};

			return 1; 
		} );

		comp_border->add_comp( "TestSliderV", "SliderV", [ &client = client ] ( PComp * comp ) {
			comp->offset += glm::ivec2 { 0, -200 };

			auto data = comp->get< SliderVComp::SliderVData >( );
			data->set_bounds( 0.0f, 359.0f );
			data->set_value( 180 );

			data->func_read = [ &client = client, data ] ( PComp * comp ) {
				data->set_value( client.chunk_mgr.get_sun_deg( ) );

				return 0;
			};

			data->func_write = [ &client = client, data ] ( PComp * comp ) {
				client.chunk_mgr.set_sun_deg( data->value );

				return 0;
			};

			return 1;
		} );

		page->add_comp( "TestResize", "Resize", [ ] ( PComp * comp ) { return 0; } );

		comp_border->add_comp( "TestTextField", "TextField", [ &client = client ] ( PComp * comp ) {
			comp->dim = { 400, 33 };
			comp->offset = { -200, 0 };
			auto data = comp->get< TextFieldComp::TextFieldData >( );
			data->data_label->text = "Hello World";

			return 0;
		} );

		auto button = page->add_comp( "CloseButton", "TextButton", [ &client = client ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 0.0f };
			comp->offset = { 16 + 300, 16 };

			auto data = comp->get< TextButtonComp::TextButtonData >( );

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


TestPage::~TestPage( ) { }
