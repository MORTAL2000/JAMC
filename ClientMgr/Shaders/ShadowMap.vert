#version 430 core

layout( location = 0 ) in vec4 vert;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec3 norm;
layout( location = 3 ) in vec3 uv;

uniform mat4 mat_light;
uniform mat4 mat_model;

out vec3 frag_uv;
out vec4 frag_color;

void main() {
	frag_uv = uv;
	gl_Position = mat_light * mat_model * vert;
}