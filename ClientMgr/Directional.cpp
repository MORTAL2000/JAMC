#include "Directional.h"

#include <sstream>

const glm::ivec3 
	Directional::FRONT_I( 0, 0, 1 ),
	Directional::BACK_I( 0, 0, -1 ),
	Directional::LEFT_I( 1, 0, 0 ),
	Directional::RIGHT_I( -1, 0, 0 ),
	Directional::UP_I( 0, 1, 0 ),
	Directional::DOWN_I( 0, -1, 0 );

const glm::vec3
	Directional::FRONT_F( FRONT_I ),
	Directional::BACK_F( BACK_I ),
	Directional::LEFT_F( LEFT_I ),
	Directional::RIGHT_F( RIGHT_I ),
	Directional::UP_F( UP_I ),
	Directional::DOWN_F( DOWN_I );

std::array< FaceDirection, FaceDirection::FD_Size > array_face_opposite { 
	FaceDirection::FD_Back,
	FaceDirection::FD_Front,
	FaceDirection::FD_Right,
	FaceDirection::FD_Left,
	FaceDirection::FD_Down,
	FaceDirection::FD_Up
};

std::array < glm::ivec3 const *, FD_Size > Directional::array_vec_dir_i { {
	&FRONT_I, &BACK_I,
	&LEFT_I, &RIGHT_I,
	&UP_I, &DOWN_I
	} };

std::array < glm::vec3 const *, FD_Size > Directional::array_vec_dir_f { {
	&FRONT_F, &BACK_F,
	&LEFT_F, &RIGHT_F,
	&UP_F, &DOWN_F
	} };

FaceDirection Directional::get_face_opposite( FaceDirection dir_face ) { 
	return array_face_opposite[ dir_face ];
}

glm::ivec3 const & Directional::get_vec_dir_i( FaceDirection dir_face ) {
	return *array_vec_dir_i[ dir_face ];
}

glm::vec3 const & Directional::get_vec_dir_f( FaceDirection dir_face ) {
	return *array_vec_dir_f[ dir_face ];
}

bool Directional::is_point_in_rect( glm::ivec2 & pos_check, glm::ivec2 & pos_rect1, glm::ivec2 & pos_rect2 ) {
	if( glm::greaterThanEqual( pos_check,  pos_rect1 ) == glm::bvec2( true, true ) && glm::lessThanEqual( pos_check, pos_rect2 ) == glm::bvec2( true, true ) ) { 
		return true;
	}
	return false;
}

bool Directional::is_point_in_region( glm::ivec3 & pos_check, glm::ivec3 & pos_reg1, glm::ivec3 & pos_reg2 ) {
	if( pos_check.x >= pos_reg1.x && pos_check.x <= pos_reg2.x &&
		pos_check.y >= pos_reg1.y && pos_check.y <= pos_reg2.y &&
		pos_check.z >= pos_reg1.z && pos_check.z <= pos_reg2.z ) {
		return true;
	}
	return false;
}

glm::vec3 Directional::get_fwd( glm::vec3 & rot ) {
	return glm::vec3(
		sin( rot.y * PI / 180.0f ) * cos( ( rot.x ) * PI / 180.0f ),
		-sin( rot.x * PI / 180.0f ),
		-cos( rot.y * PI / 180.0f ) * cos( ( rot.x ) * PI / 180.0f ) );
}

glm::vec3 Directional::get_fwd_aa( glm::vec3 & rot ) {
	return glm::vec3(
		sin( rot.y * PI / 180.0f ),
		0,
		-cos( rot.y * PI / 180.0f ) );
}

glm::vec3 Directional::get_up( glm::vec3 & rot ) {
	return glm::vec3(
		sin( rot.y * PI / 180.0f ) * cos( ( rot.x + 90 ) * PI / 180.0f ),
		-sin( ( rot.x + 90 ) * PI / 180.0f ),
		-cos( rot.y * PI / 180.0f ) * cos( ( rot.x + 90 )  * PI / 180.0f ) );
}

glm::vec3 Directional::get_up_aa( glm::vec3 & rot ) {
	return glm::vec3( 0, 1, 0 );
}

glm::vec3 Directional::get_left( glm::vec3 & rot ) {
	return glm::vec3(
		sin( ( rot.y - 90 ) * PI / 180.0f ),
		0,
		-cos( ( rot.y - 90 ) * PI / 180.0f ) );
}

bool Directional::is_point_in_cone( glm::ivec3 & point, glm::vec3 & cone_apex, glm::vec3 & cone_base, float aperture ) {
	float half_aperture = aperture / 2.0f;
	glm::vec3 apex_to_point = glm::vec3( point ) - cone_apex;
	glm::vec3 apex_to_base = cone_base - cone_apex;

	bool is_infinite = glm::dot( apex_to_point, apex_to_base ) /
		glm::length( apex_to_point ) / glm::length( apex_to_base ) >
		glm::cos( glm::radians( half_aperture ) );

	return is_infinite;
}

bool Directional::is_within_range( glm::ivec3 const & point, glm::ivec3 const & range, glm::ivec3 const & check ) {
	glm::ivec3 diff = check - point;
	diff = glm::abs( diff );
	return ( diff.x <= range.x ) && ( diff.y <= range.y ) && ( diff.z <= range.z );
}

bool Directional::is_within_range( glm::ivec2 const & point, glm::ivec2 const & range, glm::ivec2 const & check ) {
	glm::ivec2 diff = check - point;
	diff = glm::abs( diff );
	return ( diff.x <= range.x ) && ( diff.y <= range.y );
}

bool Directional::is_within_range( glm::ivec3 const & point, int const range, glm::ivec3 const & check ) {
	glm::ivec3 diff = check - point;
	diff = glm::abs( diff );
	return ( diff.x <= range ) && ( diff.y <= range ) && ( diff.z <= range );
}

std::string Directional::print_vec( glm::ivec2 vec ) {
	std::ostringstream output;
	output << vec.x << ", " << vec.y;
	return output.str( );
}

std::string Directional::print_vec( glm::vec2 vec ) {
	std::ostringstream output;
	output << vec.x << ", " << vec.y;
	return output.str( );
}

std::string Directional::print_vec( glm::ivec3 vec ) {
	std::ostringstream output;
	output << vec.x << ", " << vec.y << ", " << vec.z;
	return output.str( );
}

std::string Directional::print_vec( glm::vec3 vec ) {
	std::ostringstream output;
	output << vec.x << ", " << vec.y << ", " << vec.z;
	return output.str( );
}

std::string Directional::print_vec( glm::ivec4 vec ) {
	std::ostringstream output;
	output << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w;
	return output.str( );
}

std::string Directional::print_vec( glm::vec4 vec ) {
	std::ostringstream output;
	output << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w;
	return output.str( );
}
