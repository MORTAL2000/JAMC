#version 430

uniform sampler2D frag_sampler;

in vec4 frag_color;
in vec2 frag_uv;

out vec4 out_color;

void main() {
	out_color = texture2D( frag_sampler, frag_uv ) * frag_color;
}