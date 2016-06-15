#version 430

uniform sampler2DArray frag_sampler;
uniform float idx_layer;

in vec4 frag_color;
in vec3 frag_uv;

out vec4 out_color;

void main() {
	out_color = texture( frag_sampler, vec3( frag_uv.xy, idx_layer ) ) * frag_color;
}