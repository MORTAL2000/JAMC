#pragma once

#include "Globals.h"

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
	static void pos_gw_to_lc( glm::ivec3 const & pos_gw, glm::ivec3 & pos_lc );
	static void pos_gw_to_lc( glm::vec3 const & pos_gw, glm::ivec3 & pos_lc );

	static void pos_gw_to_lw( glm::ivec3 const & pos_gw, glm::ivec3 & pos_lw );
	static void pos_gw_to_lw( glm::vec3 const & pos_gw, glm::ivec3 & pos_lw );

	static void pos_lw_to_r( glm::ivec3 const & pos_lw, glm::ivec3 & pos_r );
	static void pos_lw_to_lr( glm::ivec3 const & pos_lw, glm::ivec3 & pos_lr );

	static void pos_trim( glm::vec3 & pos_gw, glm::ivec3 & pos_trim );
	static void pos_trim( glm::vec3 & pos_gw, glm::vec3 & pos_trim );

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
		return ( ( 251 + vec.x ) * 251 ) + vec.y;
	}

	static int get_hash( glm::ivec3 vec ) { 
		return ( ( ( ( 251 + vec.x ) * 251 ) + vec.y ) * 251 ) + vec.z;
	}
};