#version 430

uniform sampler2DArray frag_sampler;

in vec3 frag_uvs;
in vec4 frag_color;

out vec4 out_color;

void main() {
	out_color = texture( frag_sampler, frag_uvs ) * frag_color;
}