#pragma once
#include "Globals.h"

#include "ResourceMgr.h"
#include "PageLoader.h"
#include "PageComponent.h"
#include "VBO.h"

#include <vector>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>


class Page {
public:
	Page( );
	~Page( );

	Client * client;
	Handle< PComp > h_root;
	PComp * root;
	PageLoader * loader;

	glm::ivec2 pos;

	bool is_visible;
	bool is_remesh;

	std::string name;

	VBO vbo_mesh;
	glm::mat4 mat_model;

private:

public:
	glm::ivec2 & get_pos( );
	void set_pos( int x, int y );
	void set_pos( glm::ivec2 const & pos );

	glm::ivec2 & get_dim( );
	void set_dim( glm::ivec2 const & dim );

	glm::vec2 & get_anchor( );
	void set_anchor( glm::vec2 offset );

	glm::vec2 & get_offset( );
	void set_offset( glm::vec2 offset );

	PComp * add_comp( std::string const & name, std::string const & comp, PCFunc func_custom );
	PComp * get_comp( std::string const & name );
	PComp * get_comp_safe( std::string const & name );

	template< class T >
	int get_size( ) {
		return root->get_size( );
	}

	template< class T >
	T * add_data( ) {
		return root->add_data< T >( );
	}

	template< class T >
	T * get( int const index_data = 0 ) {
		return root->get< T >( index_data );
	}

	template< class T >
	void remove_data( int const index_data ) {
		root->remove_data< T >( int const index_data );
	}

	template< class T >
	void clear_data( ) {
		root->clear_data< T >( );
	}

	void reposition( );
};

