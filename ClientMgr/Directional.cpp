#include "Directional.h"

#include "ChunkMgr.h"

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
	// This is for our convenience
	float halfAperture = aperture / 2.0f;

	// std::vector pointing to X point from apex
	glm::vec3 apex_to_point = glm::vec3( point ) - cone_apex;

	// std::vector pointing from apex to circle-center point.
	glm::vec3 apex_to_base = cone_base - cone_apex;

	bool is_infinite = glm::dot( apex_to_point, apex_to_base ) /
		glm::length( apex_to_point ) / glm::length( apex_to_base ) >
		cos( halfAperture * PI / 180.0f );

	/*if( !is_infinite ) return false;

	bool is_under_cap = dot_prod( apex_to_point, axis_vect )
	/ axis_vect.mag()
	<
	axis_vect.mag();*/
	return is_infinite;
}

void Directional::pos_gw_to_lc( glm::ivec3 const & pos_gw, glm::ivec3 & pos_lc ) {
	pos_lc.x = pos_gw.x % Chunk::size_x;
	pos_lc.y = pos_gw.y % Chunk::size_y;
	pos_lc.z = pos_gw.z % Chunk::size_z;

	if( pos_lc.x < 0 ) pos_lc.x += Chunk::size_x;
	if( pos_lc.y < 0 ) pos_lc.y += Chunk::size_y;
	if( pos_lc.z < 0 ) pos_lc.z += Chunk::size_z;
}

void Directional::pos_gw_to_lc( glm::vec3 const & pos_gw, glm::ivec3 & pos_lc ) {
	pos_lc.x = int( floor( pos_gw.x ) ) % Chunk::size_x;
	pos_lc.y = int( floor( pos_gw.y ) ) % Chunk::size_y;
	pos_lc.z = int( floor( pos_gw.z ) ) % Chunk::size_z;

	if( pos_lc.x < 0 ) pos_lc.x += Chunk::size_x;
	if( pos_lc.y < 0 ) pos_lc.y += Chunk::size_y;
	if( pos_lc.z < 0 ) pos_lc.z += Chunk::size_z;
}

void Directional::pos_gw_to_lw( glm::ivec3 const & pos_gw, glm::ivec3 & pos_lw ) {
	pos_lw.x = floor( float( pos_gw.x ) / Chunk::size_x );
	pos_lw.y = floor( float( pos_gw.y ) / Chunk::size_y );
	pos_lw.z = floor( float( pos_gw.z ) / Chunk::size_z );
}

void Directional::pos_gw_to_lw( glm::vec3 const & pos_gw, glm::ivec3 & pos_lw ) {
	pos_lw.x = floor( pos_gw.x / Chunk::size_x );
	pos_lw.y = floor( pos_gw.y / Chunk::size_y );
	pos_lw.z = floor( pos_gw.z / Chunk::size_z );
}

void Directional::pos_lw_to_r( glm::ivec3 const & pos_lw, glm::ivec3 & pos_r ) {
	pos_r.x = floor( float( pos_lw.x ) / Region::size_x );
	pos_r.y = floor( float( pos_lw.y ) / Region::size_y );
	pos_r.z = floor( float( pos_lw.z ) / Region::size_z );
}

void Directional::pos_lw_to_lr( glm::ivec3 const & pos_lw, glm::ivec3 & pos_lr ) {
	pos_lr.x = pos_lw.x % Region::size_x;
	pos_lr.y = pos_lw.y % Region::size_y;
	pos_lr.z = pos_lw.z % Region::size_z;

	if( pos_lr.x < 0 ) pos_lr.x += Region::size_x;
	if( pos_lr.y < 0 ) pos_lr.y += Region::size_y;
	if( pos_lr.z < 0 ) pos_lr.z += Region::size_z;
}

void Directional::pos_trim( glm::vec3 & pos_gw ) { 
	//pos_lc.x = int( floor( pos_gw.x ) );
	//pos_lc.y = int( floor( pos_gw.y ) );
	//pos_lc.z = int( floor( pos_gw.z ) );
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
