#pragma once

#include "Globals.h"

#include "Manager.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "TimeMgr.h"
#include "Chunk.h"
#include "BlockSelector.h"

#include <Windows.h>
#include <sstream>

typedef std::array< std::array< float, 2 >, 4 > face_uvs;

struct MVPMatrices { 
	glm::vec4 pos_camera;
	glm::mat4 mat_world;
	glm::mat4 mat_perspective;
	glm::mat4 mat_ortho;
	glm::mat4 mat_view;
	float time_game;
};

class Camera {
public:
	MVPMatrices mvp_matrices;

	glm::mat4 mat_translation;
	glm::mat4 mat_rotation;

	glm::vec3 pos_camera;
	glm::vec3 rot_camera;

	glm::vec3 vec_front;
	glm::vec3 vec_left;
	glm::vec3 vec_up;

	Camera( );
};

class DisplayMgr : 
	public Manager {

private:
	HDC			hDC;
	HGLRC		hRC;
	HWND		hWnd;
	HINSTANCE	hInstance;

	glm::ivec2 dim_window;

	void init_gl_window( 
		glm::ivec2 & pos_window, glm::ivec2 & dim_window, 
		int const num_bits, bool is_fullscreen );

	void init_gl();

public:
	int freq_display;
	float fov = 90;

	Camera camera;
	BlockSelector block_selector;

	std::ostringstream out;

	DisplayMgr( Client & client );
	~DisplayMgr();

	void init( );
	void update( );
	void render( ) {}
	void end( ) {}
	void sec( ) {}

	HDC & get_HDC( );
	HGLRC & get_HRC( );
	HWND & get_HWND();

	glm::ivec2 & get_window();
	void resize_window( glm::ivec2 & dim_window );

	void draw_string( 
		glm::ivec2 const & pos_quad, std::string const & string, 
		Color4 & color, int const size );
	void draw_key( int const size );
	void draw_quad( glm::ivec2 & pos_quad, glm::ivec2 & dim_quad, Color4 const & color, face_uvs & uvs );
	void draw_block( Color4 &, int const id_block );
	void draw_skybox( glm::vec3 & pos_skybox, float const size );
	void draw_sun( glm::vec3 & pos_sun, float const size );
	void draw_chunk( Chunk & chunk );
	void draw_record_graph( 
		glm::ivec2 & pos_graph, glm::ivec2 & dim_graph, 
		std::string const & name_record, float time_ref );
	void draw_record_graph( 
		glm::ivec2 & pos_graph, glm::ivec2 & dim_graph, 
		std::string const & name_record, float tim_ref, int size_history );

	void clear_buffers();
	void set_camera();
	void swap_buffers();

	void set_ortho();
	void set_proj();
};