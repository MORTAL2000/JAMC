#include "GraphPage.h"
#include "ResizableComp.h"
#include "BorderImageComp.h"
#include "ClickableComp.h"
#include "GraphComp.h"

GraphPage::GraphPage( Client & client ) { 
	name = "Graph";

	func_alloc = [ &client = client ] ( Page * page ) {
		page->is_visible = false;
		page->set_dim( glm::vec2( 200, 200 ) );

		auto resizable = page->add_comp( "Resizable", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		resizable->add_comp( "Border", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;

			return 0;
		} );

		resizable->add_comp( "Graph", "Graph", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< GraphComp::GraphData >( );

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

		auto resizer = page->add_comp( "Resizer", "Resize", [ &client = client ] ( PComp * comp ) {

			return 0;
		} );

		return 0;
	};
}


GraphPage::~GraphPage( ) { }
