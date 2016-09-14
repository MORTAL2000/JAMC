#version 430

const vec4 offset_face[ 6 ][ 4 ] = {
	{ vec4( 0, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 0, 1, 1, 0 ) },
	{ vec4( 1, 0, 0, 0 ), vec4( 0, 0, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 1, 1, 0, 0 ) },

	{ vec4( 1, 0, 1, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 0, 0 ) }, 

	{ vec4( 1, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 1, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 0, 1, 0 ), vec4( 0, 0, 1, 0 ) }
};

const vec3 norm_face[ 6 ] = {
	vec3( 0, 0, 1 ),
	vec3( 0, 0, -1 ),
	vec3( 1, 0, 0 ),
	vec3( -1, 0, 0 ),
	vec3( 0, 1, 0 ),
	vec3( 0, -1, 0 )
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

// In Data
layout( location = 0 ) in uint data1;
layout( location = 1 ) in uint data2;
layout( location = 2 ) in uint id;
layout( location = 3 ) in vec3 vec_model;

// Uniform Data
const uint MAX_CASCADES = 8;
uniform mat4 mat_light[ MAX_CASCADES ];
uniform uint num_cascades;

uniform float dist_fade = 250.0;
uniform float dist_fade_cutoff = 300.0;

// Out Data
out FRAG_OUT {
	vec3 dir_sun;
	vec4 frag_diffuse;

	vec3 frag_uvs[ 2 ];
	vec4 frag_color;
	vec3 frag_norm;

	vec4 vert_model;
	vec4 vert_view;
	vec4 vert_light[ MAX_CASCADES ];
} frag_out;

void main() {
	vec4 vert, color;
	vec3 norm;
	uint orient, scale;
	uint tex1, tex2;
	int id_vert = gl_VertexID % 4;

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
	norm = norm_face[ orient ];

	// Vertex calc
	frag_out.vert_model = vec4( vec_model, 0 ) + vert;
	frag_out.vert_view = mat_view * frag_out.vert_model;
	gl_Position = mat_perspective * frag_out.vert_view;

	frag_out.vert_light[ 0 ] = mat_light[ 0 ] * frag_out.vert_model;
	frag_out.vert_light[ 1 ] = mat_light[ 1 ] * frag_out.vert_model;
	frag_out.vert_light[ 2 ] = mat_light[ 2 ] * frag_out.vert_model;

	//for( uint i = 0; i < num_cascades; ++i ) {
	//	frag_out.vert_light[ i ] = mat_light[ i ] * frag_out.vert_model;
	//}

	// Tex calc
	frag_out.frag_uvs[ 0 ] = vec3( uvs_face[ id_vert ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex1 );
	frag_out.frag_uvs[ 1 ] = vec3( uvs_face[ id_vert ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex2 );

	// Color calc
	frag_out.frag_norm = norm;
	frag_out.dir_sun = normalize( vec3( pos_sun ) );
	float grad_diffuse = clamp( dot( frag_out.frag_norm, frag_out.dir_sun ), 0.0, 1.0 );
	frag_out.frag_diffuse = diffuse * vec4( grad_diffuse, grad_diffuse, grad_diffuse, 1.0 );

	float grad_fade = max( 0, distance( frag_out.vert_model.xz, pos_camera.xz ) - dist_fade );
	grad_fade = min( 1, grad_fade / ( dist_fade_cutoff - dist_fade ) );
	grad_fade = 1.0 - grad_fade;
	color.a *= grad_fade;

	frag_out.frag_color = color;
}