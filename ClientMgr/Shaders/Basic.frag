#version 430

uniform sampler2D frag_sampler;

in vec3 frag_uvs;
in vec4 frag_color;

out vec4 out_color;

void main() {
	out_color = texture2D( frag_sampler, vec2( frag_uvs.xy ) ) * frag_color;
}