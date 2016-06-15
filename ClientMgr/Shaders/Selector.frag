#version 430

layout( std140 ) uniform mvp_matrices {
	vec4 pos_camera;
	mat4 mat_world;
	mat4 mat_perspective;
	mat4 mat_ortho;
	mat4 mat_view;
	float time_game;
};

uniform sampler2DArray frag_sampler;

in vec4 frag_color;
in vec3 frag_uv;

out vec4 out_color;

void main() {
	out_color = texture( frag_sampler, frag_uv ) * frag_color;
}