#version 430 core

const vec4 offset_face[ 6 ][ 4 ] = {
	{ vec4( 0, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 0, 1, 1, 0 ) },
	{ vec4( 1, 0, 0, 0 ), vec4( 0, 0, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 1, 1, 0, 0 ) },

	{ vec4( 1, 0, 1, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 0, 0 ) }, 

	{ vec4( 1, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 1, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 0, 1, 0 ), vec4( 0, 0, 1, 0 ) }
};

const vec4 scale_face[ 6 ] = {
	vec4( 1, 0, 0, 0 ),
	vec4( 1, 0, 0, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 
};

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in V_DATA {
	uint data1;
	uint data2;
	vec3 vec_model;
} v_in[];

uniform mat4 mat_light;

void main() {
	vec4 vert;
	vert.x = float( ( v_in[ 0 ].data1 >> 0 ) & 31 );
	vert.y = float( ( v_in[ 0 ].data1 >> 5 ) & 63 );
	vert.z = float( ( v_in[ 0 ].data1 >> 11 ) & 31 );
	vert.w = 1;

	uint orient = ( v_in[ 0 ].data2 >> 24 ) & 7;
	uint scale = ( v_in[ 0 ].data2 >> 27 ) & 31;

	gl_Position = mat_light * ( vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 0 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale ) );
	EmitVertex();

	gl_Position = mat_light * ( vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 1 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale ) );
	EmitVertex();

	gl_Position = mat_light * ( vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 3 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale ) );
	EmitVertex();

	gl_Position = mat_light * ( vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 2 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale ) );
	EmitVertex();
	
	EndPrimitive();
}