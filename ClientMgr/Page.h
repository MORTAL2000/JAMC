#pragma once

#include "Globals.h"

#include <vector>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "ResourceMgr.h"
#include "PageLoader.h"
#include "PageComponent.h"
#include "VBO.h"

class Page {
private:
	std::unordered_map< std::type_index, std::vector< void * > > map_data;

public:
	Page( );
	~Page( );

	Client * client;

	PageLoader * page_loader;

	bool is_visible;
	bool is_remesh;
	bool is_hold;

	std::string name;

	glm::vec2 anchor;
	glm::vec2 offset;

	glm::ivec2 pos;
	glm::ivec2 dim;

	VBO vbo_mesh;
	glm::mat4 mat_model;

	std::vector< Handle< PComp > > list_comps;
	std::unordered_map< std::string, int > map_comps;

private:

public:
	PComp * add_comp( std::string const & name, std::string const & comp, PCFunc func_custom );
	PComp * get_comp( std::string const & name );
	PComp * get_comp_safe( std::string const & name );

	bool on_down( int button );
	bool on_hold( int button );
	bool on_up( int button );

	void reposition( );

	template< class T >
	int get_data_size( ) {
		return map_data[ typeid( T ) ].size( );
	}

	template< class T >
	T * add_data( Client & client ) {
		Handle< T > * ptr_handle = new Handle< T >( );

		if( !client.resource_mgr.allocate( *ptr_handle ) ) {
			delete ptr_handle;
			return nullptr;
		}

		map_data[ typeid( T ) ].push_back( ( void * ) ptr_handle );

		return &ptr_handle->get( );
	}

	template< class T >
	T * get_data( int const index_data = 0 ) {
		return &( ( Handle< T > * ) map_data[ typeid( T ) ][ index_data ] )->get( );
	}

	template< class T >
	void remove_data( int const index_data ) {
		std::type_index index_type = typeid( T );

		auto & vect_data = map_data[ index_type ];

		( ( Handle< T > * ) vect_data[ index_data ] )->release( );

		delete ( Handle< T > * ) vect_data[ index_data ];
		map_data.vect_data.erase( index_data );
	}

	template< class T >
	void clear_data( ) {
		std::type_index index_type = typeid( T );

		auto & vect_data = map_data[ index_type ];

		for( auto data : vect_data ) {
			( ( Handle< T > * ) data )->release( );
			delete ( Handle< T > * ) data;
		}

		vect_data.clear( );
		map_data.erase( index_type );
	}
};

