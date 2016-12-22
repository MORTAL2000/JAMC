#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class LabelComp : public PageComponentLoader {
public:
	struct LabelData {
		enum AlignHorizontal { 
			AH_Left,
			AH_Center,
			AH_Right
		};

		enum AlignVertical {
			AV_Top,
			AV_Center,
			AV_Bottom
		};

		int size_text;
		std::string text;

		AlignHorizontal alignment_h = AlignHorizontal::AH_Right;
		AlignVertical alignment_v = AlignVertical::AV_Top;
	};

	LabelComp( Client & client );
	~LabelComp( );
};

