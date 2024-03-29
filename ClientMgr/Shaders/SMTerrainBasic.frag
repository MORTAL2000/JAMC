#version 430

/*
uniform sampler2DArray frag_sampler;

layout( std140 ) uniform mvp_matrices {
	vec4 pos_camera;
	mat4 mat_world;
	mat4 mat_perspective;
	mat4 mat_ortho;
	mat4 mat_view;
	float time_game;
};

const int max_emitters = 8;
layout( std140 ) uniform light_data {
	vec4 pos_sun;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	ivec4 num_emitters;
	vec4 list_pos[ max_emitters ];
	vec4 list_color[ max_emitters ];
	vec4 list_radius[ max_emitters ];
};

in FRAG_OUT {
	vec3 dir_sun;
	vec4 frag_diffuse;

	vec3 frag_uvs[ 2 ];
	vec4 frag_color;
	vec3 frag_norm;

	vec4 vert_model;
	vec4 vert_view;
} frag_out;

out vec4 out_color;

const float size_torch = 25.0;

void main() {
	out_color = ambient + frag_out.frag_diffuse;

	vec3 diff_emitter = vec3( pos_camera - frag_out.vert_model );
	float len_emitter = length( diff_emitter );

	if( len_emitter <= size_torch ) {
		vec3 norm_emitter = normalize( diff_emitter );
		float grad_emitter = 
			clamp( dot( frag_out.frag_norm, norm_emitter ), 0.0, 1.0 ) * 
			clamp( 1.0 - ( len_emitter * len_emitter ) / ( size_torch * size_torch ), 0.0, 1.0 );
		out_color += 
			vec4( 1.0f, 0.5, 0.0, 0.0 ) * 
			vec4( grad_emitter, grad_emitter, grad_emitter, 1.0 );
	}

	out_color *= texture( frag_sampler, frag_out.frag_uvs[ 0 ] ) * frag_out.frag_color;
}
*/

uniform sampler2DArray frag_sampler;

layout( std140 ) uniform mvp_matrices {
	vec4 pos_camera;
	mat4 mat_world;
	mat4 mat_perspective;
	mat4 mat_ortho;
	mat4 mat_view;
	float time_game;
};

const int max_emitters = 8;
layout( std140 ) uniform light_data {
	vec4 pos_sun;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	ivec4 num_emitters;
	vec4 list_pos[ max_emitters ];
	vec4 list_color[ max_emitters ];
	vec4 list_radius[ max_emitters ];
};

in G_DATA {
	vec4 color;
	vec3 uvs;
	flat vec3 norm;

	float dot_sun;
	flat vec4 diffuse;

	vec4 vert_model;
	vec4 vert_view;
} g_in;


const float size_torch = 25.0;

out vec4 out_color;

void main() {
	out_color = ambient;
	out_color += g_in.diffuse;

	vec3 diff_emitter = pos_camera.xyz - g_in.vert_model.xyz;
	float len_emitter = length( diff_emitter );

	if( len_emitter <= size_torch ) {
		vec3 norm_emitter = normalize( diff_emitter );
		float grad_emitter = 
			clamp( dot( g_in.norm, norm_emitter ), 0.0, 1.0 ) * 
			clamp( 1.0 - ( len_emitter * len_emitter ) / ( size_torch * size_torch ), 0.0, 1.0 );
		out_color += 
			vec4( 1.0f, 0.5, 0.0, 0.0 ) * 
			vec4( grad_emitter, grad_emitter, grad_emitter, 1.0 );
	}

	out_color *= texture( frag_sampler, g_in.uvs ) * g_in.color;
}