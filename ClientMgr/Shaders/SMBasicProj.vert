#version 430

layout( std140 ) uniform mvp_matrices {
	vec4 pos_camera;
	mat4 mat_world;
	mat4 mat_perspective;
	mat4 mat_ortho;
	mat4 mat_view;
	float time_game;
};

layout( location = 0 ) in vec4 vert;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec3 norm;
layout( location = 3 ) in vec3 uv;
layout( location = 4 ) in uint id;
layout( location = 5 ) in mat4 mat_model;
layout( location = 9 ) in mat3 mat_norm;

out vec3 frag_uvs;
out vec4 frag_color;

void main() {
	gl_Position = mat_perspective * mat_view * mat_model * vert;
	frag_uvs = uv;
	frag_color = vec4( color.r / 255, color.g / 255.0, color.b / 255.0, color.a / 255.0 );
}