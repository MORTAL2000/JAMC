#include "TimeMgr.h"

#include "Client.h"
#include "glm/gtc/type_ptr.hpp"

const glm::vec3 verts_graph[ 4 ] = {
	{ 0, 0, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 0 },
	{ 0, 1, 0 }
};

// Time Strings
const std::string	
TimeStrings::GAME = "Game",
TimeStrings::GAME_ACCUM = "GAccum",
TimeStrings::RENDER_ACCUM = "RAccum",
TimeStrings::SEC = "Sec";

// Record Strings
const std::string
RecordStrings::FRAME = "Frame",

RecordStrings::UPDATE = "Update",
RecordStrings::UPDATE_PRE = "Upre",
RecordStrings::UPDATE_SEC = "USec",
RecordStrings::UPDATE_QUEUE = "UQue",
RecordStrings::UPDATE_MAP = "UMap",
RecordStrings::UPDATE_TIME = "UTim",
RecordStrings::UPDATE_GUI = "UGui",

RecordStrings::RENDER = "Render",
RecordStrings::RENDER_EXLUSION = "RExl",
RecordStrings::RENDER_SORT = "RSort",
RecordStrings::RENDER_DRAW = "RDraw",
RecordStrings::RENDER_SWAP = "RSwap",

RecordStrings::SLEEP = "Slp",

RecordStrings::CHUNK_ADD = "CAdd",
RecordStrings::CHUNK_INIT = "CInit",
RecordStrings::CHUNK_NOISE = "CNoise",
RecordStrings::CHUNK_READ = "CRead",
RecordStrings::CHUNK_LOAD = "CLoad",
RecordStrings::CHUNK_GEN = "CGen",
RecordStrings::CHUNK_MESH = "CMesh",
RecordStrings::CHUNK_BUFFER = "CBuff",
RecordStrings::CHUNK_SAVE = "CSave",
RecordStrings::CHUNK_REMOVE = "CRem",

RecordStrings::TASK_MAIN = "TMain";

// *** Time Manager ***
TimeMgr::TimeMgr( Client & client ) :
	Manager( client ),
	time_map( time_size ),
	record_map( record_size ) {}

TimeMgr::~TimeMgr() {}

void TimeMgr::init() {

}

// Manager Methods
void TimeMgr::update() {

}

void TimeMgr::render() {

}

void TimeMgr::end() {

}

void TimeMgr::sec() {
}

// Time Methods
void TimeMgr::set_time( std::string const & time_name, float const new_time ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_time );
	auto iter = time_map.find( time_name );
	if( iter != time_map.end() ) {
		iter->second = new_time;
	}
	else {
		time_map.insert( make_pair( time_name, new_time ) );
	}
}

void TimeMgr::add_time( std::string const & time_name, float const delta_time ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_time );
	auto iter = time_map.find( time_name );
	if( iter != time_map.end() ) {
		iter->second += delta_time;
	}
	else {
		time_map.insert( make_pair( time_name, delta_time ) );
	}
}

float TimeMgr::get_time( std::string const & time_name ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_time );
	auto iter = time_map.find( time_name );
	if( iter != time_map.end() ) {
		return iter->second;
	}
	else {
		time_map.insert( make_pair( time_name, 0.0f ) );
		return 0;
	}
}

// Record Methods
void TimeMgr::begin_record( std::string const & record_name ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_record );
	auto iter = record_map.find( record_name );
	if( iter != record_map.end() ) {
		iter->second.start = TimeNow;
	}
	else {
		TimeRecord new_record;
		new_record.start = TimeNow;
		record_map.insert( make_pair( record_name, new_record ) );
	}
}

void TimeMgr::end_record( std::string const & record_name ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_record );
	auto iter = record_map.find( record_name );
	if( iter != record_map.end() ) {
		TimeRecord & record = iter->second;
		record.end = TimeNow;
		record.current = std::chrono::duration_cast< std::chrono::nanoseconds >( record.end - record.start ).count() / 1000000.0f;
	}
	else {
		TimeRecord new_record;
		new_record.current = 0;
		record_map.insert( make_pair( record_name, new_record ) );
	}
}

float TimeMgr::get_record_curr( std::string const & record_name ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_record );
	auto iter = record_map.find( record_name );
	if( iter != record_map.end() ) {
		TimeRecord & record = iter->second;
		return record.current;
	}
	else {
		return 0;
	}
}

void TimeMgr::push_record( std::string const & record_name ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_record );
	auto iter = record_map.find( record_name );
	if( iter != record_map.end() ) {
		TimeRecord & record = iter->second;
		record.history.insert( record.history.begin(), record.current );
		while( record.history.size() > history_size ) {
			record.history.erase( record.history.end() - 1 );
		}
	}
}

TimeRecord & TimeMgr::get_record( std::string const & record_name ) {
	return record_map[ record_name ];
}

float TimeMgr::get_average( std::string const & record_name ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_record );
	auto iter = record_map.find( record_name );
	if( iter != record_map.end() ) {
		TimeRecord & record = iter->second;
		float total = 0.0f;
		for( int i = 0; i < record.history.size(); i++ ) {
			total += record.history.at( i );
		}
		total /= record.history.size();

		return total;
	}
	else {
		return 0;
	}
}

float TimeMgr::get_average( std::string const & record_name, int history ) {
	std::lock_guard<std::recursive_mutex> lock( mutex_record );
	auto iter = record_map.find( record_name );
	if( iter != record_map.end() ) {
		TimeRecord & record = iter->second;
		float total = 0.0f;
		if( history > ( int )record.history.size() ) history = ( int )record.history.size();
		for( int i = 0; i < history; i++ ) {
			total += record.history.at( i );
		}
		total /= history;

		return total;
	}
	else {
		return 0;
	}
}