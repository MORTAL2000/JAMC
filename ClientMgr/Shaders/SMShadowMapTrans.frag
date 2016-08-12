#version 430 core

uniform sampler2DArray frag_sampler;

in vec3 frag_uv;
in vec4 frag_color;

layout(location = 0) out float frag_depth;

void main() {
	vec4 color = texture( frag_sampler, frag_uv ) * frag_color;

	if( color.a < 0.8 ) {
		discard;
	}

	frag_depth = gl_FragCoord.z;
}