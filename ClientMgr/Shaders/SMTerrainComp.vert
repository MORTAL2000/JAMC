#version 430

const vec4 offset_face[ 6 ][ 4 ] = {
	{ vec4( 0, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 0, 1, 1, 0 ) },
	{ vec4( 1, 0, 0, 0 ), vec4( 0, 0, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 1, 1, 0, 0 ) },

	{ vec4( 1, 0, 1, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 0, 0 ) }, 

	{ vec4( 1, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 1, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 0, 1, 0 ), vec4( 0, 0, 1, 0 ) }
};

const vec2 uvs_face[ 4 ] = {
	vec2( 0, 0 ),
	vec2( 1, 0 ),
	vec2( 1, 1 ),
	vec2( 0, 1 )
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

layout( std140 ) uniform mvp_matrices {
	vec4 pos_camera;
	mat4 mat_world;
	mat4 mat_perspective;
	mat4 mat_ortho;
	mat4 mat_view;
	float time_game;
};

const int max_emitters = 128;
layout( std140 ) uniform light_data {
	vec4 pos_sun;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	ivec4 num_emitters;
	vec4 list_pos[ max_emitters ];
	vec4 list_color[ max_emitters ];
	vec4 list_radius[ max_emitters ];
};

layout( location = 0 ) in uint data1;
layout( location = 1 ) in uint data2;
layout( location = 2 ) in uint id;
layout( location = 3 ) in mat4 mat_model;
layout( location = 7 ) in mat3 mat_norm;

out vec3 frag_uvs;
out vec4 frag_color;

void main() {
	vec4 pos, color;
	uint orient, scale;
	uint tex1, tex2;
	int id_vert = gl_VertexID % 4;

	pos.x = float( ( data1 >> 0 ) & 31 );
	pos.y = float( ( data1 >> 5 ) & 63 );
	pos.z = float( ( data1 >> 11 ) & 31 );
	pos.w = 1;

	color.r = float( ( data1 >> 16u ) & 31u ) / 32.0;
	color.g = float( ( data1 >> 21u ) & 31u ) / 32.0;
	color.b = float( ( data1 >> 26u ) & 31u ) / 32.0;
	color.a = 1.0;/*float( 
		( ( ( data1 >> 31u ) & 1u ) ) + ( ( ( data2 >> 0u ) & 15u ) << 1u )
	) / 32.0;*/

	tex1 = ( data2 >> 4 ) & 1023;
	tex2 = ( data2 >> 14 ) & 1023; 

	orient = ( data2 >> 24 ) & 7;
	scale = ( data2 >> 27 ) & 31;

	pos = pos + offset_face[ orient ][ id_vert ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale );

	gl_Position = mat_perspective * mat_view * mat_model * pos;
	frag_uvs = vec3( uvs_face[ id_vert ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex1 );
	frag_color = color;
}