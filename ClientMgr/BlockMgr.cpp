#include "BlockMgr.h"

#include <filesystem>
#include <sstream>
#include <iostream>

#include "glm\glm.hpp"
#include "glm\gtx\transform.hpp"
#include "tinyxml2-master\tinyxml2.h"

#include "Client.h"
#include "Directional.h"
#include "TextureMgr.h"

BlockMgr::BlockMgr( Client & client ) : 
	Manager( client ) { }

BlockMgr::~BlockMgr( ) { }

void BlockMgr::init( ) { 
	load_block_data( );
	load_block_mesh( );
}

void BlockMgr::update( ) { }

void BlockMgr::render( ) { }

void BlockMgr::end( ) { }

void BlockMgr::sec( ) { }

void BlockMgr::load_block_data( ) {
	using namespace std::tr2::sys;

	path path_base( "./Blocks" );
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLElement const * ele_block;
	tinyxml2::XMLElement const * ele_face;
	tinyxml2::XMLElement const * ele_data;
	tinyxml2::XMLElement const * ele_var;

	for( directory_iterator iter_blocks( path_base ); iter_blocks != directory_iterator( ); ++iter_blocks ) {
		if( iter_blocks->path( ).extension( ).string( ) != ".xml" ) {
			continue;
		}

		BlockLoader loader_block;

		doc.LoadFile( iter_blocks->path( ).string( ).c_str( ) );
		if( doc.Error( ) ) {
			std::cout << "ERROR! Error reading: " << iter_blocks->path( ).string( ) << std::endl;
			doc.PrintError( );
			continue;
		}
		ele_block = doc.FirstChildElement( "Block" );

		ele_data = ele_block->FirstChildElement( "Name" );
		if( ele_data != nullptr ) {
			loader_block.name = ele_data->GetText( );
		}

		ele_data = ele_block->FirstChildElement( "Texture" );
		if( ele_data != nullptr ) {
			loader_block.texture = ele_data->GetText( );
			loader_block.id_texture = client.texture_mgr.get_texture_id( loader_block.texture );
		}

		ele_data = ele_block->FirstChildElement( "Transparent" );
		if( ele_data != nullptr ) {
			ele_data->QueryBoolText( &loader_block.is_trans );
		}

		ele_data = ele_block->FirstChildElement( "Color" );
		if( ele_data != nullptr ) {
			ele_var = ele_data->FirstChildElement( "R" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &loader_block.color.r );
			}

			ele_var = ele_data->FirstChildElement( "G" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &loader_block.color.g );
			}

			ele_var = ele_data->FirstChildElement( "B" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &loader_block.color.b );
			}

			ele_var = ele_data->FirstChildElement( "A" );
			if( ele_var != nullptr ) {
				ele_var->QueryFloatText( &loader_block.color.a );
			}
		}

		ele_data = ele_block->FirstChildElement( "Collider" );
		if( ele_data != nullptr ) {
			loader_block.is_coll = true;
		}

		ele_face = ele_block->FirstChildElement( "Face" );
		while( ele_face != nullptr ) {
			Face face;

			ele_data = ele_face->FirstChildElement( "Offset" );
			if( ele_data != nullptr ) {
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.offset.x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.offset.y );
				}

				ele_var = ele_data->FirstChildElement( "Z" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.offset.z );
				}
			}

			ele_data = ele_face->FirstChildElement( "Dimension" );
			if( ele_data != nullptr ) {
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.dim.x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.dim.y );
				}

				ele_var = ele_data->FirstChildElement( "Z" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.dim.z );
				}
			}

			ele_data = ele_face->FirstChildElement( "Rotation" );
			if( ele_data != nullptr ) {
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.rot.x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.rot.y );
				}

				ele_var = ele_data->FirstChildElement( "Z" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.rot.z );
				}
			}

			ele_data = ele_face->FirstChildElement( "Color" );
			if( ele_data != nullptr ) {
				ele_var = ele_data->FirstChildElement( "R" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.r );
				}

				ele_var = ele_data->FirstChildElement( "G" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.g );
				}

				ele_var = ele_data->FirstChildElement( "B" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.b );
				}

				ele_var = ele_data->FirstChildElement( "A" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.color.a );
				}
			}

			ele_data = ele_face->FirstChildElement( "Occlude" );
			if( ele_data != nullptr ) {
				std::string text( ele_data->GetText( ) );

				if( text == "Front" ) {
					face.occlude = FaceDirection::FD_Front;
				}
				else if( text == "Back" ) {
					face.occlude = FaceDirection::FD_Back;
				}
				else if( text == "Left" ) {
					face.occlude = FaceDirection::FD_Left;
				}
				else if( text == "Right" ) {
					face.occlude = FaceDirection::FD_Right;
				}
				else if( text == "Up" ) {
					face.occlude = FaceDirection::FD_Up;
				}
				else if( text == "Down" ) {
					face.occlude = FaceDirection::FD_Down;
				}
				else {
					face.occlude = FaceDirection::FD_Size;
				}
			}

			ele_data = ele_face->FirstChildElement( "SubTexture" );
			if( ele_data != nullptr ) {
				face.subtex = ele_data->GetText( );
				face.id_subtex = client.texture_mgr.get_texture_layer( loader_block.texture, face.subtex );
			}

			ele_data = ele_face->FirstChildElement( "UV" );
			int cnt = 0;
			while( ele_data != nullptr && cnt < 4 ) {
				ele_var = ele_data->FirstChildElement( "X" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.uvs[ cnt ].x );
				}

				ele_var = ele_data->FirstChildElement( "Y" );
				if( ele_var != nullptr ) {
					ele_var->QueryFloatText( &face.uvs[ cnt ].y );
				}

				cnt++;
				ele_data = ele_data->NextSiblingElement( "UV" );
			}

			loader_block.faces.emplace_back( face );

			ele_face = ele_face->NextSiblingElement( "Face" );
		}

		for( int i = 0; i < loader_block.faces.size( ); ++i ) {
			auto & face = loader_block.faces[ i ];

			if( face.occlude != FaceDirection::FD_Size ) {
				loader_block.occlude_lookup[ face.occlude ].emplace_back( i );
			}
			else {
				loader_block.include_lookup.emplace_back( i );
			}

			glm::vec3 dim_h = face.dim / 2.0f;
			glm::vec3 pos_c = { 0.5f, 0.5f, 0.5f };
			glm::mat4 rotate_z = glm::rotate( glm::mat4( 1.0f ), glm::radians( face.rot.z ), glm::vec3( 0, 0, 1 ) );
			glm::mat4 rotate_y = glm::rotate( glm::mat4( 1.0f ), glm::radians( face.rot.y ), glm::vec3( 0, 1, 0 ) );
			glm::mat4 rotate_x = glm::rotate( glm::mat4( 1.0f ), glm::radians( face.rot.x ), glm::vec3( 1, 0, 0 ) );

			face.verts[ 0 ] = { dim_h.x, -dim_h.y, 0 };
			face.verts[ 1 ] = { -dim_h.x, -dim_h.y, 0 };
			face.verts[ 2 ] = { -dim_h.x, dim_h.y, 0 };
			face.verts[ 3 ] = { dim_h.x, dim_h.y, 0 };

			face.norms[ 0 ] = { 0, 0, -1 };
			face.norms[ 1 ] = { 0, 0, -1 };
			face.norms[ 2 ] = { 0, 0, -1 };
			face.norms[ 3 ] = { 0, 0, -1 };

			for( auto & vert : face.verts ) {
				vert = glm::vec3( rotate_z * glm::vec4( vert, 0 ) );
				vert = glm::vec3( rotate_y * glm::vec4( vert, 0 ) );
				vert = glm::vec3( rotate_x * glm::vec4( vert, 0 ) );
				vert = vert + pos_c + face.offset;
			}

			for( auto & norm : face.norms ) {
				norm = glm::vec3( rotate_z * glm::vec4( norm, 0 ) );
				norm = glm::vec3( rotate_y * glm::vec4( norm, 0 ) );
				norm = glm::vec3( rotate_x * glm::vec4( norm, 0 ) );
				norm = glm::normalize( norm );
			}

			/*glm::vec2 dim_dx = { 1.0f, 1.0f };
			glm::ivec2 const & dim = TextureMgr::get_texture( block.texture )->dim;
			dim_dx /= dim;
			int num_x, num_y;*/

			for( auto & uv : face.uvs ) {
				/*num_x = std::floor( uv.x / dim_dx.x );
				num_y = std::floor( uv.y / dim_dx.y );

				if( num_x == 0 ) num_x = 1;
				if( num_y == 0 ) num_y = 1;

				uv.x = num_x * dim_dx.x - dim_dx.x / 2.0f;
				uv.y = num_y * dim_dx.y - dim_dx.y / 2.0f;


				std::cout << "Uv: num_x: " << num_x << " num_y: " << num_y << std::endl;
				std::cout << "dim_dx: " << Directional::print_vec( dim_dx ) << " uv: " << Directional::print_vec( uv ) << std::endl;*/

				uv.z = face.id_subtex;
			}
		}

		loader_block.id = ( int ) list_block_loader.size( );
		list_block_loader.emplace_back( loader_block );
		map_block_loader.insert( { loader_block.name, ( GLuint ) list_block_loader.size( ) - 1 } );
	}
}

