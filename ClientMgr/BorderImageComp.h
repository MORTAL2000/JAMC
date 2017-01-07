#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class BorderImageComp : public PageComponentLoader {
public:
	struct BorderImageData {
		//PCFunc func_resize;

		glm::vec4 color;

		int unsigned padding_border;
		int unsigned padding_texture;

		int unsigned id_texture;
		int unsigned id_subtex;

		float d_pix;
		float dh_pix;
		float d_uv;

		void set_texture( 
			Client & client, std::string const & name_texture, 
			std::string const & name_subtex, int unsigned padding ) {

			auto texture = client.texture_mgr.get_texture( name_texture );

			padding_texture = padding;

			id_texture = texture->id_texture;
			id_subtex = client.texture_mgr.get_texture_layer( name_texture, name_subtex );

			d_pix = 1.0f / texture->dim_texture.x;
			dh_pix = 0.0f;
			d_uv = d_pix * padding_texture;
		}
	};

	BorderImageComp( Client & client );
	~BorderImageComp( );
};

