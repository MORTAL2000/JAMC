#include "ThreadMgr.h"

#include "Client.h"

#include <iostream>
#include <sstream>

int const ThreadMgr::size_prio_init = 10;
int const ThreadMgr::size_func_init = 3000;

ThreadMgr::ThreadMgr( Client & client ) :
	Manager( client ),
	is_shutdown( false ),
	total_threads( std::thread::hardware_concurrency( ) ),
	cnt_total( 0 ),
	cnt_main_total( 0 ), cnt_io_total( 0 ), 
	cnt_async_total( 0 ), cnt_sync_total( 0 ) {
}

ThreadMgr::~ThreadMgr() {}

void ThreadMgr::init() {
	printTabbedLine( 0, "Init ThreadMgr..." );

	std::ostringstream out;

	if( total_threads < 4 ) {
		out.str( "" );
		out << "Sorry, a dual core or greater is needed.";
		printTabbedLine( 1, out.str( ).c_str() );

		client.is_running = false;
	}
	else {
		size_io = 1;
		size_async = total_threads - 2;
		size_sync = size_async;
	}

	out.str( "" );
	out << "Total Threads: " << total_threads;
	printTabbedLine( 1, out.str( ) );

	out.str( "" );
	out << "IO: " << size_io << " ASYNC: " << size_async << " SYNC: " << size_sync;
	printTabbedLine( 1, out.str( ) );

	threads_io.reserve( size_io );
	threads_async.reserve( size_async );
	threads_sync.reserve( size_sync );

	funcs_main.resize( size_prio_init );
	funcs_io.resize( size_prio_init );
	funcs_async.resize( size_prio_init );
	funcs_sync.resize( size_prio_init );

	cnt_prio_main.resize( size_prio_init );
	cnt_prio_main.resize( size_prio_init );
	cnt_prio_main_last.resize( size_prio_init );

	cnt_io.resize( size_io );

	cnt_async.resize( size_async );
	cnt_prio_async.resize( size_prio_init );
	cnt_prio_async_last.resize( size_prio_init );

	cnt_sync.resize( size_sync );

	// Bind process affinity
	HANDLE process = GetCurrentProcess();

	DWORD_PTR processAffinityMask;
	DWORD_PTR systemAffinityMask;

	if( !GetProcessAffinityMask( process, &processAffinityMask, &systemAffinityMask ) ) {
		std::cout << "Error getting affinity." << std::endl;
	}

	out.str( "" );
	out << "System affinity: " << systemAffinityMask;
	printTabbedLine( 1, out.str( ) );

	if( SetProcessAffinityMask( process, systemAffinityMask ) != 1 ) {
		std::cout << "Error setting affinity." << std::endl;
	}

	for( int unsigned i = 0; i < size_io; i++ ) {
		threads_io.push_back( std::thread( &ThreadMgr::loop_io, this, i ) );
	}

	for( int unsigned i = 0; i < size_async; i++ ) {
		threads_async.push_back( std::thread( &ThreadMgr::loop_async, this, i ) );
	}

	for( int unsigned i = 0; i < size_sync; i++ ) {
		threads_sync.push_back( std::thread( &ThreadMgr::loop_sync, this, i ) );
	}

	printTabbedLine( 0, "...Init ThreadMgr" );
	std::cout << std::endl;
}

void ThreadMgr::update( ) {
	auto & out = client.display_mgr.out;
	int total = 0;

	// Print Main cnts
	for( int i = 0; i < funcs_main.size( ); i++ ) {
		total += funcs_main[ i ].size( );
	}

	out.str( "" );
	out << "[Main Queue]: " << total;
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Main Cnt] total: " << cnt_main_total;
	client.gui_mgr.print_to_static( out.str( ) );

	// Print Io cnts
	total = 0;
	for( int i = 0; i < funcs_io.size( ); i++ ) {
		total += funcs_io[ i ].size( );
	}

	out.str( "" );
	out << "[Io Queue] Io: " << total;
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Io Cnt]";
	for( int i = 0; i < cnt_io.size( ); i++ ) {
		out << " t" << i << ": " << cnt_io[ i ];
	}
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Io Cnt]";
	out << " total: " << cnt_io_total;
	client.gui_mgr.print_to_static( out.str( ) );

	// Print Async cnts
	total = 0;
	for( int i = 0; i < funcs_async.size( ); i++ ) { 
		total += funcs_async[ i ].size( );
	}

	out.str( "" );
	out << "[Async Queue]: " << total;
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Async Cnt]";
	for( int i = 0; i < cnt_async.size( ) / 2; i++ ) {
		out << " t" << i << ": " << cnt_async[ i ];
	}
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Async Cnt]";
	for( int i = cnt_async.size( ) / 2; i < cnt_async.size( ); i++ ) {
		out << " t" << i << ": " << cnt_async[ i ];
	}
	client.gui_mgr.print_to_static( out.str( ) );

	out.str( "" );
	out << "[Async Cnt]";
	out << " total: " << cnt_async_total;
	client.gui_mgr.print_to_static( out.str( ) );

}

