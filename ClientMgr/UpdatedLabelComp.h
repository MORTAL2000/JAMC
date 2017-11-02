#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class UpdatedLabelComp : public PageComponentLoader {
public:
	struct UpdatedLabelData {
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

		std::function< std::string( ) > func_update;
	};

	UpdatedLabelComp( Client & client );
	~UpdatedLabelComp( );
};

