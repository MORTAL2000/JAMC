#include "PageComp.h"

#include "Client.h"
#include "Page.h"

PageComp::PageComp( ) :
	client( nullptr ),
	parent( nullptr ),
	vec_pos( 0, 0 ),
	vec_dim( 0, 0 ),
	color( 1.0f, 1.0f, 1.0f, 1.0f ),
	is_visible( true ) { }


PageComp::~PageComp( ) { }

void PageComp::position( ) { 
	vec_pos = glm::ivec2( vec_anchor_pos * glm::vec2( parent->vec_dim ) ) + vec_offset_pos;
	vec_dim = glm::ivec2( vec_anchor_dim * glm::vec2( parent->vec_dim ) ) + vec_offset_dim - vec_pos;
}