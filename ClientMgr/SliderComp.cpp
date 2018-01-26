#include "SliderComp.h"

#include "InputMgr.h"
#include "ResourceMgr.h"

#include "Page.h"

#include "ResizableComp.h"
#include "OverableComp.h"
#include "ClickableComp.h"

SliderComp::SliderComp( Client & client ) { 
	name = "Slider";

	func_register = [ &client = client ] ( ) { 
		client.resource_mgr.reg_pool< SliderData >( num_comp_default );

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) { 
		comp->anchor = { 0.5f, 0.5f };
		comp->dim = { 128, 32 };

		auto data = comp->add_data< SliderData >( );
		data->cnt_remesh = 0;

		data->ratio = 0.0f;
		data->value = 0.0f;
		data->lower = 0.0f;
		data->upper = 1.0f;

		data->width_bar = 8;

		data->func_read = func_null;
		data->func_write = func_null;

		auto resizable = comp->add_comp( "Resizable", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get < ResizableComp::ResizableData >( );
			data->func_resize = [ ] ( PComp * comp ) { 
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		data->comp_border = resizable->add_comp( "Border", "BorderImage", [ &client = client, data ] ( PComp * comp ) { 
			data->data_border = comp->get< BorderImageComp::BorderImageData >( );
			data->data_border->padding_border = 4;
			data->data_border->set_texture( client, "Gui", "Default/SliderBG", 8 );

			auto overable = comp->add_comp( "Overable", "Overable", [ &client = client, data ] ( PComp * comp ) {
				auto data_over = comp->get< OverableComp::OverableData >( );

				data_over->func_enter = [ &client = client, data ] ( PComp * comp ) { 
					data->data_border->color = { 0.5f, 0.5f, 0.5f, 1.0f };
					data->comp_label_left->is_visible = true;
					data->comp_label_right->is_visible = true;
					comp->page->is_remesh = true;

					return 0;
				};

				data_over->func_exit = [ &client = client, data ] ( PComp * comp ) {
					data->data_border->color = { 1.0f, 1.0f, 1.0f, 1.0f };
					data->comp_label_left->is_visible = false;
					data->comp_label_right->is_visible = false;
					comp->page->is_remesh = true;

					return 0;
				};

				return 0;
			} );

			auto clickable = comp->add_comp( "Clickable", "Clickable", [ &client = client, data ] ( PComp * comp ) {
				auto data_click = comp->get< ClickableComp::ClickableData >( );

				data_click->func_hold = [ &client = client, data ] ( PComp * comp ) {
					int posl_mouse = client.input_mgr.get_mouse( ).x - ( comp->page->pos.x + data->comp_bar->pos.x );

					if( posl_mouse < 0 ) {
						posl_mouse = 0;
					}

					else if( posl_mouse > data->length_bar ) {
						posl_mouse = data->length_bar;
					}

					data->set_ratio( posl_mouse / data->length_bar );
					data->func_write( comp );
					comp->page->is_remesh = true;


					return 0;

					return 0;
				};

				return 0;
			} );

			return 0;
		} );

		data->comp_bar = comp->add_comp( "Bar", "BorderImage", [ &client = client, data ] ( PComp * comp ) {
			data->data_bar = comp->get< BorderImageComp::BorderImageData >( );
			data->data_bar->padding_border = 4;
			data->data_bar->set_texture( client, "Gui", "Default/SliderBG", 8 );

			return 0;
		} );

		data->comp_label_title = comp->add_comp( "TitleLabel", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->is_visible = true;
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { 3.0f, 3.0f };

			data->data_label_title = comp->get< LabelComp::LabelData >( );
			data->data_label_title->size_text = 12;
			data->data_label_title->alignment_v = LabelComp::LabelData::AlignVertical::AV_Top;
			data->data_label_title->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Right;
			data->data_label_title->text = "Title";

			return 0;
		} );

		data->comp_label_left = comp->add_comp( "LeftLabel", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->is_visible = false;
			comp->anchor = { 0.0f, 0.0f };
			comp->offset = { 3.0f, -3.0f };

			data->data_label_left = comp->get< LabelComp::LabelData >( );
			data->data_label_left->size_text = 10;
			data->data_label_left->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;
			data->data_label_left->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Center;
			data->data_label_left->text = "Left";

			return 0;
		} );

		data->comp_label_right = comp->add_comp( "RightLabel", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->is_visible = false;
			comp->anchor = { 1.0f, 0.0f };
			comp->offset = { -3.0f, -3.0f };

			data->data_label_right = comp->get< LabelComp::LabelData >( );
			data->data_label_right->size_text = 10;
			data->data_label_right->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;
			data->data_label_right->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Center;
			data->data_label_right->text = "Right";

			return 0;
		} );

		data->comp_label_value = comp->add_comp( "ValueLabel", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 0.5f, 0.0f };
			comp->offset = { 0.0f, -3.0f };

			data->data_label_value = comp->get< LabelComp::LabelData >( );
			data->data_label_value->size_text = 12;
			data->data_label_value->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;
			data->data_label_value->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Center;
			data->data_label_value->text = "Value";

			return 0;
		} );

		data->comp_slider = comp->add_comp( "Slider", "BorderImage", [ &client = client, data ] ( PComp * comp ) {
			data->data_slider = comp->get< BorderImageComp::BorderImageData >( );
			data->data_slider->padding_border = 4;
			data->data_slider->set_texture( client, "Gui", "Default/SliderBG", 8 );

			return 0;
		} );

		return 0;
	};	
	
	func_update = [ &client = client ] ( PComp * comp ) {
		auto data = comp->get< SliderData >( );

		data->length_bar = comp->dim.x - data->data_border->padding_border * 2;

		data->comp_bar->dim = { data->length_bar, data->width_bar };
		data->comp_bar->offset = -data->comp_bar->dim / 2;

		data->comp_slider->anchor = { 0.5f, 0.5f };
		data->comp_slider->dim = { data->width_bar, comp->dim.y };
		data->comp_slider->offset = -data->comp_slider->dim / 2;
		data->comp_slider->offset.x += ( data->ratio - 0.5f ) * data->length_bar;

		data->func_read( comp );

		if( ++data->cnt_remesh % 4 == 0 ) {
			comp->page->is_remesh = true;
		}

		return 0;
	};
}


SliderComp::~SliderComp( ) { }
