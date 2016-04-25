#pragma once

#include "Globals.h"

#include "PageCompData.h"

#include <functional>

typedef std::function< bool( PageComp & comp ) > FuncComp;

class PageCompFuncs {
public:
	static FuncComp cust_null;

	// Comp base
	static FuncComp alloc_base;
	static FuncComp on_down_debug;

	// Comp Close
	static FuncComp alloc_close;
	static FuncComp on_down_close;

	// Comp Edit
	static FuncComp alloc_edit;
	static FuncComp on_down_edit;

	// Comp resizes
	static FuncComp alloc_resize_top_left;
	static FuncComp alloc_resize_bot_left;
	static FuncComp alloc_resize_top_right;
	static FuncComp alloc_resize_bot_right;

	static FuncComp alloc_resize_left;
	static FuncComp alloc_resize_right;
	static FuncComp alloc_resize_top;
	static FuncComp alloc_resize_bot;

	static FuncComp on_down_resize;

	static FuncComp on_hold_resize_top_left;
	static FuncComp on_hold_resize_bot_left;
	static FuncComp on_hold_resize_top_right;
	static FuncComp on_hold_resize_bot_right;

	static FuncComp on_hold_resize_left;
	static FuncComp on_hold_resize_right;
	static FuncComp on_hold_resize_top;
	static FuncComp on_hold_resize_bot;

	static FuncComp on_up_resize;

	// Comp Titlebar
	static FuncComp alloc_titlebar;
	static FuncComp on_down_titlebar;
	static FuncComp on_hold_titlebar;
	static FuncComp on_up_titlebar;

	// Comp Console
	static FuncComp alloc_console;
	static FuncComp update_console;

	// Comp Command
	static FuncComp alloc_command;
	static FuncComp update_command;
	static FuncComp on_down_command;

	// Comp Static
	static FuncComp alloc_static;
	static FuncComp update_static;
};