#pragma once
#include "Globals.h"

#include <iostream>

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
#include "BlockMgr.h"
#include "BiomeMgr.h"

class Client {
public:
	static Client & get_instance( ) { 
		static Client client;
		return client;
	}

private:
	void thread_main_loop();

	int update_last, render_last;
	int update_cnt, render_cnt;

	void render_output();

public:
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

	LRESULT CALLBACK WndProc( HWND p_hWnd, UINT p_uiMessage, WPARAM p_wParam, LPARAM p_lParam );
};

Client & get_client( ) { 
	return Client::get_instance( );
}