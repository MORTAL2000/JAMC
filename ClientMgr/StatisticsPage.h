#pragma once

#include "Client.h"

#include "PageLoader.h"

#include "TextFieldComp.h"
#include "SliderVComp.h"

class StatisticsPage : public PageLoader {

public:
	struct StatisticsData {
		static int const num_max_labels = 32;

		int padding_border;
		int padding_category;
		int padding_statistics;

		int size_text_title;
		int size_text_category;
		int size_text_statistics;

		std::vector< std::function< std::string( ) > > list_entry;
		std::vector< PComp * > list_labels;

		Page * statistics;

		PComp * comp_border_text;

		TextFieldComp::TextFieldData * data_input;

		PComp * comp_slider;
		SliderVComp::SliderVData * data_slider;

		void add_entry( std::function< std::string( ) > func_update ) {
			if( list_entry.size( ) >= num_max_labels ) {
				return;
			}

			list_entry.emplace_back( func_update );
			list_labels[ list_entry.size( ) - 1 ]->is_visible = true;
		}

		void update_entries( ) { 
			for( int i = 0; i < list_entry.size( ); ++i ) {
				auto data = list_labels[ i ]->get< LabelComp::LabelData >( );
				data->text = list_entry[ i ]( );
			}

			statistics->is_remesh = true;
		}
	};

	StatisticsPage( Client & client );
	~StatisticsPage( );
};

