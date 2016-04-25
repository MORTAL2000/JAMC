#pragma once
#include "Globals.h"

#include "ThreadMgr.h"

#include <vector>
#include <mutex>
#include <functional>


class AbstractHandle {
public:
	int id;
	virtual void release( ) = 0;

	AbstractHandle( ) :
		id( -1 ) { }
};

template< class T > class Handle :
	public AbstractHandle {
public:
	ResPool< T >* pool;

	Handle( ) :
		AbstractHandle( ),
		pool( NULL ) { }

	Handle( int const id, ResPool< T > & pool ) :
		id( id ),
		pool( pool ) { }

	Handle( Handle< T > const & rho ) {
		id = rho.id;
		pool = rho.pool;
	}
	//Handle(Handle< T > const & rho) = delete;

	T& get( ) { return pool->list_data[ id ]; }
	void release( ) { if( id != -1 ) pool->release( *this ); }
	bool is_live( ) { return id != -1; }
};

class Pool {
public:
	Pool( ) { }
	~Pool( ) { }
};

template < class T > class ResPool : public Pool {
private:
	friend T& Handle< T >::get( );

	int size_max;
	int id_first, id_last, id_next;
	int cnt_live;

	std::vector< bool > list_live;
	std::vector< T > list_data;

public:
	ResPool( ) :
		Pool( ),
		size_max( 100 ),
		id_first( 100 ), id_last( -1 ), id_next( 0 ),
		cnt_live( 0 ),
		list_live( 100, false ),
		list_data( 100, T( ) ) { }

	ResPool( int const size_init ) :
		Pool( ),
		size_max( size_init ),
		id_first( size_init ), id_last( -1 ), id_next( 0 ),
		cnt_live( 0 ),
		list_live( size_init, false ),
		list_data( size_init, T( ) ) { }

	~ResPool( ) { }

	bool allocate( Handle< T > & handle ) {
		if( id_next >= size_max ) { return false; }
		if( id_next < id_first ) { id_first = id_next; }
		if( id_next > id_last ) { id_last = id_next; }

		handle.id = id_next;
		handle.pool = this;

		cnt_live += 1;

		list_live[ id_next ] = true;

		do {
			++id_next;
		} while( id_next < size_max && list_live[ id_next ] );

		return true;
	}

	void release( Handle< T > & handle ) {
		if( handle.id < id_next ) { id_next = handle.id; }
		if( handle.id == id_first ) {
			do {
				++id_first;
			} while( id_first < size_max && !list_live[ id_first ] );
		}
		if( handle.id == id_last ) {
			do {
				--id_last;
			} while( id_last >= 0 && !list_live[ id_last ] );
		}

		cnt_live -= 1;

		list_live[ handle.id ] = false;

		handle.id = -1;
		handle.pool = NULL;
	}

	void apply_func_all( std::function< void( T & ) > func ) {
		for( int i = 0; i < size_max; ++i ) {
			func( list_data[ i ] );
		}
	}

	void apply_func_live( std::function< void( T & ) > func ) {
		for( int i = id_first; i <= id_last; ++i ) {
			if( list_live[ i ] )
				func( list_data[ i ] );
		}
	}

	void apply_func_range( int const begin, int const end,
		std::function< void( T & ) > func ) {
		for( int i = begin; i <= end; ++i ) {
			if( list_live[ i ] )
				func( list_data[ i ] );
		}
	}

	void apply_func_live_threads( ThreadMgr & thread_mgr, int const priority,
		int const num_threads, std::function< void( T & ) > func ) {

		int first = id_first;
		int last = id_last;
		int range = last - first + 1;
		float step = float( range ) / num_threads;

		std::mutex mutex_back;
		std::condition_variable cond_callback;
		int num_callback = 0;

		std::unique_lock< std::mutex > lock_callback( mutex_back );

		thread_mgr.task_sync( priority, [ & ]( ) {
			apply_func_range( first, first + int( step ), func );

			std::lock_guard< std::mutex > lock( mutex_back );
			num_callback += 1;
			cond_callback.notify_one( );
		} );

		for( int i = 1; i < num_threads - 1; i++ ) {
			thread_mgr.task_sync( priority, [ &, i ]() {
				apply_func_range( first + int( step * i ) + 1, first + int( step  * ( i + 1 ) ), func );

				std::lock_guard< std::mutex > lock( mutex_back );
				num_callback += 1;
				cond_callback.notify_one( );
			} );
		}

		thread_mgr.task_sync( priority, [ &]() {
			apply_func_range( first + int( step * ( num_threads - 1 ) ) + 1, last, func );

			std::lock_guard< std::mutex > lock( mutex_back );
			num_callback += 1;
			cond_callback.notify_one( );
		} );

		cond_callback.wait( lock_callback, [ & ]() {
			return num_callback == num_threads;
		} );
	}

	bool resize( int const size ) {
		if( cnt_live == 0 ) {
			size_max = size;
			list_live.clear( );
			list_live.shrink_to_fit( );
			list_live.resize( size_max, false );

			list_data.clear( );
			list_data.shrink_to_fit( );
			list_data.resize( size_max, T( ) );

			return true;
		}
		return false;
	}

	int size( ) {
		return cnt_live;
	}

	int capacity( ) {
		return size_max;
	}

	friend std::ostream& operator<<( std::ostream & lho, const ResPool< T > & rho ) {
		return lho << "rp[ a:" << rho.cnt_live << " m:" << rho.size_max << " f:" << rho.id_first << " n:" << rho.id_next << " l:" << rho.id_last << " ]";
	}
};