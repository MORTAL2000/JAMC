#pragma once
#include "Globals.h"

#include "WorldSize.h"

#include "glm/glm.hpp"

#include <array>

enum FaceDirection {
	FD_Front,
	FD_Back,
	FD_Left,
	FD_Right,
	FD_Up,
	FD_Down,
	FD_Size
};

class Directional {
public:
	static const glm::ivec3 FRONT_I, BACK_I;
	static const glm::ivec3 LEFT_I, RIGHT_I;
	static const glm::ivec3 UP_I, DOWN_I;

	static const glm::vec3 FRONT_F, BACK_F;
	static const glm::vec3 LEFT_F, RIGHT_F;
	static const glm::vec3 UP_F, DOWN_F;

	static std::array < glm::ivec3 const *, FD_Size > array_vec_dir_i;
	static std::array < glm::vec3 const *, FD_Size > array_vec_dir_f;

	static std::array< FaceDirection, FaceDirection::FD_Size > array_opposite_face;

	static glm::ivec3 const & get_vec_dir_i( FaceDirection dir_face );
	static glm::vec3 const & get_vec_dir_f( FaceDirection dir_face );

	static FaceDirection get_face_opposite( FaceDirection dir_face );

	static bool is_point_in_rect( glm::ivec2 & pos_check, glm::ivec2 & pos_rect1, glm::ivec2 & pos_rect2 );

	static bool is_point_in_region( glm::ivec3 & pos_check, glm::ivec3 & pos_reg1, glm::ivec3 & pos_reg2 );

	// Vec directional functions
	static glm::vec3 get_fwd( glm::vec3 & rot );
	static glm::vec3 get_fwd_aa( glm::vec3 & rot );
	static glm::vec3 get_up( glm::vec3 & rot );
	static glm::vec3 get_up_aa( glm::vec3 & rot );
	static glm::vec3 get_left( glm::vec3 & rot );

	static bool is_point_in_cone( glm::ivec3 & point, glm::vec3 & cone_apex, glm::vec3 & cone_base, float aperture );

	// Vec morph functions
	static inline void Directional::pos_gw_to_lc( glm::ivec3 const & pos_gw, glm::ivec3 & pos_lc ) {
		pos_lc.x = pos_gw.x % WorldSize::Chunk::size_x;
		pos_lc.y = pos_gw.y % WorldSize::Chunk::size_y;
		pos_lc.z = pos_gw.z % WorldSize::Chunk::size_z;

		if( pos_lc.x < 0 ) pos_lc.x += WorldSize::Chunk::size_x;
		if( pos_lc.y < 0 ) pos_lc.y += WorldSize::Chunk::size_y;
		if( pos_lc.z < 0 ) pos_lc.z += WorldSize::Chunk::size_z;
	}

	static inline void Directional::pos_gw_to_lc( glm::vec3 const & pos_gw, glm::ivec3 & pos_lc ) {
		pos_lc.x = int( floor( pos_gw.x ) ) % WorldSize::Chunk::size_x;
		pos_lc.y = int( floor( pos_gw.y ) ) % WorldSize::Chunk::size_y;
		pos_lc.z = int( floor( pos_gw.z ) ) % WorldSize::Chunk::size_z;

		if( pos_lc.x < 0 ) pos_lc.x += WorldSize::Chunk::size_x;
		if( pos_lc.y < 0 ) pos_lc.y += WorldSize::Chunk::size_y;
		if( pos_lc.z < 0 ) pos_lc.z += WorldSize::Chunk::size_z;
	}

	static inline void Directional::pos_gw_to_lw( glm::ivec3 const & pos_gw, glm::ivec3 & pos_lw ) {
		pos_lw.x = ( int ) floor( float( pos_gw.x ) / WorldSize::Chunk::size_x );
		pos_lw.y = ( int ) floor( float( pos_gw.y ) / WorldSize::Chunk::size_y );
		pos_lw.z = ( int ) floor( float( pos_gw.z ) / WorldSize::Chunk::size_z );
	}

	static inline void Directional::pos_gw_to_lw( glm::vec3 const & pos_gw, glm::ivec3 & pos_lw ) {
		pos_lw.x = ( int ) floor( pos_gw.x / WorldSize::Chunk::size_x );
		pos_lw.y = ( int ) floor( pos_gw.y / WorldSize::Chunk::size_y );
		pos_lw.z = ( int ) floor( pos_gw.z / WorldSize::Chunk::size_z );
	}

	static inline void Directional::pos_lw_to_r( glm::ivec3 const & pos_lw, glm::ivec3 & pos_r ) {
		pos_r.x = ( int ) floor( float( pos_lw.x ) / WorldSize::Region::size_x );
		pos_r.y = ( int ) floor( float( pos_lw.y ) / WorldSize::Region::size_y );
		pos_r.z = ( int ) floor( float( pos_lw.z ) / WorldSize::Region::size_z );
	}

	static inline void Directional::pos_lw_to_lr( glm::ivec3 const & pos_lw, glm::ivec3 & pos_lr ) {
		pos_lr.x = pos_lw.x % WorldSize::Region::size_x;
		pos_lr.y = pos_lw.y % WorldSize::Region::size_y;
		pos_lr.z = pos_lw.z % WorldSize::Region::size_z;

		if( pos_lr.x < 0 ) pos_lr.x += WorldSize::Region::size_x;
		if( pos_lr.y < 0 ) pos_lr.y += WorldSize::Region::size_y;
		if( pos_lr.z < 0 ) pos_lr.z += WorldSize::Region::size_z;
	}

	static inline void Directional::pos_trim( glm::vec3 & pos_gw, glm::ivec3 & pos_trim ) {
		pos_trim.x = ( int ) floor( pos_gw.x );
		pos_trim.y = ( int ) floor( pos_gw.y );
		pos_trim.z = ( int ) floor( pos_gw.z );
	}

	static inline void Directional::pos_trim( glm::vec3 & pos_gw, glm::vec3 & pos_trim ) {
		pos_trim.x = floor( pos_gw.x );
		pos_trim.y = floor( pos_gw.y );
		pos_trim.z = floor( pos_gw.z );
	}

	// Vec Range functions
	static bool is_within_range( glm::ivec3 const & point, glm::ivec3  const & range, glm::ivec3  const & check );
	static bool is_within_range( glm::ivec2  const & point, glm::ivec2  const & range, glm::ivec2  const & check );
	static bool is_within_range( glm::ivec3  const & point, int const range, glm::ivec3 const & check );

	static std::string print_vec( glm::ivec2 vec );
	static std::string print_vec( glm::vec2 vec );

	static std::string print_vec( glm::ivec3 vec );
	static std::string print_vec( glm::vec3 vec );

	static std::string print_vec( glm::ivec4 vec );
	static std::string print_vec( glm::vec4 vec );

	static int get_max( glm::ivec3 pos ) {
		int temp = std::abs( pos.x );
		if( std::abs( pos.y ) > temp ) temp = std::abs( pos.y );
		if( std::abs( pos.z ) > temp ) temp = std::abs( pos.z );
		return temp;
	}

	static int get_hash( glm::ivec2 vec ) {
		return ( ( 263 + vec.x ) * 257 ) + vec.y;
	}

	static int get_hash( glm::ivec3 vec ) { 
		return ( ( ( ( 263 + vec.x ) * 257 ) + vec.y ) * 251 ) + vec.z;
	}
};