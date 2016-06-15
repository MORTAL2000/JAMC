#pragma once

#include "Globals.h"

#include "ResourceMgr.h"
#include "PageFuncs.h"
#include "PageComp.h"
#include "VBO.h"

#include <vector>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

class Page {
public:
	Client * client;

	VBO vbo;
	glm::mat4 mat_model;

	glm::ivec2 vec_pos;
	glm::ivec2 vec_dim;

	std::string str_name;

	Color4 color;

	bool is_dirty;
	bool is_hold;
	bool is_edit;
	bool is_visibile;

	int cnt_update = 0;

	std::vector< Handle< PageComp > > list_comps;
	std::unordered_map< std::string, PageComp * > map_comps;

public:
	Page( );
	~Page( );

	std::unordered_map< std::type_index, std::unordered_map< std::string, void * > > map_data;

public:
	void add_comp( std::string & str_name,
		FuncComp func_alloc, FuncComp func_custom );

	PageComp & get_comp( std::string & str_name );

	bool on_down( int button );
	bool on_hold( int button );
	bool on_up( int button );

	void update( );

	template< class T >
	void add_data( std::string & str_data ) {
		std::type_index index_type = typeid( T );
		auto iter_data = map_data.find( index_type );

		if( iter_data != map_data.end( ) ) { 
			auto & map_str = iter_data->second;
			auto iter_str = map_str.find( str_data );

			if( iter_str == map_str.end( ) ) {
				Handle< T > * ptr_handle = new Handle< T >( );
				if( client->resource_mgr.allocate( *ptr_handle ) ) {
					map_str.insert( { str_data, ( void * ) ptr_handle } );
				}
				else {
					delete ptr_handle;
				}
			}
		}
		else {
			Handle< T > * ptr_handle = new Handle< T >( );
			if( client->resource_mgr.allocate( *ptr_handle ) ) {
				auto iter_str = map_data.insert( { index_type, std::unordered_map< std::string, void * >( ) } ).first;
				iter_str->second.insert( { str_data, ( void * ) ptr_handle } );
			}
			else {
				delete ptr_handle;
			}
		}
	}

	template< class T >
	T & get_data( std::string & str_data ) {
		return ( ( Handle< T > * ) map_data[ typeid( T ) ][ str_data ] )->get( );
	}

	template< class T >
	std::unordered_map< std::string, void * > & get_map_data( ) {
		return map_data[ typeid( T ) ];
	}

	template< class T >
	void remove_data( std::string & str_data ) {
		std::type_index index_type = typeid( T );
		Handle< T > * handle_data = ( Handle< T > * ) map_data[ index_type ][ str_data ];
		handle_data->release( );
		delete handle_data;
		map_data[ index_type ].erase( str_data );
	}

	/*
	template< class T >
	void clear_data( ) {
		std::type_index index_type = typeid( T );
		auto & vect_data = map_data[ index_type ];
		for( auto data : vect_data ) {
			( ( Handle< T > * ) data )->release( );
			delete ( Handle< T > * ) data;
			vect_data.clear( );
			map_data.erase( index_type );
		}
	}*/

	void resize( );
};