void ThreadMgr::end() {
	is_shutdown = true;

	cond_io.notify_all();
	cond_async.notify_all();
	cond_sync.notify_all();

	for( int unsigned i = 0; i < size_io; ++i ) {
		threads_io[ i ].join();
	}

	for( int unsigned i = 0; i < size_async; ++i ) {
		threads_async[ i ].join();
	}

	for( int unsigned i = 0; i < size_sync; ++i ) {
		threads_sync[ i ].join();
	}
}

void ThreadMgr::sec() {
	/*task_main( 5, [ & ] ( ) {
		client.gui_mgr.print_to_console( std::string( "test_main_low_prio" ) );
	} );

	task_main( 6, [ & ] ( ) {
		client.gui_mgr.print_to_console( std::string( "test_main_high_prio" ) );
	} );

	task_io( 5, [ & ] ( ) {
		client.gui_mgr.print_to_console( std::string( "test_io" ) );
	} );

	task_sync( 5, [ & ] ( ) {
		client.gui_mgr.print_to_console( std::string( "test_sync" ) );
	} );

	task_async( 5, [ & ] ( ) {
		client.gui_mgr.print_to_console( std::string( "test_async" ) );
	} );*/
}

// could reduce spin time in for loop
void ThreadMgr::loop_main( int const time_max ) {
	std::function < void( ) > func;
	bool is_func = false;
	int prio;

	client.time_mgr.begin_record( RecordStrings::TASK_MAIN );
	client.time_mgr.end_record( RecordStrings::TASK_MAIN );

	while( client.time_mgr.get_record_curr( RecordStrings::TASK_MAIN ) < time_max ) {
		is_func = false;

		{
			std::lock_guard< std::mutex > lock_main( mtx_main );

			select_func_main( is_func, prio, func );
		}

		if( is_func ) {
			{
				std::unique_lock< std::mutex > lock( mtx_cnt );

				cnt_prio_main[ prio ] += 1;
				cnt_prio_main_last[ prio ] = cnt_main_total;

				cnt_main_total += 1;
			}

			func( );
		}
		else {
			client.time_mgr.end_record( RecordStrings::TASK_MAIN );
			break;
		}

		client.time_mgr.end_record( RecordStrings::TASK_MAIN );
	}

	client.time_mgr.push_record( RecordStrings::TASK_MAIN );
}

static int const max_iterm_main = 5;

void ThreadMgr::select_func_main( bool & is_func, int & priority, std::function< void( ) > & func ) {
	for( int i = 0; i < size_prio_init; i++ ) {
		if( !funcs_main[ i ].empty( ) && cnt_main_total - cnt_prio_main_last[ i ] >= max_iterm_main ) {
			priority = i;
			func = std::move( funcs_main[ i ].front( ) );
			funcs_main[ i ].pop( );
			is_func = true;
			return;
		}
	}

	for( int i = size_prio_init - 1; i >= 0; --i ) {
		if( !funcs_main[ i ].empty( ) ) {
			priority = i;
			func = std::move( funcs_main[ i ].front( ) );
			funcs_main[ i ].pop( );
			is_func = true;
			return;
		}
	}
}

void ThreadMgr::task_main( int priority, std::function < void( ) > func ) {
	std::lock_guard< std::mutex > lock( mtx_main );

	if( priority < 0 ) priority = 0;
	else if( priority >= size_prio_init ) priority = size_prio_init - 1;

	funcs_main[ priority ].push( func );
}

