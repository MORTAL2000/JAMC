#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

#include "LabelComp.h"

class TextFieldComp : public PageComponentLoader {
public:
	struct TextFieldData { 
		int updates;
		int padding;

		std::string text;

		int pos_curs;
		int pos_d;
		int num_d;
		int pos_hl_s;
		int pos_hl_e;

		PComp * comp_label;
		LabelComp::LabelData * data_label;
		PComp * comp_cursor;
		PComp * comp_highlight;
	};

	TextFieldComp( Client & client );
	~TextFieldComp( );
};

