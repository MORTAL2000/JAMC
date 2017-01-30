#pragma once

#include "Client.h"
#include "PageLoader.h"
#include "TextFieldComp.h"

class ConsolePage : public PageLoader {
public:
	struct ConsoleData {
		int padding;
		int dy_input;

		int size_text;

		int num_visible;

		int id_labels;

		std::vector< std::string  > list_strings;
		std::vector< PComp * > list_labels;

		Page * console;

		PComp * comp_border_text;

		TextFieldComp::TextFieldData * data_input;

		void set_text_size( unsigned int const size ) {
			size_text = size;

			for( auto label : list_labels ) { 
				label->get_data< LabelComp::LabelData >( )->size_text = size_text;
			}
		}

		void check_visibles( ) { 
			int new_num_visible = ( comp_border_text->dim.y - padding ) / ( size_text + 0
				
				);

			if( new_num_visible > list_strings.size( ) ) { 
				new_num_visible = list_strings.size( );
			}

			if( new_num_visible > num_visible ) { 
				for( unsigned int i = 0; i < new_num_visible - num_visible; ++i ) { 
					auto label = comp_border_text->add_comp( "Label" + std::to_string( id_labels++ ), "Label", [ this ] ( PComp * comp ) {
						auto data = comp->get_data< LabelComp::LabelData >( );
						data->size_text = this->size_text;
						data->text = "Test";
						data->alignment_h = LabelComp::LabelData::AH_Right;
						data->alignment_v = LabelComp::LabelData::AV_Top;

						return 0;
					} );

					list_labels.push_back( label );
				}

				copy_texts( );
				reposition_labels( );
			}
			else if( new_num_visible < num_visible ) { 
				list_labels.erase( 
					list_labels.end( ) - 1 - ( num_visible - new_num_visible ), 
					list_labels.end( ) - 1 );
				reposition_labels( );
			}

			num_visible = new_num_visible;
		}

		void copy_texts( ) { 
			for( unsigned int i = 0; i < list_labels.size( ); ++i ) {
				auto data = list_labels[ i ]->get_data< LabelComp::LabelData >( );
				data->text = list_strings[ list_strings.size( ) - 1 - i ];
			}
		}

		void reposition_labels( ) {
			for( unsigned int i = 0; i < list_labels.size( ); ++i ) { 
				list_labels[ i ]->anchor = { 0.0f, 0.0f };
				list_labels[ i ]->offset = { 
					padding, 
					padding + i * ( size_text + 0 ) 
				};
			}
		}

		void push_command( ) { 
			list_strings.push_back( "" );
			std::swap( list_strings.back( ), data_input->text );
			check_visibles( );
			//reposition_labels( );
		}
	};

	ConsolePage( Client & client );
	~ConsolePage( );
};

