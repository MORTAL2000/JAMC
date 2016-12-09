#include "DisplayMgr.h"

#include <iostream>

#include "ChunkMgr.h"

Camera::Camera( ) :
	pos_camera( 0, WorldSize::Chunk::size_y * 2 / 3, 0 ),
	rot_camera( 0.0f, 180.0f, 0.0f ) { }

DisplayMgr::DisplayMgr( ) :
	Manager( ),
	is_vsync( false ),
	is_limiter( true ) { }

DisplayMgr::~DisplayMgr( ) { }

void DisplayMgr::init( ) {
	printTabbedLine( 0, "Init DisplayMgr..." );

	init_gl_window( 
		glm::ivec2( 0, 0 ), 
		glm::ivec2( 1920, 1080 ), 
		32, false );

	init_gl( );
	set_proj( );

	freq_display = get_refresh( );

	out.str( "" );
	out << "Display freq: " << freq_display;
	printTabbedLine( 1, out );

	printTabbedLine( 1, ( char* ) glGetString( GL_VERSION ) );

	camera.mvp_matrices.mat_world = glm::mat4( 1.0f );
	camera.mvp_matrices.mat_view = glm::mat4( 1.0f );

	camera.mat_translation = glm::translate( glm::mat4( 1.0f ), -camera.pos_camera );
	camera.mat_rotation = glm::mat4( 1.0f );
	camera.mat_rotation = glm::rotate( glm::mat4( 1.0f ), glm::radians( 180.0f ), glm::vec3( 0, 1, 0 ) );

	printTabbedLine( 0, "...Init DisplayMgr" );
	std::cout << std::endl;
}

void DisplayMgr::update( ) {
	auto & p_ec_state = client.entity_mgr.entity_player->h_state.get( );

	camera.pos_camera = p_ec_state.pos;
	camera.pos_camera.y += p_ec_state.dim.y / 2.0f;
	camera.rot_camera = p_ec_state.rot;

	camera.mat_rotation = glm::rotate( glm::mat4( 1.0f ), glm::radians( camera.rot_camera.x ), glm::vec3( 1, 0, 0 ) );
	camera.mat_rotation = glm::rotate( camera.mat_rotation, glm::radians( camera.rot_camera.y ), glm::vec3( 0, 1, 0 ) );

	camera.mat_translation = glm::translate( glm::mat4( 1.0f ), -camera.pos_camera );

	camera.mvp_matrices.mat_view = camera.mat_rotation * camera.mat_translation;
	camera.mvp_matrices.pos_camera = glm::vec4( camera.pos_camera, 1.0f );

	camera.vec_front = Directional::get_fwd( camera.rot_camera );
	camera.vec_left = Directional::get_left( camera.rot_camera );
	camera.vec_up = Directional::get_up_aa( camera.rot_camera );

	out.str( "" );
	out << "Camera pos: " << Directional::print_vec( camera.pos_camera );
	client.gui_mgr.print_to_static( out.str( ) );
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
		L"[ JAMC ]",								// Window Title
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
	set_vsync( is_vsync );

	printTabbedLine( 2, "Init Glew..." );
	glewInit( );
	printTabbedLine( 2, "...Init Glew" );

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClearDepth( 3000.0f );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDepthFunc( GL_LEQUAL );

	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	glShadeModel( GL_SMOOTH );

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

typedef BOOL( APIENTRY* PFNWGLSWAPINTERVALPROC )( int );

PFNWGLSWAPINTERVALPROC get_vsync( ) {
	const char *extensions = ( char* ) glGetString( GL_EXTENSIONS );

	if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 ) {
		return nullptr;
	}
	else {
		return ( PFNWGLSWAPINTERVALPROC ) wglGetProcAddress( "wglSwapIntervalEXT" );
	}
}

void DisplayMgr::set_vsync( bool is_vsync ) { 
	static PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = get_vsync( );

	this->is_vsync = is_vsync;
	if( wglSwapIntervalEXT ) {
		wglSwapIntervalEXT( is_vsync );
	}
}

void DisplayMgr::toggle_vsync( ) { 
	is_vsync = !is_vsync;
	set_vsync( is_vsync );
}

void DisplayMgr::toggle_limiter( ) { 
	is_limiter = !is_limiter;
}

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

void DisplayMgr::clear_buffers( ) {
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
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

	float aspect = float( dim_window.x ) / float( dim_window.y );
	//float fov_y = fov / aspect;

	gluPerspective( fov, aspect, 0.1f, 3000.0f );

	camera.mvp_matrices.mat_perspective = glm::perspectiveFov( 
		glm::radians( fov ), ( float ) dim_window.x, ( float ) dim_window.y, 0.1f, 3000.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
}