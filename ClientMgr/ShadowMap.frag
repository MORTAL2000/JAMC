#version 430 core

uniform sampler2DArray frag_sampler;

in vec3 frag_uv;

//layout(location = 0) out float frag_depth;

void main() {
	gl_FragDepth = 300.0f;
	/*vec4 color = texture( frag_sampler, frag_uv );
	if( color.a < 0.5 ) {
		discard;
	}
	else {
		gl_FragDepth = gl_FragCoord.z;
	}*/
}