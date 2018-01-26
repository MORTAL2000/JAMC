#include "SliderVComp.h"

#include "InputMgr.h"

#include "Page.h"

#include "ResizableComp.h"
#include "OverableComp.h"
#include "ClickableComp.h"

SliderVComp::SliderVComp( Client & client ) { 
	name = "SliderV";

	func_register = [ &client = client ] ( ) {
		client.resource_mgr.reg_pool< SliderVData >( num_comp_default );

		return 0;
	};

	func_alloc = [ &client = client ] ( PComp * comp ) {
		comp->anchor = { 0.5f, 0.5f };
		comp->dim = { 32, 128 };

		auto data = comp->add_data< SliderVData >( );
		data->cnt_remesh = 0;

		data->is_label_visible = true;

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

					if( data->is_label_visible ) {
						data->comp_label_bottom->is_visible = true;
						data->comp_label_top->is_visible = true;
					}

					comp->page->is_remesh = true;

					return 0;
				};

				data_over->func_exit = [ &client = client, data ] ( PComp * comp ) {
					data->data_border->color = { 1.0f, 1.0f, 1.0f, 1.0f };

					if( data->is_label_visible ) {
						data->comp_label_bottom->is_visible = false;
						data->comp_label_top->is_visible = false;
					}

					comp->page->is_remesh = true;

					return 0;
				};

				return 0;
			} );

			auto clickable = comp->add_comp( "Clickable", "Clickable", [ &client = client, data ] ( PComp * comp ) {
				auto data_click = comp->get< ClickableComp::ClickableData >( );

				data_click->func_hold = [ &client = client, data ] ( PComp * comp ) {
					int posl_mouse = client.input_mgr.get_mouse( ).y - ( comp->page->pos.y + data->comp_bar->pos.y );

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
			comp->anchor = { 0.5f, 1.0f };
			comp->offset = { 0.0f, 3.0f };

			data->data_label_title = comp->get< LabelComp::LabelData >( );
			data->data_label_title->size_text = 12;
			data->data_label_title->alignment_v = LabelComp::LabelData::AlignVertical::AV_Top;
			data->data_label_title->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Center;
			data->data_label_title->text = "Title";

			return 0;
		} );

		data->comp_label_bottom = comp->add_comp( "BottomLabel", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->is_visible = false;
			comp->anchor = { 1.0f, 0.0f };
			comp->offset = { 3.0f, 1.0f };

			data->data_label_left = comp->get< LabelComp::LabelData >( );
			data->data_label_left->size_text = 10;
			data->data_label_left->alignment_v = LabelComp::LabelData::AlignVertical::AV_Top;
			data->data_label_left->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Right;
			data->data_label_left->text = "Bottom";

			return 0;
		} );

		data->comp_label_top = comp->add_comp( "TopLabel", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->is_visible = false;
			comp->anchor = { 1.0f, 1.0f };
			comp->offset = { 3.0f, -1.0f };

			data->data_label_right = comp->get< LabelComp::LabelData >( );
			data->data_label_right->size_text = 10;
			data->data_label_right->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;
			data->data_label_right->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Right;
			data->data_label_right->text = "Top";

			return 0;
		} );

		data->comp_label_value = comp->add_comp( "ValueLabel", "Label", [ &client = client, data ] ( PComp * comp ) {
			comp->anchor = { 1.0f, 0.5f };
			comp->offset = { 3.0f, 0.0f };

			data->data_label_value = comp->get< LabelComp::LabelData >( );
			data->data_label_value->size_text = 12;
			data->data_label_value->alignment_v = LabelComp::LabelData::AlignVertical::AV_Center;
			data->data_label_value->alignment_h = LabelComp::LabelData::AlignHorizontal::AH_Right;
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
		auto data = comp->get< SliderVData >( );

		data->length_bar = comp->dim.y - data->data_border->padding_border * 2;

		data->comp_bar->dim = { data->width_bar, data->length_bar };
		data->comp_bar->offset = -data->comp_bar->dim / 2;

		data->comp_slider->anchor = { 0.5f, 0.5f };
		data->comp_slider->dim = { comp->dim.x, data->width_bar };
		data->comp_slider->offset = -data->comp_slider->dim / 2;
		data->comp_slider->offset.y += ( data->ratio - 0.5f ) * data->length_bar;

		data->func_read( comp );

		if( ++data->cnt_remesh % 4 == 0 ) {
			comp->page->is_remesh = true;
		}

		return 0;
	};
}


SliderVComp::~SliderVComp( ) { }
