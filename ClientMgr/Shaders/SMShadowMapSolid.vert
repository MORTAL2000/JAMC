#version 430 core

const vec4 offset_face[ 6 ][ 6 ] = {
	{ vec4( 0, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 0, 1, 0 ) },
	{ vec4( 1, 0, 0, 0 ), vec4( 0, 0, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 0, 0, 0 ) },

	{ vec4( 1, 0, 1, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 1, 0 ), vec4( 1, 0, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 0, 0, 0 ) }, 

	{ vec4( 1, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 1, 1, 0, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 0, 0, 0 ) }
};

const vec4 scale_face[ 6 ] = {
	vec4( 1, 0, 0, 0 ),
	vec4( 1, 0, 0, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 
};

// In Data
layout( location = 0 ) in uint data1;
layout( location = 1 ) in uint data2;
layout( location = 2 ) in uint id;
layout( location = 3 ) in vec3 vec_model;

uniform mat4 mat_light;

void main() {
	vec4 vert;
	uint orient, scale;
	int id_vert = gl_VertexID % 6;

	// Extract data
	vert.x = float( ( data1 >> 0 ) & 31 );
	vert.y = float( ( data1 >> 5 ) & 63 );
	vert.z = float( ( data1 >> 11 ) & 31 );
	vert.w = 1;

	orient = ( data2 >> 24 ) & 7;
	scale = ( data2 >> 27 ) & 31;

	vert = vert + offset_face[ orient ][ id_vert ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale );

	gl_Position = mat_light * ( vec4( vec_model, 0 ) + vert );
}