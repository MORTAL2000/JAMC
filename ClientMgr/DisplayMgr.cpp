#include "DisplayMgr.h"
#include "Client.h"

#include "ChunkMgr.h"
#include "Color4.h"

#include <iostream>

Camera::Camera( ) :
	pos_camera( 0, Chunk::size_y * 2 / 3, 0 ),
	rot_camera( 0.0f, 180.0f, 0.0f ) { }

DisplayMgr::DisplayMgr( Client & client ) :
	block_selector( client ),
	Manager( client ) { }

DisplayMgr::~DisplayMgr( ) { }

void DisplayMgr::init( ) {
	printTabbedLine( 0, "Init DisplayMgr..." );

	init_gl_window( 
		glm::ivec2( 0, 0 ), 
		glm::ivec2( 1900, 1000 ), 
		32,
		false );

	init_gl( );
	set_proj( );

	freq_display = get_refresh( );

	out.str( "" );
	out << "Display freq: " << freq_display;
	printTabbedLine( 1, out );

	printTabbedLine( 1, ( char* ) glGetString( GL_VERSION ) );
	printTabbedLine( 1, checkGlErrors( ) );

	camera.mvp_matrices.mat_world = glm::mat4( 1.0f );
	camera.mvp_matrices.mat_view = glm::mat4( 1.0f );

	camera.mat_translation = glm::translate( glm::mat4( 1.0f ), -camera.pos_camera );
	camera.mat_rotation = glm::mat4( 1.0f );
	camera.mat_rotation = glm::rotate( glm::mat4( 1.0f ), glm::radians( 180.0f ), glm::vec3( 0, 1, 0 ) );

	block_selector.init( );

	printTabbedLine( 0, "...Init DisplayMgr" );
	std::cout << std::endl;
}

float mouse_sensitivity = 0.1f;

void DisplayMgr::update( ) {
	if( !client.input_mgr.is_cursor_vis( ) ) {
		camera.rot_camera.x -= client.input_mgr.get_mouse_delta( ).y * mouse_sensitivity;
		camera.rot_camera.y += client.input_mgr.get_mouse_delta( ).x * mouse_sensitivity;

		while( camera.rot_camera.x >= 360 ) camera.rot_camera.x -= 360;
		while( camera.rot_camera.x < 0 ) camera.rot_camera.x += 360;
		if( camera.rot_camera.x > 180 && camera.rot_camera.x < 270 ) camera.rot_camera.x = 270;
		if( camera.rot_camera.x < 180 && camera.rot_camera.x > 90 ) camera.rot_camera.x = 90;

		camera.mat_rotation = glm::rotate( glm::mat4( 1.0f ), glm::radians( camera.rot_camera.x ), glm::vec3( 1, 0, 0 ) );
		camera.mat_rotation = glm::rotate( camera.mat_rotation, glm::radians( camera.rot_camera.y ), glm::vec3( 0, 1, 0 ) );
	}

	camera.mat_translation = glm::translate( glm::mat4( 1.0f ), -camera.pos_camera );
	camera.mvp_matrices.mat_view = camera.mat_rotation * camera.mat_translation;
	camera.mvp_matrices.pos_camera = glm::vec4( camera.pos_camera, 1.0f );

	camera.vec_front = Directional::get_fwd( camera.rot_camera );
	camera.vec_left = Directional::get_left( camera.rot_camera );
	camera.vec_up = Directional::get_up_aa( camera.rot_camera );

	block_selector.update( );
}

HDC & DisplayMgr::get_HDC( ) { 
	return hDC;
}

HGLRC & DisplayMgr::get_HRC( ) { 
	return hRC;
}

HWND & DisplayMgr::get_HWND( ) { 
	return hWnd;
}

glm::ivec2 & DisplayMgr::get_window( ) {
	return  dim_window;
}

