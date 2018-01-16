#pragma once

#include "Client.h"

#include "PageLoader.h"

#include "TextFieldComp.h"
#include "SliderVComp.h"

//#include <stdlib.h>
#include <algorithm>
#include <mutex>

class ConsolePage : public PageLoader {

public:
	struct ConsoleData {
		static unsigned int const num_history_max = 32;
		static unsigned int const num_text_max = 256;

		int padding;
		int padding_text;
		int size_text;
		int dy_input;

		int idx_visible;
		int num_visible;

		int idx_labels;
		int size_labels;

		int idx_history;
		int idx_history_recall;

		int size_title;
		int size_text_title;

		std::vector< std::string > list_history;
		std::vector< PComp * > list_labels;

		Page * console;

		PComp * comp_border_text;

		TextFieldComp::TextFieldData * data_input;

		PComp * comp_slider;
		SliderVComp::SliderVData * data_slider;

		void clear_text( ) { 
			std::lock_guard< std::recursive_mutex > lock( console->client->gui_mgr.mtx_console );

			int idx;
			for( int i = 0; i < num_visible; ++i ) { 
				idx = idx_labels - idx_visible - i;
				idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;
				list_labels[ idx ]->is_visible = false;
			}

			num_visible = 0;
			idx_visible = 0;
			idx_labels = 0;
			size_labels = 0;

			idx_history = 0;
			idx_history_recall = 0;

			data_slider->set_ratio( 0.0f );
			data_slider->func_write( comp_slider );
		}

		void set_text_size( unsigned int const size ) {
			std::lock_guard< std::recursive_mutex > lock( console->client->gui_mgr.mtx_console );

			size_text = size;

			for( auto label : list_labels ) { 
				label->get< LabelComp::LabelData >( )->size_text = size_text;
			}
		}

		void check_visibles( ) { 
			std::lock_guard< std::recursive_mutex > lock( console->client->gui_mgr.mtx_console );

			int new_num_visible = ( comp_border_text->dim.y - padding ) / ( size_text + padding_text );

			if( new_num_visible > size_labels - idx_visible ) { 
				new_num_visible = size_labels - idx_visible;
			}

			if( new_num_visible > num_visible ) {
				int idx;

				for( int i = num_visible; i < new_num_visible; ++i ) {
					idx = idx_labels - idx_visible - i;
					idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;

					list_labels[ idx ]->is_visible = true;
				}
			}
			else if( new_num_visible < num_visible ) {
				int idx;

				for( int i = new_num_visible; i < num_visible; ++i ) {
					idx = idx_labels - idx_visible - i;
					idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;

					list_labels[ idx ]->is_visible = false;
				}
			}

			num_visible = new_num_visible;
		}

		void reposition_labels( ) {
			std::lock_guard< std::recursive_mutex > lock( console->client->gui_mgr.mtx_console );

			int idx;

			for( int i = 0; i < num_visible; ++i ) { 
				idx = idx_labels - idx_visible - i;
				while( idx < 0 ) { idx += num_text_max; }
				idx = idx % num_text_max;

				list_labels[ idx ]->offset = {
					padding,
					padding + i * ( size_text + padding_text )
				};
			}
		}

		void push_command( ) {
			std::lock_guard< std::recursive_mutex > lock( console->client->gui_mgr.mtx_console );

			if( !data_input->text.size( ) ) { 
				return; 
			}

			idx_labels++;
			idx_labels = idx_labels % num_text_max;

			std::swap(
				list_labels[ idx_labels ]->get< LabelComp::LabelData >( )->text,
				data_input->text );
			data_input->text.clear( );

			list_labels[ idx_labels - idx_visible ]->is_visible = true;

			if( size_labels < num_text_max ) { 
				size_labels++;
			}

			if( size_labels > num_visible ) {
				int idx = idx_labels - idx_visible - num_visible;
				idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;

				list_labels[ idx ]->is_visible = false;
			}

			check_visibles( );
			reposition_labels( );
			data_slider->set_ratio( 0.0f );
			data_slider->func_write( comp_slider );
		}

		void push_text( std::string const & string ) { 
			std::lock_guard< std::recursive_mutex > lock( console->client->gui_mgr.mtx_console );

			idx_labels++;
			idx_labels = idx_labels % num_text_max;

			list_labels[ idx_labels ]->get< LabelComp::LabelData >( )->text = string;

			list_labels[ idx_labels - idx_visible ]->is_visible = true;

			if( size_labels < num_text_max ) {
				size_labels++;
			}

			if( size_labels > num_visible ) {
				int idx = idx_labels - idx_visible - num_visible;
				idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;

				list_labels[ idx ]->is_visible = false;
			}

			check_visibles( );
			reposition_labels( );
			data_slider->set_ratio( 0.0f );
			data_slider->func_write( comp_slider );
		}

		void set_idx_visible( int new_idx_visible ) { 
			std::lock_guard< std::recursive_mutex > lock( console->client->gui_mgr.mtx_console );

			if( new_idx_visible > idx_visible ) {
				int idx;
				
				for( int i = idx_visible; i < new_idx_visible; ++i ) { 
					idx = idx_labels - i;
					idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;
					list_labels[ idx ]->is_visible = false;

					idx = idx = idx_labels - i - num_visible;
					idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;
					list_labels[ idx ]->is_visible = true;
				}

				reposition_labels( );
			}
			else if( new_idx_visible < idx_visible ) {
				int idx;

				for( int i = idx_visible; i >= new_idx_visible; --i ) {
					idx = idx_labels - i;
					idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;
					list_labels[ idx ]->is_visible = true;

					idx = idx = idx_labels - i - num_visible;
					idx = ( ( idx % num_text_max ) + num_text_max ) % num_text_max;
					list_labels[ idx ]->is_visible = false;
				}
			}

			idx_visible = new_idx_visible;
		}
	};

	ConsolePage( Client & client );
	~ConsolePage( );
};

