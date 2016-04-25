#pragma once

#include "Globals.h"

#include "Manager.h"
#include "glm/glm.hpp"

struct Mouse {
	glm::ivec2 pos;
	glm::ivec2 pos_relative;
	glm::ivec2 pos_delta;
	glm::ivec2 pos_last;

	bool action[2];
	bool down[2];
	bool up[2];
	int dur[2];
	int hold[2];

	glm::ivec2 pos_down;
	glm::ivec2 pos_up;

	glm::ivec2 pos_reset;

	POINT point_poll;

	int accum_wheel_delta;
	int wheel_delta;

	bool is_visible;

	Mouse( ) : is_visible( true ),
		pos( 0, 0 ), pos_last( 0, 0 ),
		pos_delta( 0, 0 ), pos_reset( 0, 0 ),
		pos_down( 0, 0 ), pos_up( 0, 0 ),
		accum_wheel_delta( 0 ), wheel_delta( 0 ) {
		down[0] = false; down[1] = false;
		up[0] = false; up[1] = false;
		dur[0] = 0; dur[1] = 0;
		hold[0] = 0; hold[1] = 0;
	}
};

struct Keyboard {
	bool keys[256];

	Keyboard( ) {
		fill_array( keys, false );
	}
};

class InputMgr :
	public Manager {

private:
	int last_command;

	Mouse mouse;
	Keyboard keyboard;

	void poll_mouse_pos( );
	void resolve_mouse_action( );
	void process_input( );

public:
	InputMgr( Client & client );
	~InputMgr( );

	void init( );
	void update( );
	void render( ) { }
	void end( ) { }
	void sec( ) { }

	void set_key( int const key, bool const is_down );
	bool is_key( int const key );

	glm::ivec2 & get_mouse( );
	glm::ivec2 & get_mouse_relative( );
	glm::ivec2 & get_mouse_delta( );
	glm::ivec2 & get_mouse_down( int const button );
	glm::ivec2 & get_mouse_up( int const button );
	int get_mouse_dur( int const button );
	int get_mouse_hold( int const button );

	void set_mouse( glm::ivec2 const & new_pos );

	void set_mouse_button( int const button, bool const is_down );

	bool is_mouse_down( int const button );
	bool is_mouse_up( int const button );

	void add_wheel_delta( int const wheel_delta );
	int get_wheel_delta( );

	bool is_cursor_vis( );
	void toggle_cursor_vis( );

};