void ThreadMgr::loop_io( int const id_thread ) {
	std::function< void() > func;

	while( !is_shutdown ) {
		{
			std::unique_lock< std::mutex > lock( mtx_io );

			cond_io.wait( lock, [ &]() {
				if( is_shutdown ) return true;

				for( int i = size_prio_init - 1; i >= 0; --i ) {
					if( !funcs_io[ i ].empty() ) return true;
				}

				return false;
			} );

			if( is_shutdown ) return;

			for( int i = size_prio_init - 1; i >= 0; --i ) {
				if( !funcs_io[ i ].empty() ) {
					func = std::move( funcs_io[ i ].front( ) );
					funcs_io[ i ].pop();
					break;
				}
			}
		}

		{
			std::unique_lock< std::mutex > lock( mtx_cnt );
			cnt_io_total += 1;
			cnt_io[ id_thread ] += 1;
		}

		func();
	}
}

void ThreadMgr::task_io( int priority, std::function < void() > func ) {
	std::lock_guard< std::mutex > lock( mtx_io );

	if( priority < 0 ) priority = 0;
	else if( priority >= size_prio_init ) priority = size_prio_init - 1;

	funcs_io[ priority ].push( func );

	cond_io.notify_one();
}

void ThreadMgr::loop_async( int const id_thread ) {
	int prio;
	std::function< void() > func;

	while( !is_shutdown ) {
		{
			std::unique_lock<  std::mutex > lock( mtx_async );

			cond_async.wait( lock, [ &]() {
				if( is_shutdown ) return true;

				for( int i = size_prio_init - 1; i >= 0; --i ) {
					if( !funcs_async[ i ].empty() ) return true;
				}

				return false;
			} );

			if( is_shutdown ) return;

			select_func_async( prio, func );
		}

		{
			std::unique_lock< std::mutex > lock( mtx_cnt );

			cnt_prio_async[ prio ] += 1;
			cnt_prio_async_last[ prio ] = cnt_async_total;

			cnt_async[ id_thread ] += 1;
			cnt_async_total += 1;
		}

		func();
	}
}

static int max_interm_async = 5;

void ThreadMgr::select_func_async( int & priority, std::function< void( ) > & func ) {
	for( int i = 0; i < size_prio_init; i++ ) { 
		if( !funcs_async[ i ].empty( ) && cnt_async_total - cnt_prio_async_last[ i ] >= max_interm_async ) { 
			priority = i;
			func = std::move( funcs_async[ i ].front( ) );
			funcs_async[ i ].pop( );
			return;
		}
	}

	for( int i = size_prio_init - 1; i >= 0; --i ) {
		if( !funcs_async[ i ].empty( ) ) {
			priority = i;
			func = std::move( funcs_async[ i ].front( ) );
			funcs_async[ i ].pop( );
			return;
		}
	}
}

void ThreadMgr::task_async( int priority, std::function < void() > func ) {
	std::lock_guard< std::mutex > lock( mtx_async );

	if( priority < 0 ) priority = 0;
	else if( priority >= size_prio_init ) priority = size_prio_init - 1;

	funcs_async[ priority ].push( func );

	cond_async.notify_one();
}

void ThreadMgr::loop_sync( int const id_thread ) {
	std::function< void() > func;

	while( !is_shutdown ) {
		{
			std::unique_lock< std::mutex > lock( mtx_sync );

			cond_sync.wait( lock, [ &]() {
				if( is_shutdown ) return true;

				for( int i = size_prio_init - 1; i >= 0; --i ) {
					if( !funcs_sync[ i ].empty() ) return true;
				}

				return false;
			} );

			if( is_shutdown ) return;

			for( int i = size_prio_init - 1; i >= 0; --i ) {
				if( !funcs_sync[ i ].empty() ) {
					func = std::move( funcs_sync[ i ].front( ) );
					funcs_sync[ i ].pop();
					break;
				}
			}
		}

		{
			std::unique_lock< std::mutex > lock( mtx_cnt );
			cnt_sync_total += 1;
			cnt_sync[ id_thread ] += 1;
		}

		func();
	}
}

void ThreadMgr::task_sync( int priority, std::function < void() > func ) {
	std::lock_guard< std::mutex > lock( mtx_sync );

	if( priority < 0 ) priority = 0;
	else if( priority >= size_prio_init ) priority = size_prio_init - 1;

	funcs_sync[ priority ].push( func );

	cond_sync.notify_one();
}

int ThreadMgr::get_max_prio( ) {
	return size_prio_init;
}

int unsigned ThreadMgr::cnt_thread_io() {
	return size_io;
}

int unsigned ThreadMgr::cnt_thread_async() {
	return size_async;
}

int unsigned ThreadMgr::cnt_thread_sync() {
	return size_sync;
}