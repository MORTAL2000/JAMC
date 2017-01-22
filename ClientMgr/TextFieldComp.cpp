#include "TextFieldComp.h"

#include "ResizableComp.h"
#include "ClickableComp.h"
#include "ImageComp.h"
#include "BorderImageComp.h"

TextFieldComp::TextFieldComp( Client & client ) {
	name = "TextField";

	func_register = [ &client = client ] ( ) {
		client.resource_mgr.reg_pool< TextFieldData >( num_comp_default );

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->anchor = { 0.5f, 0.5f };
		comp->dim = { 100, 33 };
		comp->is_visible = true;

		auto data = comp->add_data< TextFieldData >( );
		data->text = "Hello World";
		data->padding = 4;
		data->pos_curs = 0;
		data->pos_d = 0;
		data->pos_hl_s = 0;
		data->pos_hl_e = 0;

		auto resizable = comp->add_comp( "Resizable", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< ResizableComp::ResizableData >( );
			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto border = resizable->add_comp( "Border", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get_data< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;

			return 0;
		} );

		data->comp_label = border->add_comp( "Label", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 0.5f };
			comp->offset = { data->padding, 0 };

			data->data_label = comp->get_data< LabelComp::LabelData >( );
			data->data_label->size_text = 16;
			data->data_label->alignment_h = LabelComp::LabelData::AH_Right;
			data->data_label->alignment_v = LabelComp::LabelData::AV_Center;

			return 0;
		} );

		data->comp_highlight = border->add_comp( "HighlightImage", "Image", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 0.5f };
			comp->dim = { data->data_label->size_text * 2 / 3, data->data_label->size_text + 6 };
			comp->offset = { data->padding, -comp->dim.y / 2 };

			auto data_highlight = comp->get_data< ImageComp::ImageData >( );
			data_highlight->set_texture( client, "Materials", "Details/Cursor" );
			data_highlight->color = { 0.0f, 0.0f, 0.6f, 0.3f };

			return 0;
		} );

		data->comp_cursor = border->add_comp( "CursorImage", "Image", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 0.0f, 0.5f };
			comp->dim = { data->data_label->size_text + 6 , data->data_label->size_text + 6 };
			comp->offset = { data->padding + -comp->dim.x / 2, -comp->dim.y / 2 };

			auto data_cursor = comp->get_data< ImageComp::ImageData >( );
			data_cursor->set_texture( client, "Gui", "Default/Cursor" );

			return 0;
		} );

		auto clickable = border->add_comp( "ClickableBorder", "Clickable", [ &client = client, data ] ( PComp * comp ) {
			auto data_clickable = comp->get_data< ClickableComp::ClickableData >( );

			data_clickable->func_down = [ &client = client, data ] ( PComp * comp ) {
				data->pos_curs = client.input_mgr.get_mouse( ).x - comp->page->pos.x - comp->pos.x - data->padding;
				data->pos_curs /= data->data_label->size_text * 2 / 3;

				data->pos_hl_s = data->pos_d + data->pos_curs;

				return 0;
			};

			data_clickable->func_up = [ &client = client, data ] ( PComp * comp ) {
				if( Directional::is_point_in_rect(
					client.input_mgr.get_mouse( ),
					comp->page->pos + comp->pos,
					comp->page->pos + comp->pos + comp->dim ) ) {
				
					data->pos_curs = client.input_mgr.get_mouse( ).x - comp->page->pos.x - comp->pos.x - data->padding;
					data->pos_curs /= data->data_label->size_text * 2 / 3;
				}

				data->pos_hl_e = data->pos_d + data->pos_curs;

				if( data->pos_hl_e < data->pos_hl_s ) { 
					std::swap( data->pos_hl_s, data->pos_hl_e );
				}

				if( data->pos_hl_e > data->text.size( ) ) {
					data->pos_hl_e = data->text.size( );
				}

				if( data->pos_hl_s > data->text.size( ) ) {
					data->pos_hl_s = data->text.size( );
				}

				return 0;
			};

			return 0;
		} );

		return 0;
	};

	func_update = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get_data< TextFieldData >( );
		data->num_d = ( comp->dim.x - data->padding * 2 ) / ( data->data_label->size_text * 2 / 3 );

		data->data_label->text = data->text;
		data->data_label->text.resize( data->num_d );

		if( data->pos_curs > data->text.size( ) ) {
			data->pos_curs = data->text.size( );
		}

		if( data->pos_curs > data->num_d ) {
			data->pos_curs = data->num_d;
		}

		data->comp_cursor->offset = {
			data->padding + -data->comp_cursor->dim.x / 2 + data->pos_curs * ( data->data_label->size_text * 2 / 3 ),
			-data->comp_cursor->dim.y / 2
		};

		data->comp_highlight->dim = {
			( data->pos_hl_e - data->pos_hl_s ) * ( data->data_label->size_text * 2 / 3 ),
			data->data_label->size_text + 6
		};

		data->comp_highlight->offset = {
			data->padding + data->pos_hl_s * ( data->data_label->size_text * 2 / 3 ),
			-data->comp_highlight->dim.y / 2
		};

		if( ++data->updates % 30 == 0 ) { 
			data->comp_cursor->is_visible = !data->comp_cursor->is_visible;
		}

		return 0;
	};
}

TextFieldComp::~TextFieldComp( ) { }