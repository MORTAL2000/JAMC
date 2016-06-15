#pragma once

#include "Globals.h"

#include "Manager.h"
#include "Chunk.h"
#include "VBO.h"

#include <chrono>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

#define TimePoint std::chrono::high_resolution_clock::time_point
#define TimeDuration std::chrono::high_resolution_clock::duration
#define TimeNow std::chrono::high_resolution_clock::now()

static int const	history_size = 144 * 5,
					time_size = 64,
					record_size = 64;

struct TimeStrings {
	static const std::string
		GAME,
		GAME_ACCUM,
		RENDER_ACCUM,
		SEC;
};

struct RecordStrings {
	static const std::string
		FRAME,
		UPDATE,
		UPDATE_PRE,
		UPDATE_SEC,
		UPDATE_QUEUE,
		UPDATE_MAP,
		UPDATE_TIME,
		UPDATE_GUI,
		RENDER,
		RENDER_DRAW,
		RENDER_EXLUSION,
		RENDER_SORT,
		RENDER_SWAP,
		SLEEP,
		CHUNK_ADD,
		CHUNK_INIT,
		CHUNK_NOISE,
		CHUNK_READ,
		CHUNK_LOAD,
		CHUNK_GEN,
		CHUNK_MESH,
		CHUNK_BUFFER,
		CHUNK_SAVE,
		CHUNK_REMOVE,
		TASK_MAIN;
};

struct TimeRecord {
	TimePoint start;
	TimePoint end;
	float current;
	std::vector< float > history;

	TimeRecord() : 
		start( TimeNow ),
		end( TimeNow ),
		current( 0 ) {
		history.reserve( history_size );
	}
};

class TimeMgr :
	public Manager {

private:
	std::recursive_mutex mutex_time, mutex_record;
	std::unordered_map< std::string, float > time_map;
	std::unordered_map< std::string, TimeRecord > record_map;

	std::vector< std::string const * > list_render;

	static const int padding = 5;
	static glm::ivec2 dim_graph;

	VBO vbo;
	bool is_dirty;
	void mesh_graphs( );

public:
	TimeMgr( Client & client );
	~TimeMgr();

	void init();
	void update();
	void render();
	void end();
	void sec();

	void set_time( std::string const & time_name, float const new_time );
	void add_time( std::string const & time_name, float const delta_time );

	float get_time( std::string const & time_name );

	void begin_record( std::string const & record_name );
	void end_record( std::string const & record_name );
	float get_record_curr( std::string const & record_name );

	void push_record( std::string const & record_name );

	TimeRecord & get_record( std::string const & record_name );

	float get_average( std::string const & record_name );
	float get_average( std::string const & record_name, int  history );
};