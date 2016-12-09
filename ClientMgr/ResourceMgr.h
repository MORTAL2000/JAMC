#pragma once

#include "Globals.h"

#include "Manager.h"
#include "ResPool.h"

class ResourceMgr : public Manager {
private:
	static int const size_pool_init = 64;
public:
	ResourceMgr( ) : 
		Manager( ) {}
	~ResourceMgr() {}

	template< class T, int ident_pool = 0 >
	ResPool< T > & pool() {
		static ResPool< T > static_pool( size_pool_init );
		return static_pool;
	}

	template< class T, int ident_pool = 0 >
	bool reg_pool( int const size_pool ) {
		return pool< T, ident_pool >().resize( size_pool );
	}

	template< class T, int ident_pool = 0 >
	bool allocate( Handle< T > & ptr_handle ) {
		return pool< T, ident_pool >().allocate( ptr_handle );
	}

	template< class T >
	void release( Handle< T > & ptr_handle ) {
		ptr_handle.release();
	}

	void init() {}
	void update() {}
	void render() {}
	void end() {}
	void sec() {}
};