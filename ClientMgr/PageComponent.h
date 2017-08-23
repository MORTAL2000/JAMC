#pragma once

#include <unordered_map>
#include <typeindex>

#include "glm\glm.hpp"

#include "ResourceMgr.h"
#include "PageComponentLoader.h"

class Page;
class Client;

class PageComponent {
private:
	std::unordered_map< std::type_index, std::vector< void * > > map_data;
	
public:
	PageComponent( );
	~PageComponent( );

	Client * client;

	bool is_visible;
	bool is_hold;

	Page * page;
	PComp * parent;
	PCLoader * pc_loader;

	std::string name;

	glm::vec2 anchor;
	glm::vec2 offset;

	glm::ivec2 pos;
	glm::ivec2 dim;

	std::vector< Handle< PComp > > list_comps;
	std::unordered_map< std::string, int > map_comps;

private:

public:
	PComp * add_comp( std::string const & name, std::string const & comp, PCFunc func_custom );
	PComp * get_comp( std::string const & name );
	PComp * get_comp_safe( std::string const & name );

	template< class T >
	int get_size( ) {
		return map_data[ typeid( T ) ].size( );
	}

	template< class T >
	T * add_data() {
		Handle< T > * ptr_handle = new Handle< T >( );

		if( !client->resource_mgr.allocate( *ptr_handle ) ) {
			delete ptr_handle;
			return nullptr;
		}

		map_data[ typeid( T ) ].push_back( ( void * ) ptr_handle );

		return &ptr_handle->get( );
	}

	template< class T >
	T * get( int const index_data = 0 ) {
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

	//template< class T >
	void clear_data( ) {
		/*
		std::type_index index_type = typeid( T );

		auto & vect_data = map_data[ index_type ];

		for( auto data : vect_data ) {
			( ( Handle< T > * ) data )->release( );
			delete ( Handle< T > * ) data;
		}

		vect_data.clear( );
		map_data.erase( index_type );
		*/

		for( auto & vec_data : map_data ) {
			for( auto & data : vec_data.second ) {
				( ( AbstractHandle * ) ( data ) )->release( );
				delete ( AbstractHandle * ) ( data );
			}
			vec_data.second.clear( );
			vec_data.second.shrink_to_fit( );
		}

		map_data.clear( );
	}

	void reposition( );
};