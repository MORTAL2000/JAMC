#include "TestPage.h"

#include "ResizableComp.h"
#include "LabelComp.h"
#include "TextButtonComp.h"
#include "BorderImageComp.h"
#include "SliderComp.h"

TestPage::TestPage( Client & client ) {
	name = "Test";

	func_alloc = [ &client = client ] ( Page * page ) { 
		page->is_visible = true;
		page->is_remesh = true;

		page->root->anchor = { 0.5f, 0.5f };
		page->root->offset = { 0, 0 };
		page->root->dim = { 500, 500 };

		auto comp_resize = page->add_comp( "RootResizable", "Resizable", PageComponentLoader::func_null );
		auto data_resize = comp_resize->get_data< ResizableComp::ResizableData >( );
		data_resize->func_resize = [ &client = client, parent = comp_resize->parent ] 
		( PComp * comp ) { 
			comp->dim = parent->dim;
			comp->offset += -comp->dim / 2;

			return 0;
		};

		auto comp_border = comp_resize->add_comp( "Border", "BorderImage", [ ] ( PComp * comp ) {

			return 0;
		} );

		comp_border->add_comp( "TestButton", "Button", [ ] ( PComp * comp ) {
			comp->anchor = { 0.5f, 0.5f };

			auto button_data = comp->get_data< TextButtonComp::TextButtonData >( );
			auto label_data = button_data->comp_label->get_data< LabelComp::LabelData >( );

			label_data->text = "TestButton";

			comp->page->is_remesh = true;

			return 0;
		} );

		comp_border->add_comp( "TitleLabel", "Label", [ ] ( PComp * comp ) {
			auto data = comp->get_data< LabelComp::LabelData >( );

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
			auto data = comp->get_data< SliderComp::SliderData >( );
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

		return 0;
	};
}


TestPage::~TestPage( ) { }
