#version 430 core

uniform sampler2DArray frag_sampler;

in G_DATA {
	vec4 color;
	vec3 uvs;
} g_in;

layout(location = 0) out float frag_depth;

void main() {
	vec4 color = texture( frag_sampler, g_in.uvs ) * g_in.color;

	if( color.a < 0.8 ) {
		discard;
	}

	frag_depth = gl_FragCoord.z;
}