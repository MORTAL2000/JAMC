#pragma once

#include "Client.h"
#include "PageComponentLoader.h"

class ImageComp : public PageComponentLoader {
public:
	struct ImageData { 
		glm::vec4 color;

		int unsigned id_texture;
		int unsigned id_subtex;

		void set_texture( Client & client, std::string const & name_texture, std::string const & name_subtex ) { 
			id_texture = client.texture_mgr.get_texture_id( name_texture );
			id_subtex = client.texture_mgr.get_texture_layer( name_texture, name_subtex );
		}
	};

	ImageComp( Client & client );
	~ImageComp( );
};

