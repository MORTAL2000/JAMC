#include "Page.h"

#include "Client.h"
#include "Directional.h"

const glm::vec3 verts_page[ 4 ] = { 
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 } 
};

Page::Page( ) { }

Page::~Page( ) { }

glm::ivec2 & Page::get_pos( ) {
	return pos;
}

void Page::set_pos( int x, int y ) { 
	pos.x = x;
	pos.y = y;
}

void Page::set_pos( glm::ivec2 const & pos ) { 
	this->pos = pos;
}

glm::ivec2 & Page::get_dim( ) {
	return root->dim;
}

void Page::set_dim( glm::ivec2 const & dim ) { 
	root->dim = dim;
}

glm::vec2 & Page::get_anchor( ) {
	return root->anchor;
}

void Page::set_anchor( glm::vec2 offset ) { 
	root->anchor = offset;
}

glm::vec2 & Page::get_offset( ) {
	return root->offset;
}

void Page::set_offset( glm::vec2 offset ) { 
	root->offset = offset;
}

PComp * Page::add_comp( std::string const & name_comp, std::string const & name_loader, PCFunc func_custom ) {
	return 	root->add_comp( name_comp, name_loader, func_custom );
}

PComp * Page::get_comp( std::string const & name ) {
	return root->get_comp( name );
}

PComp * Page::get_comp_safe( std::string const & name ) {
	return root->get_comp_safe( name );
}

void Page::reposition( ) { 
	set_pos( 
		client->display_mgr.get_window( ).x * root->anchor.x + root->offset.x,
		client->display_mgr.get_window( ).y * root->anchor.y + root->offset.y );

	mat_model = glm::translate( glm::mat4( ), glm::vec3( pos, 0 ) );
}