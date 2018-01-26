#pragma once

#include "StatisticsPage.h"

#include "InputMgr.h"

#include "ResizableComp.h"
#include "ClickableComp.h"
#include "LabelComp.h"
#include "BorderImageComp.h"
#include "ResizeComp.h"

StatisticsPage::StatisticsPage( Client & client ) {
	name = "Statistics";

	func_alloc = [ &client = client ] ( Page * page ) {
		page->set_dim( { 600, 250 } );
		page->set_offset( { 0, -250 } );
		page->set_anchor( { 0, 1 } );

		auto data_statistics = page->add_data< StatisticsData >( );
		data_statistics->statistics = page;

		data_statistics->padding_border = 4;
		data_statistics->padding_category = 4;
		data_statistics->padding_statistics = 0;

		data_statistics->size_text_title = 20;
		data_statistics->size_text_category = 24;
		data_statistics->size_text_statistics = 12;

		auto resizable_root = page->add_comp( "ResizableRoot", "Resizable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->offset = -comp->dim / 2;

				return 0;
			};

			return 0;
		} );

		auto border_root = resizable_root->add_comp( "BorderBG", "BorderImage", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;

			return 0;
		} );

		auto clickable_border = border_root->add_comp( "ClickableRoot", "Clickable", [ &client = client ] ( PComp * comp ) {
			auto data = comp->get< ClickableComp::ClickableData >( );
			data->func_hold = [ &client = client ] ( PComp * comp ) {
				comp->page->root->offset += client.input_mgr.get_mouse_delta( );

				return 0;
			};

			return 0;
		} );

		auto text_title = border_root->add_comp( "Title", "Label", [ &client = client, data_statistics ] ( PComp * comp ) {
			auto data = comp->get< LabelComp::LabelData >( );
			data->text = "Statistics";
			data->size_text = data_statistics->size_text_title;
			data->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;

			comp->anchor = { 0.0f, 1.0f };
			comp->offset = { data_statistics->padding_border, -data_statistics->padding_border };

			return 0;
		} );

		auto resizer = page->add_comp( "ResizerRoot", "Resize", PageComponentLoader::func_null );

		auto resizable_text = border_root->add_comp( "ResizableText", "Resizable", [ &client = client, data_statistics ] ( PComp * comp ) {
			auto data = comp->get< ResizableComp::ResizableData >( );

			data->func_resize = [ &client = client, data_statistics ] ( PComp * comp ) {
				comp->dim = comp->parent->dim;
				comp->dim.x -= data_statistics->padding_border * 2;
				comp->dim.y -= data_statistics->size_text_title + data_statistics->padding_border * 3;

				comp->offset = -comp->dim / 2;
				comp->offset.y -= data_statistics->size_text_title / 2 + data_statistics->padding_border / 2;

				return 0;
			};

			return 0;
		} );

		auto border_text = resizable_text->add_comp( "BorderText", "BorderImage", [ &client = client ] ( PComp * comp ) { 
			auto data = comp->get< BorderImageComp::BorderImageData >( );
			data->padding_border = 4;
			data->color *= 0.75f;

			return 0;
		} );

		for( int i = 0; i < StatisticsData::num_max_labels; ++i ) {
			PComp * comp = border_text->add_comp( "Label" + std::to_string( i ), "Label", PageComponentLoader::func_null );
			comp->is_visible = false;
			comp->anchor = { 0.0f, 1.0f };
			comp->offset = {
				data_statistics->padding_border,
				-data_statistics->padding_border - i * data_statistics->size_text_statistics };

			auto data = comp->get < LabelComp::LabelData >( );
			data->text = "Label" + std::to_string( i );
			data->size_text = data_statistics->size_text_statistics;
			data->alignment_v = LabelComp::LabelData::AlignVertical::AV_Bottom;
			data_statistics->list_labels.push_back( comp );
		}

		return 0;
	};

	func_update = [ &client = client ] ( Page * page ) {
		if( client.cnt_update( ) % 8 == 0 ) {
			auto data_statistics = page->get< StatisticsData >( );
			data_statistics->update_entries( );
		}

		return 0;
	};
}


StatisticsPage::~StatisticsPage( ) { }
