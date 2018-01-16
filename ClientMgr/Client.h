#pragma once
#include "Globals.h"

#include "StaticLibs.h"

#include "Manager.h"
#include "ResourceMgr.h"
#include "TextureMgr.h"
#include "TimeMgr.h"
#include "ThreadMgr.h"
#include "DisplayMgr.h"
#include "InputMgr.h"
#include "GuiMgr.h"
#include "ChunkMgr.h"
#include "EntityMgr.h"
#include "BiomeMgr.h"
#include "BlockMgr.h"

//#include <stdlib.h>
#include <iostream>

class Client : 
	public Manager {

private:
	std::mutex mutex_thead_ids;
	std::vector< std::string > list_thread_ids;

	MSG msg;

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

	ResourceMgr		resource_mgr;
	TextureMgr		texture_mgr;
	TimeMgr			time_mgr;
	ThreadMgr		thread_mgr;
	DisplayMgr		display_mgr;
	InputMgr		input_mgr;
	GuiMgr			gui_mgr;
	BlockMgr		block_mgr;
	BiomeMgr		biome_mgr;
	ChunkMgr		chunk_mgr;
	EntityMgr		entity_mgr;

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