void BlockMgr::load_block_mesh( ) {
	/*for( auto & block : list_block_data ) {
		SMChunkIncl::SMHandle handle;

		handle.clear( );

		sm_inclusive.request_handle( handle );

		handle.push_set( SMChunkIncl::SMGSet(
			SMChunkIncl::TypeGeometry::TG_Triangles,
			glm::mat4( 1.0f ),
			TextureMgr::get_program_id( "SMTerrain" ),
			TextureMgr::get_texture_id( "Blocks" ) ) );

		handle.request_buffer( );

		GLuint idx_inds = 0;

		for( auto idx_face : block.include_lookup ) {
			auto & face = block.faces[ idx_face ];
			handle.push_verts( {
				{
					{ face.verts[ 0 ].x, face.verts[ 0 ].y, face.verts[ 0 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 0 ].x, face.norms[ 0 ].y, face.norms[ 0 ].z },
					{ face.uvs[ 0 ].x, face.uvs[ 0 ].y, face.uvs[ 0 ].z }
				},
				{
					{ face.verts[ 1 ].x, face.verts[ 1 ].y, face.verts[ 1 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 1 ].x, face.norms[ 1 ].y, face.norms[ 1 ].z },
					{ face.uvs[ 1 ].x, face.uvs[ 1 ].y, face.uvs[ 1 ].z }
				},
				{
					{ face.verts[ 2 ].x, face.verts[ 2 ].y, face.verts[ 2 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 2 ].x, face.norms[ 2 ].y, face.norms[ 2 ].z },
					{ face.uvs[ 2 ].x, face.uvs[ 2 ].y, face.uvs[ 2 ].z }
				},
				{
					{ face.verts[ 3 ].x, face.verts[ 3 ].y, face.verts[ 3 ].z },
					{ block.color.r * face.color.r * 255, block.color.g * face.color.g * 255, block.color.b * face.color.b * 255, block.color.a * face.color.a * 255 },
					{ face.norms[ 3 ].x, face.norms[ 3 ].y, face.norms[ 3 ].z },
					{ face.uvs[ 3 ].x, face.uvs[ 3 ].y, face.uvs[ 3 ].z }
				}
			} );

			handle.push_inds( {
				idx_inds + 0, idx_inds + 1, idx_inds + 2, idx_inds + 2, idx_inds + 3, idx_inds + 0,
			} );

			idx_inds += 4;
		}

		handle.finalize_set( );

		handle.submit_buffer( );

		handle.release_buffer( );

		list_handles_inclusive.push_back( handle );
	}*/
}

BlockLoader * BlockMgr::get_block_loader( int const id_block ) {
	return &list_block_loader[ id_block ];
}

BlockLoader * BlockMgr::get_block_loader_safe( int const id_block ) { 
	if( id_block >= 0 && id_block < list_block_loader.size( ) ) { 
		return &list_block_loader[ id_block ];
	}

	return nullptr;
}

BlockLoader * BlockMgr::get_block_loader( std::string const & name ) {
	return &list_block_loader[ map_block_loader[ name ] ];
}

BlockLoader * BlockMgr::get_block_loader_safe( std::string const & name ) {
	auto iter_name = map_block_loader.find( name );

	if( iter_name == map_block_loader.end( ) ) { 
		return nullptr;
	}

	return &list_block_loader[ map_block_loader[ name ] ];
}

std::string const & BlockMgr::get_block_string( int const id ) {
	static std::string const str_air( "Air" );
	static std::string const str_null( "Null" );

	if( id == -1 ) {
		return str_air;
	}
	else if( id >= 0 && id < list_block_loader.size( ) ) {
		return list_block_loader[ id ].name;
	}
	else {
		return str_null;
	}
}

int BlockMgr::get_num_blocks( ) {
	return ( int ) list_block_loader.size( );
}
