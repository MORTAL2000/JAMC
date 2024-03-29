#pragma once
#include "Globals.h"

#include "ResourceMgr.h"
#include "SharedMesh.h"

#include "glm\glm.hpp"

#include <functional>
#include <typeindex>
#include <unordered_map>

typedef std::function< int( Client &, Entity & ) > EFAlloc;
typedef std::function< int( Client &, Entity & ) > EFCustom;
typedef std::function< int( Client &, Entity & ) > EFUpdate;
typedef std::function< int( Client &, Entity & ) > EFRelease;
typedef std::function< int( Client &, Entity & ) > EFMesh;

using EntityVertex = BaseVertex< float, float, float, float >;
using SMEntity = SharedMesh< EntityVertex >;

enum ErrorEntity {
	EE_Ok,
	EE_Failed,
	EE_Size
};

struct ErrorEntityLookup {
	static const char * to_text[ ];
};

struct EntityLoader {
	std::string const name;

	EFAlloc ef_alloc;
	EFRelease ef_release;
	EFUpdate ef_update;
	EFMesh ef_mesh;
};

struct ECState {
	glm::vec3 pos;
	glm::vec3 pos_last;
	glm::vec3 pos_delta;
	glm::vec3 veloc;
	glm::vec3 accel;

	glm::vec3 rot;
	glm::vec3 rot_veloc;
	glm::vec3 rot_accel;

	glm::vec3 dim;

	glm::mat4 mat_model;
	glm::mat3 mat_norm;

	glm::vec3 gravity;

	int id_block;

	bool is_coll;
	bool is_coll_face[ 6 ];
};

class Entity {
public:
	Handle< ECState > h_state;

	SMEntity::SMHandle h_mesh;

	int id = 0;
	int time_live = 0;
	EntityLoader * loader;
	glm::vec4 color;

	std::unordered_map< std::type_index, std::vector< void * > > map_data;

	bool is_visible;
	bool is_shutdown;
	bool is_dirty;

	Entity( );
	~Entity( );

public:
	template< class T >
	int get_size( ) { 
		return map_data[ typeid( T ) ].size( );
	}

	/*template< class T >
	void set( int const index_data, Handle< T > & data ) { 
		auto & vect_data = map_data[ typeid( T ) ];
		( ( Handle< T > * ) vect_data[ index_data ] )->release( );
		vect_data[ index_data ] = &data;
	}*/

	template< class T >
	bool add_data( Client & client ) { 
		Handle< T > * ptr_handle = new Handle< T >( );

		if( !client.resource_mgr.allocate( *ptr_handle ) ) { 
			delete ptr_handle;
			return false;
		}

		map_data[ typeid( T ) ].push_back( ( void * )ptr_handle );

		return true;
	}

	template< class T > 
	Handle< T > & get( int const index_data = 0 ) { 
		return *( ( Handle< T > * ) map_data[ typeid( T ) ][ index_data ] );
	}

	template< class T >
	void remove_data( int const index_data ) { 
		std::type_index index_type = typeid( T );

		auto & vect_data = map_data[ index_type ];

		( ( Handle< T > * ) vect_data[ index_data ] )->release();

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

