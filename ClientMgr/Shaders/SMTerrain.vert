#version 430

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

layout( location = 0 ) in vec4 vert;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec3 norm;
layout( location = 3 ) in vec3 uv;

layout( location = 4 ) in uint id;
layout( location = 5 ) in mat4 mat_model;
layout( location = 9 ) in mat3 mat_norm;

uniform float dist_fade = 250.0;
uniform float dist_fade_cutoff = 300.0;

const uint MAX_CASCADES = 8;
uniform mat4 mat_light[ MAX_CASCADES ];
out vec4 frag_vert_light[ MAX_CASCADES ];
uniform uint num_cascades;

out vec4 frag_diffuse;
out vec4 frag_color;
out vec3 frag_norm;
out vec3 frag_uv;

out vec4 vert_model;
out vec4 vert_view;

out vec3 diff_sun;

float grad_diffuse;
float grad_fade;

void main() {
	vert_model = mat_model * vert;
	vert_view = mat_view * vert_model;
	frag_norm = mat_norm * norm;

	grad_fade = max( 0, distance( vert_model.xz, pos_camera.xz ) - dist_fade );
	grad_fade = min( 1, grad_fade / ( dist_fade_cutoff - dist_fade ) );
	grad_fade = 1.0 - grad_fade;

	diff_sun = normalize( vec3( pos_sun ) );
	grad_diffuse = clamp( dot( frag_norm, diff_sun ), 0.0, 1.0 );
	frag_diffuse = diffuse * vec4( grad_diffuse, grad_diffuse, grad_diffuse, 1.0 );

	for( uint i = 0; i < num_cascades; ++i ) {
		frag_vert_light[ i ] = mat_light[ i ] * mat_model * vec4( vert.xyz, 1.0 );
	}

	gl_Position = mat_perspective * mat_view * vert_model;
	frag_color = vec4( color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0 );
	frag_color.a *= grad_fade;

	frag_uv = uv;
}