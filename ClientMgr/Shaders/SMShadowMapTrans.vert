#version 430 core

layout( location = 0 ) in vec4 vert;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec3 norm;
layout( location = 3 ) in vec3 uv;
layout( location = 4 ) in uint id;
layout( location = 5 ) in mat4 mat_model;
layout( location = 9 ) in mat3 mat_norm;

uniform mat4 mat_light;

out vec3 frag_uv;
out vec4 frag_color;

void main() {
	frag_uv = uv;
	frag_color = vec4( color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0 );
	gl_Position = mat_light * mat_model * vert;
}