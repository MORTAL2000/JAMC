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
	list_render.push_back( &RecordStrings::FRAME );
	list_render.push_back( &RecordStrings::UPDATE );
	list_render.push_back( &RecordStrings::TASK_MAIN );
	list_render.push_back( &RecordStrings::UPDATE_MAP );
	list_render.push_back( &RecordStrings::UPDATE_TIME );
	list_render.push_back( &RecordStrings::UPDATE_GUI );
	list_render.push_back( &RecordStrings::RENDER );
	list_render.push_back( &RecordStrings::RENDER_DRAW );
	list_render.push_back( &RecordStrings::RENDER_SWAP );
	list_render.push_back( &RecordStrings::SLEEP );

	vbo.init( );
}

// Manager Methods
void TimeMgr::update() {
	static unsigned int cnt = 0;
	if( cnt++ % 2 == 0 ) is_dirty = true;
	mesh_graphs( );
}

void TimeMgr::render() {
	client.texture_mgr.bind_program( "Basic" );
	static GLuint idx_model = glGetUniformLocation( client.texture_mgr.id_prog, "mat_model" );

	glUniformMatrix4fv( idx_model, 1, GL_FALSE, 
		glm::value_ptr( 
			glm::translate( glm::mat4( 1.0f ), 
				glm::vec3( 
					client.display_mgr.get_window().x - dim_graph.x - padding, 
					client.display_mgr.get_window( ).y, 
					0 
	) ) ) );

	vbo.render( client );
}

void TimeMgr::end() {

}

void TimeMgr::sec() {
}

int size_history_max = 288;
glm::ivec2 TimeMgr::dim_graph( 144 * 2, 30 );

void TimeMgr::mesh_graphs( ) {
	if( is_dirty ) { 
		vbo.clear( );

		int unsigned size_history;
		float dx = float( dim_graph.x ) / size_history_max;
		float max_record = 0;
		glm::ivec2 dim_char = { 8, 12 };

		auto & norm = Directional::get_vec_dir_f( FaceDirection::FD_Front );
		auto color = glm::vec4( 0.5f, 0.5f, 0.5f, 0.7f );

		vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
			"Basic", client.texture_mgr.id_materials,
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		// Background
		for( int i = 0; i < list_render.size( ); ++i ) { 
			for( int j = 0; j < 4; ++j ) { 
				vbo.push_data( VBO::Vertex {
					{ verts_graph[ j ].x * dim_graph.x, 
					( -( i + 1 ) * ( dim_graph.y + padding ) ) + verts_graph[ j ].y * dim_graph.y, 0 },
					{ color.r, color.g, color.b, color.a },
					{ norm.x, norm.y, norm.z },
					{ 0, 0, 0 }
				} );
			}
		}

		color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );

		vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Lines,
			"Basic", client.texture_mgr.id_materials,
			std::vector< GLuint >{ 0, 1 }
		) );

		// Line Graph
		for( int i = 0; i < list_render.size( ); ++i ) {
			auto & record = get_record( *list_render[ i ] );
			
			size_history = record.history.size( );

			if( size_history == 0 ) { 
				continue;
			}
			else if( size_history > size_history_max ) {
				size_history = size_history_max;
			}

			max_record = TIME_FRAME_MILLI;
			for( int j = 0; j < size_history; ++j ) {
				if( max_record < record.history[ j ] ) { 
					max_record = record.history[ j ];
				}
			}

			for( int j = 0; j < size_history - 1; ++j ) {
				vbo.push_data( VBO::Vertex { 
					{ j * dx, 
					( -( i + 1 ) * ( dim_graph.y + padding ) ) + ( record.history[ j ] / max_record ) * dim_graph.y, 0 },
					{ color.r, color.g, color.b, color.a },
					{ norm.x, norm.y, norm.z },
					{ 0, 0, 0 }
				} );

				vbo.push_data( VBO::Vertex {
					{ ( j + 1 ) * dx, 
					( -( i + 1 ) * ( dim_graph.y + padding ) ) + ( record.history[ j + 1 ] / max_record ) * dim_graph.y, 0 },
					{ color.r, color.g, color.b, color.a },
					{ norm.x, norm.y, norm.z },
					{ 0, 0, 0 }
				} );
			}

			vbo.push_data( VBO::Vertex {
				{ 0,
				( -( i + 1 ) * ( dim_graph.y + padding ) ) + ( TIME_FRAME_MILLI / max_record ) * dim_graph.y, 0 },
				{ 0.0f, 1.0f, 0.0f, 1.0f },
				{ norm.x, norm.y, norm.z },
				{ 0, 0, 0 }
			} );

			vbo.push_data( VBO::Vertex {
				{ float( dim_graph.x ),
				( -( i + 1 ) * ( dim_graph.y + padding ) ) + ( TIME_FRAME_MILLI / max_record ) * dim_graph.y, 0 },
				{ 0.0f, 1.0f, 0.0f, 1.0f },
				{ norm.x, norm.y, norm.z },
				{ 0, 0, 0 }
			} );
		}

		color = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );

		vbo.push_set( VBO::IndexSet( VBO::TypeGeometry::TG_Triangles,
			"Basic", client.texture_mgr.id_fonts,
			std::vector< GLuint >{ 0, 1, 2, 2, 3, 0 }
		) );

		// Text
		for( int i = 0; i < list_render.size( ); ++i ) {
			auto & name = *list_render[ i ];

			for( int j = 0; j < name.size( ); j++ ) {
				auto & uvs = client.texture_mgr.get_uvs_fonts( name[ j ] - 32 );

				for( int k = 0; k < 4; k++ ) {
					vbo.push_data( {
						{ padding + j * dim_char.x + verts_graph[ k ].x * dim_char.x,
						( -( i + 1 ) * ( dim_graph.y + padding ) ) + padding + verts_graph[ k ].y * dim_char.y, 0 },
						{ color.r, color.g, color.b, color.a },
						{ norm.x, norm.y, norm.z },
						{ uvs[ k ][ 0 ], uvs[ k ][ 1 ], 0 }
					} );
				}
			}
		}

		vbo.finalize_set( );

		client.thread_mgr.task_main( 5, [ & ] ( ) {
			vbo.buffer( );
		} );

		is_dirty = false;
	}
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