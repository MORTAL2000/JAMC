#pragma once
#include "Globals.h"

#include "Manager.h"

#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <thread>

class ThreadMgr : public Manager {
private:
	static int const size_prio_init;
	static int const size_func_init;

	bool is_shutdown;

	int total_threads;

	int unsigned size_io, size_async, size_sync;
	std::mutex mtx_main, mtx_io, mtx_async, mtx_sync;
	std::condition_variable cond_io, cond_async, cond_sync;
	std::vector< std::thread > threads_io, threads_async, threads_sync;
	std::vector< std::queue< std::function< void() > > > funcs_main, funcs_io, funcs_async, funcs_sync;

	std::mutex mtx_cnt;
	int unsigned cnt_total;
	int unsigned cnt_main_total, cnt_io_total, cnt_async_total, cnt_sync_total;
	std::vector< int unsigned > cnt_io, cnt_async, cnt_sync;
	std::vector< int unsigned > cnt_prio_main, cnt_prio_async;
	std::vector < int unsigned > cnt_prio_main_last, cnt_prio_async_last;

	void loop_io( int const id_thread );
	void loop_async( int const id_thread );
	void loop_sync( int const id_thread );

public:
	ThreadMgr( );
	~ThreadMgr();

	void init();
	void update( );
	void render() {}
	void end();
	void sec();

	void task_main( int priority, std::function< void( ) > func );
	void task_io( int priority, std::function< void() > func );
	void task_async( int priority, std::function< void() > func );
	void task_sync( int priority, std::function< void() > func );

	void select_func_main( bool & is_func, int & priority, std::function< void( ) > & func );
	void select_func_async( int & priority, std::function< void( ) > & func );

	void loop_main( int const time_max );

	int get_max_prio( );

	int unsigned cnt_thread_io();
	int unsigned cnt_thread_async();
	int unsigned cnt_thread_sync();
};