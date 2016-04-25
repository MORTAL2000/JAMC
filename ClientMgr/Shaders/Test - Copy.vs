#version 430

uniform vec3 pos_camera;
uniform mat4 mat_view;
uniform mat4 mat_perspective;

layout( location = 0 ) in vec4 vert;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec3 norm;
layout( location = 3 ) in vec2 uv;

out vec4 frag_color;
out vec3 frag_norm;
out vec2 frag_uv;
out vec3 view_vert;
out vec3 diff_camera;

void main() {
	gl_Position = mat_perspective * mat_view * vert;
	frag_color = color;
	frag_norm = normalize( norm );
	view_vert = vec3( mat_view * vert );
	diff_camera = vec3( vert ) - pos_camera;
	frag_uv = uv;
}