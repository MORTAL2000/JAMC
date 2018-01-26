#pragma once
#include "Globals.h"

#define NOMINMAX

#include "Manager.h"

#include "glm/glm.hpp"

#include <iostream>
#include <vector>
#include <mutex>


class Client : 
	public Manager {

private:
	std::mutex mutex_thead_ids;
	std::vector< std::string > list_thread_ids;

	std::thread thread_moving;

	bool is_moving;

	void thread_main_loop();
	void main_loop( );

	int update_last, render_last;
	int update_cnt, render_cnt;

	void render_output();

public:
	static const int size_output = 15;
	static const int size_padding = 5;

	ResourceMgr &	resource_mgr;
	TextureMgr &	texture_mgr;
	TimeMgr &		time_mgr;
	ThreadMgr &		thread_mgr;
	DisplayMgr &	display_mgr;
	InputMgr &		input_mgr;
	GuiMgr &		gui_mgr;
	BlockMgr &		block_mgr;
	BiomeMgr &		biome_mgr;
	ChunkMgr &		chunk_mgr;
	EntityMgr &		entity_mgr;

	bool is_running;
	glm::ivec2 pos_output;

	Client();
	~Client();

	void init();
	void init_mgrs( );
	void update();
	void render();
	void end();
	void sec();

	int cnt_update( );
};