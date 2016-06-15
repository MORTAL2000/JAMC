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

uniform mat4 mat_model;
uniform mat3 mat_norm;
uniform mat4 mat_light;

out vec4 frag_diffuse;
out vec4 frag_color;
out vec3 frag_uv;
out vec4 vert_model;
out vec3 frag_norm;
out vec4 frag_vert_light;
out vec3 diff_sun;

float grad_diffuse;

void main() {
	vert_model = mat_model * vert;
	frag_norm = mat_norm * norm;

	diff_sun = normalize( vec3( pos_sun - pos_camera ) );
	grad_diffuse = clamp( dot(  frag_norm, diff_sun ), 0.0, 1.0 );
	frag_diffuse = diffuse * vec4( grad_diffuse, grad_diffuse, grad_diffuse, 1.0 );

	frag_vert_light = mat_light * mat_model * vec4( vert.xyz, 1.0 );

	gl_Position = mat_perspective * mat_view * vert_model;
	frag_color = color;

	frag_uv = uv;
}