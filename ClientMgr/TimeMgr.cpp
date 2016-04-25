#include "TimeMgr.h"

#include "Client.h"

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

glm::ivec2 TimeMgr::dim_graph( 72 * 5, 30 );

// *** Time Manager ***
TimeMgr::TimeMgr( Client & client ) :
	Manager( client ),
	time_map( time_size ),
	record_map( record_size ) {}

TimeMgr::~TimeMgr() {}

void TimeMgr::init() {
	glGenBuffers( 1, &id_vbo );

	list_render.push_back( &RecordStrings::FRAME );
	list_render.push_back( &RecordStrings::UPDATE );
	//list_render.push_back( &RecordStrings::UPDATE_MAP );
	//list_render.push_back( &RecordStrings::UPDATE_QUEUE );
	//list_render.push_back( &RecordStrings::CHUNK_BUFFER );
	list_render.push_back( &RecordStrings::RENDER );
	list_render.push_back( &RecordStrings::RENDER_DRAW );
	list_render.push_back( &RecordStrings::RENDER_SWAP );
	list_render.push_back( &RecordStrings::SLEEP );
}

// Manager Methods
void TimeMgr::update() {
	is_dirty = true;
	mesh_graphs( );
}

void TimeMgr::render() {
	client.texture_mgr.bind_materials( );

	glPushMatrix( );

	glTranslatef( client.display_mgr.get_window().x - dim_graph.x - padding, client.display_mgr.get_window().y - dim_graph.y - padding, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
	glVertexPointer( 3, GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 0 ) );
	glColorPointer( 4, GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 12 ) );
	glNormalPointer( GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 28 ) );
	glTexCoordPointer( 2, GL_FLOAT, sizeof( ChunkVert ), BUFFER_OFFSET( 40 ) );

	glDrawArrays( GL_QUADS, 0, index_graph );
	
	if( index_text > index_graph ) {
		client.texture_mgr.bind_fonts( );
		glDrawArrays( GL_QUADS, index_graph, index_text - index_graph );
	}

	glPopMatrix( );
}

void TimeMgr::end() {

}

void TimeMgr::sec() {
}

void TimeMgr::mesh_graphs( ) { 
	if( is_dirty ) {
		mesh_buffer.clear( );

		int size_history;
		float size_bar;
		float max_record;
		Color4 color;
		auto & verts = Block::get_verts( FD_Front );
		auto & norm = Directional::get_vec_dir_f( FD_Front );

		for( int i = 0; i < list_render.size( ); i++ ) {
			auto & record = get_record( *list_render[ i ] );
			size_history = record.history.size( );

			if( size_history > 0 ) { 
				auto & uvs = client.texture_mgr.get_uvs_materials( );
				size_bar = dim_graph.x / float( size_history );
				max_record = TIME_FRAME_MILLI;

				for( int j = 0; j < size_history; j++ ) {
					if( record.history[ j ] > max_record ) {
						max_record = record.history[ j ];
					}
				}

				color = Color4( 0.5f, 0.5f, 0.5f, 0.7f );

				for( int j = 0; j < 4; j++ ) {
					mesh_buffer.push_back( {
						{ verts[ j ][ 0 ] * dim_graph.x, ( -i * ( dim_graph.y + padding ) ) + verts[ j ][ 1 ] * dim_graph.y, 0 },
						{ color.r, color.g, color.b, color.a },
						{ norm.x, norm.y, norm.z },
						{ uvs[ j ][ 0 ], uvs[ j ][ 1 ] },
						{ 0, 0, 0, 0 }
					} );
				}

				color = Color4( 1.0f, 0.0f, 0.0f, 0.5f );

				for( int j = 0; j < size_history; j++ ) { 
					for( int k = 0; k < 4; k++ ) { 
						mesh_buffer.push_back( {
							{ j * size_bar + verts[ k ][ 0 ] * size_bar, ( -i * ( dim_graph.y + padding ) ) + verts[ k ][ 1 ] * dim_graph.y * ( record.history[ j ] / max_record ), 0 },
							{ color.r, color.g, color.b, color.a },
							{ norm.x, norm.y, norm.z },
							{ uvs[ k ][ 0 ], uvs[ k ][ 1 ] },
							{ 0, 0, 0, 0 }
						} );
					}
				}

				color = Color4( 0.0f, 1.0f, 0.0f, 0.5f );

				for( int j = 0; j < 4; j++ ) {
					mesh_buffer.push_back( {
						{ verts[ j ][ 0 ] * dim_graph.x, ( -i * ( dim_graph.y + padding ) ) + dim_graph.y * ( TIME_FRAME_MILLI / max_record ) + verts[ j ][ 1 ] * 1.0f, 0 },
						{ color.r, color.g, color.b, color.a },
						{ norm.x, norm.y, norm.z },
						{ uvs[ j ][ 0 ], uvs[ j ][ 1 ] },
						{ 0, 0, 0, 0 }
					} );
				}
			}
		}

		index_graph = mesh_buffer.size( );

		glm::ivec2 dim_char( 8, 12 );
		color = Color4( 0.0f, 1.0f, 0.0f, 0.5f );

		for( int i = 0; i < list_render.size( ); i++ ) {
			auto & record = get_record( *list_render[ i ] );
			size_history = record.history.size( );

			if( size_history > 0 ) {
				auto & string = *list_render[ i ];

				for( int j = 0; j < string.size( ); j++ ) {
					auto & uvs = client.texture_mgr.get_uvs_fonts( string[ j ] - 32 );

					for( int k = 0; k < 4; k++ ) {
						mesh_buffer.push_back( {
							{ ( -int( string.size() ) * dim_char.x ) - padding + j * dim_char.x + verts[ k ][ 0 ] * dim_char.x, 
							( -i * ( dim_graph.y + padding ) ) + ( 0.5f * dim_graph.y ) - ( 0.5f * dim_char.y ) + verts[ k ][ 1 ] * dim_char.y, 0 },
							{ color.r, color.g, color.b, color.a },
							{ norm.x, norm.y, norm.z },
							{ uvs[ k ][ 0 ], uvs[ k ][ 1 ] },
							{ 0, 0, 0, 0 }
						} );
					}
				}
			}
		}

		index_text = mesh_buffer.size( );

		glBindBuffer( GL_ARRAY_BUFFER, id_vbo );
		glBufferData( GL_ARRAY_BUFFER, index_text * sizeof( ChunkVert ), nullptr, GL_STATIC_DRAW );

		glBufferData( GL_ARRAY_BUFFER, index_text * sizeof( ChunkVert ), mesh_buffer.data(), GL_STATIC_DRAW );

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