void DisplayMgr::init_gl_window( glm::ivec2 & pos_window, glm::ivec2 & dim_window, int const num_bits, bool is_fullscreen ) {
	printTabbedLine( 1, "Init GLWindow..." );
	
	// Bind process affinity
	HANDLE process = GetCurrentProcess( );

	DWORD_PTR processAffinityMask;
	DWORD_PTR systemAffinityMask;

	if( !GetProcessAffinityMask( process, &processAffinityMask, &systemAffinityMask ) ) {
		std::cout << "Error getting affinity." << std::endl;
	}

	if( SetProcessAffinityMask( process, systemAffinityMask ) != 1 ) {
		std::cout << "Error setting affinity." << std::endl;
	}

	HANDLE threadHandle = GetCurrentThread( );

	DWORD_PTR threadAffinityMask = 3;

	if( !SetThreadAffinityMask( threadHandle, threadAffinityMask ) ) {
		std::cout << "Error setting thread affinity1." << std::endl;
	}

	SetThreadPriority( threadHandle, THREAD_PRIORITY_ABOVE_NORMAL );
	
	GLuint		PixelFormat;
	WNDCLASS	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		WindowRect;

	WindowRect.left = ( long ) 0;
	WindowRect.right = ( long ) dim_window.x;
	WindowRect.top = ( long ) 0;
	WindowRect.bottom = ( long ) dim_window.y;

	hInstance = GetModuleHandle( NULL );			// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = ( WNDPROC ) WndProc;			// WndProc Handles Messages
	wc.cbClsExtra = 0;								// No Extra Window Data
	wc.cbWndExtra = 0;								// No Extra Window Data
	wc.hInstance = hInstance;						// Set The Instance
	wc.hIcon = LoadIcon( NULL, IDI_WINLOGO );		// Load The Default Icon
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );		// Load The Arrow Pointer
	wc.hbrBackground = NULL;						// No Background Required For GL
	wc.lpszMenuName = NULL;							// We Don't Want A Page
	wc.lpszClassName = L"DisplayManager";			// Set The Class Name

	if( !RegisterClass( &wc ) ) {
		MessageBox( NULL, L"Failed To Register The Window Class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION );
	}

	if( is_fullscreen ) {
		DEVMODE dmScreenSettings;								// Device Mode
		memset( &dmScreenSettings, 0, sizeof( dmScreenSettings ) );	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof( dmScreenSettings );		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = dim_window.x;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = dim_window.y;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = num_bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if( ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL ) {
			MessageBox( NULL, L"Failed Fullscreen", L"ERROR", MB_OK | MB_ICONSTOP );
		}
	}

	if( is_fullscreen ) {
		dwExStyle = WS_EX_APPWINDOW;				// Window Extended Style
		dwStyle = WS_POPUP;
	}
	else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(								// Adjust Window To True Requested Size
		&WindowRect, dwStyle, 
		FALSE, dwExStyle );	

	// Create The Window
	if( !( hWnd = CreateWindowEx(
		dwExStyle,									// Extended Style For The Window
		L"DisplayManager",							// Class Name
		L"Minceclone",								// Window Title
		dwStyle |									// Defined Window Style
		WS_CLIPSIBLINGS |							// Required Window Style
		WS_CLIPCHILDREN,							// Required Window Style
		pos_window.x, pos_window.y,					// Window page.state.position
		WindowRect.right - WindowRect.left,			// Calculate Window Width
		WindowRect.bottom - WindowRect.top,			// Calculate Window Height
		NULL,										// No Parent Window
		NULL,										// No Page
		hInstance,									// Instance
		NULL ) ) ) {								// Dont Pass Anything To WM_CREATE
		MessageBox( NULL, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION );
	}

	static	PIXELFORMATDESCRIPTOR pfd = {			// pfd Tells Windows How We Want Things To Be
		sizeof( PIXELFORMATDESCRIPTOR ),			// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		num_bits,									// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		32,											// 32Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if( !( hDC = GetDC( hWnd ) ) ) { MessageBox( NULL, L"Can't Create A GL Device Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION ); }									// Did We Get A Device Context?
	if( !( PixelFormat = ChoosePixelFormat( hDC, &pfd ) ) ) { MessageBox( NULL, L"Can't Find A Suitable PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION ); }		// Did Windows Find A Matching Pixel Format?
	if( !SetPixelFormat( hDC, PixelFormat, &pfd ) ) { MessageBox( NULL, L"Can't Set The PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION ); }						// Are We Able To Set The Pixel Format?
	if( !( hRC = wglCreateContext( hDC ) ) ) { MessageBox( NULL, L"Can't Create A GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION ); }						// Are We Able To Get A Rendering Context?
	if( !wglMakeCurrent( hDC, hRC ) ) { MessageBox( NULL, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION ); }						// Try To Activate The Rendering Context

	ShowWindow( hWnd, SW_SHOW );					// Show The Window
	ShowWindow( hWnd, SW_MAXIMIZE );

	RECT rect_win;
	GetClientRect( hWnd, &rect_win );
	dim_window.x = rect_win.right - rect_win.left;
	dim_window.y = rect_win.bottom - rect_win.top;

	SetForegroundWindow( hWnd );					// Slightly Higher Priority
	SetFocus( hWnd );								// Sets Keyboard Focus To The Window
	resize_window( dim_window );					// Set Up Our Perspective GL Screen

	printTabbedLine( 1, "...Init GLWindow" );
}

void DisplayMgr::init_gl( ) {
	printTabbedLine( 1, "Init GL..." );

	// set vsync
	typedef BOOL( APIENTRY* PFNWGLSWAPINTERVALPROC )( int );
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = ( char* ) glGetString( GL_EXTENSIONS );

	if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 ) {
		return;
	}
	else {
		wglSwapIntervalEXT = ( PFNWGLSWAPINTERVALPROC ) wglGetProcAddress( "wglSwapIntervalEXT" );
		if( wglSwapIntervalEXT ) wglSwapIntervalEXT( 0 );
	}

	printTabbedLine( 2, "Init Glew..." );
	glewInit( );
	printTabbedLine( 2, "...Init Glew" );

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//( GL_CULL_FACE );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClearDepth( 3000.0f );

	//glEnable( GL_CULL_FACE );
	//glCullFace( GL_BACK );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDepthFunc( GL_LEQUAL );

	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	glShadeModel( GL_SMOOTH );

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glEnable( GL_COLOR_MATERIAL );

	glEnable( GL_LIGHTING );
	//glEnable( GL_LIGHT1 );

	printTabbedLine( 1, "...Init GL" );
}

void DisplayMgr::resize_window( glm::ivec2 & dim_window ) {
	if( dim_window.y == 0 ) {
		dim_window.y = 1;
	}

	this->dim_window = dim_window;
	glViewport( 0, 0, dim_window.x, dim_window.y );
	camera.mvp_matrices.mat_ortho = glm::ortho( 0.0f, ( float ) dim_window.x, 0.0f, ( float ) dim_window.y, 0.0f, 1000.0f );
	set_proj( );
}

void DisplayMgr::draw_string( glm::ivec2 const & pos, std::string const & string, Color4 & color, int const size ) {
	glPushMatrix( );

	glLineWidth( size / 10.0f );
	glTranslatef( pos.x, pos.y - size / 2, 0 );
	glColor4f( color.r, color.g, color.b, color.a );
	glScalef( size / 104.76f, size / 119.05f, 1.0f );
	for( auto iter = string.begin( ); iter != string.end( ); iter++ ) {
		if( *iter != ' ' )
			glTranslatef( size, 0.0f, 0.0f );

		glutStrokeCharacter( GLUT_STROKE_ROMAN, *iter );
	}

	glPopMatrix( );
}

/*
void DisplayMgr::draw_string( Vect3< int > const & pos, std::string & string, Color4 & color, int const size ) {
	glPushMatrix();
	glLineWidth( size / 10.0f );
	glTranslatef( pos.x, pos.y, pos.z );
	glColor4f( color.r, color.g, color.b, color.a );
	glScalef( size / 104.76f, size / 119.05f, 1.0f );
	for( auto iter = string.begin(); iter != string.end(); iter++ ) {
		/*if( *iter == ' ' || *iter == '_' || *iter == '>' || *iter == '<' ) {
		glScalef( 0.25f, 1.0f, 1.0f );
		glutStrokeCharacter( GLUT_STROKE_ROMAN, *iter );
		glScalef( 4.0f, 1.0f, 1.0f );
		glTranslatef( 6.0f, 0.0f, 0.0f );
		}
		else {
		glutStrokeCharacter( GLUT_STROKE_ROMAN, *iter );
		glTranslatef( 6.0f, 0.0f, 0.0f );
		}

		glutStrokeCharacter( GLUT_STROKE_ROMAN, *iter );
		//glTranslatef( 6.0f, 0.0f, 0.0f );
	}
	glPopMatrix();
}
*/

void DisplayMgr::draw_key( int const size ) {
	glPushMatrix( );

	glTranslatef( dim_window.x / 2, dim_window.y / 2, -size );

	glRotatef( camera.rot_camera.x, 1, 0, 0 );
	glRotatef( camera.rot_camera.y, 0, 1, 0 );
	glRotatef( camera.rot_camera.z, 0, 0, 1 );

	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	glVertex3f( 0, 0, 0 );
	glVertex3f( size, 0, 0 );

	glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, size, 0 );

	glColor4f( 0.0f, 0.0f, 1.0f, 1.0f );
	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, 0, size );

	glEnd();

	glPopMatrix( );
}

void DisplayMgr::draw_quad( glm::ivec2 & pos_quad, glm::ivec2 & dim_quad, Color4 const & color, face_uvs & uvs ) { 
	glPushMatrix( );
	glBegin( GL_QUADS );
	glColor4f( color.r, color.g, color.b, color.a );
	glTexCoord2f( uvs[ 0 ][ 0 ], uvs[ 0 ][ 1 ] );
	glVertex2f( pos_quad.x, pos_quad.y );
	glTexCoord2f( uvs[ 1 ][ 0 ], uvs[ 1 ][ 1 ] );
	glVertex2f( pos_quad.x + dim_quad.x, pos_quad.y );
	glTexCoord2f( uvs[ 2 ][ 0 ], uvs[ 2 ][ 1 ] );
	glVertex2f( pos_quad.x + dim_quad.x, pos_quad.y + dim_quad.y );
	glTexCoord2f( uvs[ 3 ][ 0 ], uvs[ 3 ][ 1 ] );
	glVertex2f( pos_quad.x, pos_quad.y + dim_quad.y );
	glEnd( );
	glPopMatrix( );
}

void DisplayMgr::draw_block( Color4 & color, int const id_block ) {
	glBegin( GL_QUADS );
	glColor4f( color.r, color.g, color.b, color.a );
	auto & block = client.chunk_mgr.get_block_data( id_block );
	for( int i = 0; i < FD_Size; i++ ) { 
		auto block_face = ( FaceDirection ) i;
		auto & block_face_uvs = block.get_uvs( block_face );
		auto & block_norm = Directional::get_vec_dir_f( block_face );
		auto & block_face_verts = Block::get_verts( block_face );
		for( int j = 0; j < 4; j++ ) { 
			glNormal3f( block_norm.x, block_norm.y, block_norm.z );
			glTexCoord2f( block_face_uvs[ j ][ 0 ], block_face_uvs[ j ][ 1 ] );
			glVertex3f( block_face_verts[ j ][ 0 ], block_face_verts[ j ][ 1 ], block_face_verts[ j ][ 2 ] );
		}
	}
	glEnd( );
}

void DisplayMgr::draw_skybox( glm::vec3 & pos_skybox, float const size ) {
	client.texture_mgr.bind_skybox( );

	glPushMatrix( );

	glDisable( GL_CULL_FACE );
	glDisable( GL_LIGHT0 );

	glTranslatef( pos_skybox.x, pos_skybox.y, pos_skybox.z );
	glRotatef( client.time_mgr.get_time( TimeStrings::GAME ) / TIME_MILLISEC, 0, 1, 0 );
	glTranslatef( -size / 2, -size / 2, -size / 2 );

	glBegin( GL_QUADS );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	for( int i = 0; i < FD_Size; i++ ) {
		auto face = ( FaceDirection ) i;
		auto & face_uvs = client.texture_mgr.get_uvs_skybox( face );
		auto & face_verts = Block::get_verts( face );
		for( int j = 0; j < 4; j++ ) {
			glTexCoord2f( face_uvs[ j ][ 0 ], face_uvs[ j ][ 1 ] );
			glVertex3f( face_verts[ j ][ 0 ] * size, face_verts[ j ][ 1 ] * size, face_verts[ j ][ 2 ] * size );
		}
	}
	glEnd( );

	glEnable( GL_LIGHT0 );
	glEnable( GL_CULL_FACE );

	glPopMatrix( );
}

void DisplayMgr::draw_sun( glm::vec3 & pos_sun, float const size ) {
	glm::vec3 delta_sun = camera.pos_camera - pos_sun;

	glm::vec3 rot_sun( 
		atan( -delta_sun.y / sqrt( pow( delta_sun.x, 2 ) + pow( delta_sun.z, 2 ) ) ) * 180 / PI,
		atan2( delta_sun.x, delta_sun.z ) * 180 / PI,
		0 );

	auto & sun_uvs = client.texture_mgr.get_uvs_sun( );
	auto & sun_verts = Block::get_verts( FD_Front );

	glPushMatrix( );

	glDisable( GL_LIGHTING );

	glTranslatef( pos_sun.x, pos_sun.y, pos_sun.z );
	glRotatef( rot_sun.y, 0, 1, 0 );
	glRotatef( rot_sun.x, 1, 0, 0 );
	glTranslatef( -size / 2, -size / 2, -size / 2 );
	glColor4f( 0.8f, 0.8f, 0.8f, 1.0f );
	glBegin( GL_QUADS );
	for( int m = 0; m < 4; m++ ) {
		glTexCoord2f(
			sun_uvs[ m ][ 0 ],
			sun_uvs[ m ][ 1 ] );

		glVertex3f(
			sun_verts[ m ][ 0 ] * size,
			sun_verts[ m ][ 1 ] * size,
			sun_verts[ m ][ 2 ] * size );
	}
	glEnd( );

	glEnable( GL_LIGHTING );

	glPopMatrix( );
}

void DisplayMgr::draw_chunk( Chunk & chunk ) {
	glPushMatrix( );

	glTranslatef( 
		chunk.pos_gw.x, 
		chunk.pos_gw.y,
		chunk.pos_gw.z );

	glBegin( GL_QUADS );
	for( int i = 0; i < Chunk::size_x; i++ ) {
		for( int j = 0; j < Chunk::size_y; j++ ) {
			for( int k = 0; k < Chunk::size_z; k++ ) {
				int id_block = chunk.id_blocks[ i ][ j ][ k ];

				if( id_block != -1 ) {
					auto & block = client.chunk_mgr.get_block_data( id_block );

					glColor4f( 
						block.color.r, 
						block.color.g,
						block.color.b,
						block.color.a );

					for( int l = 0; l < FD_Size; l++ ) {
						auto block_face = ( FaceDirection ) l;
						auto & block_face_uvs = block.get_uvs( block_face );
						auto & block_face_verts = Block::get_verts( block_face );

						for( int m = 0; m < 4; m++ ) {
							glTexCoord2f( 
								block_face_uvs[ m ][ 0 ], 
								block_face_uvs[ m ][ 1 ] );

							glVertex3f(
								i + block_face_verts[ m ][ 0 ], 
								j + block_face_verts[ m ][ 1 ], 
								k + block_face_verts[ m ][ 2 ] );
						}
					}
				}
			}
		}
	}
	glEnd( );

	glPopMatrix( );
}

static int const size_graph_text = 12;

void DisplayMgr::draw_record_graph( glm::ivec2 & pos_graph, glm::ivec2 & dim_graph, std::string const & name_record, float time_ref, int size_history ) { 
	auto & record_graph = client.time_mgr.get_record( name_record );
	if( size_history > record_graph.history.size( ) || size_history <= 0 ) { 
		size_history = record_graph.history.size( );
	}
	if( size_history > 0 ) {
		float size_bar = dim_graph.x / float( size_history );

		float max_record = time_ref;

		for( int i = 0; i < size_history; i++ ) {
			if( record_graph.history[ i ] > max_record ) {
				max_record = record_graph.history[ i ];
			}
		}
		glPushMatrix( );

		glTranslatef( pos_graph.x, pos_graph.y, 0 );

		glBegin( GL_QUADS );

		glColor4f( 0.5f, 0.5f, 0.5f, 0.7f );

		glVertex2f( -1, -1 );
		glVertex2f( dim_graph.x + 1, -1 );
		glVertex2f( dim_graph.x + 1, dim_graph.y + 1 );
		glVertex2f( -1, dim_graph.y + 1 );

		glColor4f( 1.0f, 0.0f, 0.0f, 0.5f );

		for( int i = 0; i < size_history; i++ ) {
			glVertex2f( i * size_bar, 0 );
			glVertex2f( i * size_bar + size_bar, 0 );
			glVertex2f( i * size_bar + size_bar, dim_graph.y * ( record_graph.history[ i ] / max_record ) );
			glVertex2f( i * size_bar, dim_graph.y * ( record_graph.history[ i ] / max_record ) );
		}

		glColor4f( 0.0f, 1.0f, 0.0f, 0.5f );

		glVertex2f( 0, dim_graph.y * ( time_ref / max_record ) - 1 );
		glVertex2f( dim_graph.x, dim_graph.y * ( time_ref / max_record ) - 1 );
		glVertex2f( dim_graph.x, dim_graph.y * ( time_ref / max_record ) );
		glVertex2f( 0, dim_graph.y * ( time_ref / max_record ) );

		glEnd( );

		glm::ivec2 pos_string = pos_graph;
		pos_string.y += dim_graph.y - 1 - size_graph_text / 2;
		pos_string.x += 1;

		glPopMatrix( );

		draw_string( pos_string, name_record, Color4( 0.0f, 0.0f, 1.0f, 0.5f ), size_graph_text );
	}
}

void DisplayMgr::draw_record_graph( glm::ivec2 & pos_graph, glm::ivec2 & dim_graph, std::string const & name_record, float time_ref ) {
	draw_record_graph( pos_graph, dim_graph, name_record, time_ref, 0 );
}

void DisplayMgr::clear_buffers( ) {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity( );
	glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
}

void DisplayMgr::set_camera( ) {
	glRotatef( camera.rot_camera.x, 1, 0, 0 );
	glRotatef( camera.rot_camera.y, 0, 1, 0 );
	glRotatef( camera.rot_camera.z, 0, 0, 1 );
	glTranslatef( -camera.pos_camera.x, -camera.pos_camera.y, -camera.pos_camera.z );
}

void DisplayMgr::swap_buffers( ) {
	SwapBuffers( hDC );
}

void DisplayMgr::set_ortho( ) {
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );

	glOrtho( 0, dim_window.x, 0, dim_window.y, 0, 1000 );
	camera.mvp_matrices.mat_ortho = glm::ortho( 0.0f, ( float ) dim_window.x, 0.0f, ( float  ) dim_window.y, 0.0f, 1000.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
}

void DisplayMgr::set_proj( ) {
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );

	gluPerspective( 35.0f, float( dim_window.x ) / dim_window.y, 0.2f, 3000.0f );
	camera.mvp_matrices.mat_perspective = glm::perspective( glm::radians( 35.0f ), float( dim_window.x ) / dim_window.y, 0.2f, 3000.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
}