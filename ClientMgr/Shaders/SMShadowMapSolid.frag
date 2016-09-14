#version 430 core

uniform sampler2DArray frag_sampler;

layout(location = 0) out float frag_depth;

void main() {
	frag_depth = gl_FragCoord.z;
}