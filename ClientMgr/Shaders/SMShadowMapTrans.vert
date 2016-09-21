#version 430 core

const vec4 offset_face[ 6 ][ 6 ] = {
	{ vec4( 0, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 0, 1, 0 ) },
	{ vec4( 1, 0, 0, 0 ), vec4( 0, 0, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 0, 0, 0 ) },

	{ vec4( 1, 0, 1, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 1, 0 ), vec4( 1, 0, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 0, 0, 0 ) }, 

	{ vec4( 1, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 1, 1, 0, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 0, 0, 0 ) }
};

const vec3 norm_face[ 6 ] = {
	vec3( 0, 0, 1 ),
	vec3( 0, 0, -1 ),
	vec3( 1, 0, 0 ),
	vec3( -1, 0, 0 ),
	vec3( 0, 1, 0 ),
	vec3( 0, -1, 0 )
};

const vec2 uvs_face[ 6 ] = {
	vec2( 0, 0 ),
	vec2( 1, 0 ),
	vec2( 1, 1 ),
	vec2( 1, 1 ),
	vec2( 0, 1 ),
	vec2( 0, 0 )
};

const vec4 scale_face[ 6 ] = {
	vec4( 1, 0, 0, 0 ),
	vec4( 1, 0, 0, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 
};

const vec2 scale_uvs[ 6 ] = {
	vec2( 1, 0 ),
	vec2( 1, 0 ),

	vec2( 1, 0 ),
	vec2( 1, 0 ),

	vec2( 0, 1 ),
	vec2( 0, 1 )
};

// In Data
layout( location = 0 ) in uint data1;
layout( location = 1 ) in uint data2;
layout( location = 2 ) in uint id;
layout( location = 3 ) in vec3 vec_model;

uniform mat4 mat_light;

out vec3 frag_uv;
out vec4 frag_color;

void main() {
	vec4 vert, color;
	uint orient, scale;
	uint tex1, tex2;
	int id_vert = gl_VertexID % 6;

	// Extract data
	vert.x = float( ( data1 >> 0 ) & 31 );
	vert.y = float( ( data1 >> 5 ) & 63 );
	vert.z = float( ( data1 >> 11 ) & 31 );
	vert.w = 1;

	color.r = float( ( data1 >> 16u ) & 31u ) / 32.0;
	color.g = float( ( data1 >> 21u ) & 31u ) / 32.0;
	color.b = float( ( data1 >> 26u ) & 31u ) / 32.0;
	color.a = 1.0; /*float( ( ( ( data1 >> 31u ) & 1u ) ) + ( ( ( data2 >> 0u ) & 15u ) << 1u ) ) / 32.0;*/

	tex1 = ( data2 >> 4 ) & 1023;
	tex2 = ( data2 >> 14 ) & 1023; 

	orient = ( data2 >> 24 ) & 7;
	scale = ( data2 >> 27 ) & 31;

	vert = vert + offset_face[ orient ][ id_vert ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale );

	frag_uv = vec3( uvs_face[ id_vert ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex1 );
	frag_color = color;
	gl_Position = mat_light * ( vec4( vec_model, 0 ) + vert );
}