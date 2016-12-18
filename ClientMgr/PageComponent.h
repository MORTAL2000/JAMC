#pragma once

#include <unordered_map>
#include <typeindex>

#include "glm\glm.hpp"

#include "PageComponentLoader.h"

class Page;
class Client;

class PageComponent {
private:
	std::unordered_map< std::type_index, std::vector< void * > > map_data;
	
public:
	PageComponent( );
	~PageComponent( );

	bool is_visible;
	bool is_hold;

	Page * page;
	PageComponent * parent;
	PCLoader * pc_loader;

	std::string name;

	glm::vec2 anchor;
	glm::vec2 offset;

	glm::ivec2 pos;
	glm::ivec2 dim;

private:

public:
	template< class T >
	int get_data_size( ) {
		return map_data[ typeid( T ) ].size( );
	}

	template< class T >
	bool add_data( Client & client ) {
		Handle< T > * ptr_handle = new Handle< T >( );

		if( !client.resource_mgr.allocate( *ptr_handle ) ) {
			delete ptr_handle;
			return false;
		}

		map_data[ typeid( T ) ].push_back( ( void * ) ptr_handle );

		return true;
	}

	template< class T >
	T & get_data( int const index_data = 0 ) {
		return ( ( Handle< T > * ) map_data[ typeid( T ) ][ index_data ] )->get( );
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

	void reposition( );
};