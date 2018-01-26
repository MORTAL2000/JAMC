#pragma once

#include "Client.h"
#include "PageComponentLoader.h"
#include "BorderImageComp.h"
#include "LabelComp.h"

#include <iostream>
#include <iomanip>
#include <sstream>

class SliderComp : public PageComponentLoader {
public:
	struct SliderData {
		PCFunc func_read;
		PCFunc func_write;

		int unsigned cnt_remesh;

		int unsigned id_texture;
		int unsigned id_subtex;

		float ratio;

		float value;

		float lower;
		float upper;

		float width_bar;
		float length_bar;

		PComp * comp_border;
		PComp * comp_bar;
		PComp * comp_slider;

		PComp * comp_label_title;
		PComp * comp_label_left;
		PComp * comp_label_right;
		PComp * comp_label_value;

		BorderImageComp::BorderImageData * data_border;
		BorderImageComp::BorderImageData * data_bar;
		BorderImageComp::BorderImageData * data_slider;

		LabelComp::LabelData * data_label_title;
		LabelComp::LabelData * data_label_left;
		LabelComp::LabelData * data_label_right;
		LabelComp::LabelData * data_label_value;

		void set_bounds( float lower, float upper ) { 
			this->lower = lower;
			this->upper = upper;

			std::ostringstream out;
			out << std::setprecision( 1 ) << std::fixed;
			out << lower;
			data_label_left->text = out.str( );
			out.str( "" );
			out << upper;
			data_label_right->text = out.str( );
		}

		void set_value( float new_value ) {
			value = new_value;
			if( value < lower ) value = lower;
			if( value > upper ) value = upper;

			ratio = ( value - lower ) / ( upper - lower );

			std::ostringstream out;
			out << std::setprecision( 1 ) << std::fixed;
			out << value;
			data_label_value->text = out.str( );
		}

		void set_ratio( float new_ratio ) { 
			ratio = new_ratio;

			if( ratio < 0.0f ) ratio = 0.0f;
			if( ratio > 1.0f ) ratio = 1.0f;

			value = lower + ( upper - lower ) * ratio;
		
			std::ostringstream out;
			out << std::setprecision( 1 ) << std::fixed;
			out << value;
			data_label_value->text = out.str( );
		}
	};

	SliderComp( Client & client );
	~SliderComp( );
};

