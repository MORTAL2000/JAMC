#pragma once
#include "Globals.h"

#include "Manager.h"
#include "ResPool.h"

class ResourceMgr : 
	public Manager {
private:
	static int const size_pool_init = 64;
public:
	ResourceMgr( Client & client ) :
		Manager( client ) {}
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
	T * allocate( Handle< T > & ptr_handle ) {
		return pool< T, ident_pool >().allocate( ptr_handle );
	}

	template< class T >
	void release( Handle< T > & ptr_handle ) {
		ptr_handle.release();
	}

	void init() override {}
	void update() override {}
	void render() override {}
	void end() override {}
	void sec() override {}
};