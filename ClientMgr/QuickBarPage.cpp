#include "QuickBarPage.h"

#include "ResizableComp.h"
#include "ClickableComp.h"
#include "ImageComp.h"
#include "BorderImageComp.h"


QuickBarPage::QuickBarPage( Client & client ) { 
	name = "QuickBar";

	func_alloc = [ &client = client ] ( Page * page ) {
		page->set_dim( { 256, 64 } );

		page->set_anchor( { 0.5f, 1.0f } );
		page->set_offset( { -page->get_dim( ).x / 2, -page->get_dim( ).y } );

		page->is_visible = true;

		auto clickable_page = page->add_comp( "ClickablePage", "Clickable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ClickableComp::ClickableData >( );

			data->func_hold = [ &client = client ] ( PComp * comp ) {
				comp->offset += client.input_mgr.get_mouse_delta( );

				return 0;
			};

			return 0;
		} );

		auto resizable_border = page->add_comp( "ResizableBorder", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto border_page = resizable_border->add_comp( "BorderPage", "BorderImage", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get< BorderImageComp::BorderImageData >( );
			data->set_texture( client, "Gui", "Default/PageBG", 8 );
			data->padding_border = 4;

			return 0;
		} );
		
		auto resizable_borders = page->add_comp( "ResizableBorders", "Resizable", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = { comp->parent->dim.y - 8, comp->parent->dim.y - 8 };
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		resizable_borders->add_comp( "BorderOptions", "BorderImage", [ &client = client ] ( PComp * comp ) {
			comp->anchor.x = 0.25f;

			auto data = comp->get< BorderImageComp::BorderImageData >( );
			data->set_texture( client, "Gui", "Default/PageBG", 8 );
			data->padding_border = 4;

			comp->add_comp( "ClickableOptions", "Clickable", [ &client = client ] ( PComp * comp ) {
				auto data = comp->get< ClickableComp::ClickableData >( );
				data->func_up = [ &client = client ] ( PComp * comp ) {
					if( Directional::is_point_in_rect(
						client.input_mgr.get_mouse( ),
						comp->page->get_pos( ) + comp->pos,
						comp->page->get_pos( ) + comp->pos + comp->dim ) ) { 

						auto page = client.gui_mgr.get_page_safe( "Options" );
						if( !page ) {
							return 0;
						}

						page->is_visible = !page->is_visible;
						if( page->is_visible ) { 
							page->is_remesh = true;
						}
					}
					
					return 0;
				};

				return 0;
			} );

			return 0;
		} );

		resizable_borders->add_comp( "BorderGraph", "BorderImage", [ &client = client ] ( PComp * comp ) {
			comp->anchor.x = 0.75f;

			auto data = comp->get< BorderImageComp::BorderImageData >( );
			data->set_texture( client, "Gui", "Default/PageBG", 8 );
			data->padding_border = 4;

			comp->add_comp( "ClickableGraph", "Clickable", [ &client = client ] ( PComp * comp ) {
				auto data = comp->get< ClickableComp::ClickableData >( );

				data->func_up = [ &client = client ] ( PComp * comp ) {
					static int id_graph = 0;

					if( Directional::is_point_in_rect(
						client.input_mgr.get_mouse( ),
						comp->page->get_pos( ) + comp->pos,
						comp->page->get_pos( ) + comp->pos + comp->dim ) ) {

						auto page = client.gui_mgr.add_page( "Graph" + std::to_string( id_graph++ ), "Graph", func_null );
						page->is_visible = true;
						/*
						auto page = client.gui_mgr.get_page_safe( "Graph" );
						if( !page ) {
							return 0;
						}

						page->is_visible = !page->is_visible;
						if( page->is_visible ) {
							page->is_remesh = true;
						}*/
					}

					return 0;
				};

				return 0;
			} );

			return 0;
		} );

		auto resizable_icons = page->add_comp( "ResizableIcons", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = { ( comp->parent->dim.y - 8 ) * 0.75f, ( comp->parent->dim.y - 8 ) * 0.75f };
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		resizable_icons->add_comp( "IconOptions", "Image", [ &client = client ] ( PComp * comp ) {
			comp->anchor.x = 0.25f;

			auto data = comp->get< ImageComp::ImageData >( );
			data->id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/Cog" );

			return 0;
		} );

		resizable_icons->add_comp( "IconGraph", "Image", [ &client = client ] ( PComp * comp ) {
			comp->anchor.x = 0.75f;

			auto data = comp->get< ImageComp::ImageData >( );
			data->id_subtex = client.texture_mgr.get_texture_layer( "Gui", "Default/Graph" );

			return 0;
		} );

		page->is_remesh = true;

		return 0;
	};
}


QuickBarPage::~QuickBarPage( ) { 

}
