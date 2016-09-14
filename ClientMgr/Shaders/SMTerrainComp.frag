#version 430

uniform sampler2DArray frag_sampler;

out FRAG_OUT {
	vec3 dir_sun;
	vec4 frag_diffuse;

	vec3 frag_uvs[ 2 ];
	vec4 frag_color;
	vec3 frag_norm;

	vec4 vert_model;
	vec4 vert_view;
	vec4 vert_light[ MAX_CASCADES ];
} frag_out;

out vec4 out_color;

void main() {
	out_color = texture( frag_sampler, frag_out.frag_uvs[ 0 ] ) * frag_out.frag_color;
}