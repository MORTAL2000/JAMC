#version 430

layout( std140 ) uniform mvp_matrices {
	vec4 pos_camera;
	mat4 mat_world;
	mat4 mat_perspective;
	mat4 mat_ortho;
	mat4 mat_view;
	float time_game;
};

const int max_emitters = 128;
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

uniform sampler2DArray frag_sampler;
uniform sampler2D frag_shadow;
uniform float bias_l;
uniform float bias_h;

in vec4 frag_diffuse;
in vec4 frag_color;
in vec3 frag_uv;
in vec4 vert_model;
in vec3 frag_norm;
in vec4 frag_vert_light;
in vec3 diff_sun;

out vec4 out_color;

vec3 diff_emitter;
float len_emitter;
vec3 norm_emitter;
float grad_emitter;
float grad_shadow;

vec3 proj_coord;
//float depth_close;
float depth_curr;
float depth_pcf;
float bias;
float shadow;
vec2 size_texel;
vec2 offset;

float shadow_calc( vec4 vert_light ) {
	proj_coord = vert_light.xyz / vert_light.w;
	proj_coord = proj_coord * 0.5 + 0.5;

	if( proj_coord.z > 1.0 ) {
		return 0.0;
	}

	depth_curr = proj_coord.z;
	bias = max( bias_h * ( 1.0 - dot( frag_norm, diff_sun ) ), bias_l );

	shadow = 0.0;
	size_texel = 1.0 / textureSize( frag_shadow, 0 );

	for( offset.x = -1; offset.x <= 1; ++offset.x ) {
		for( offset.y = -1; offset.y <= 1; ++offset.y ) {
			depth_pcf = texture( frag_shadow, proj_coord.xy + offset * size_texel ).z; 
			shadow += depth_curr - bias > depth_pcf ? 1.0 : 0.0;        
		}
	}

	return shadow / 9.0;
}

float size_torch = 25.0;

void main() {
	float shadow = shadow_calc( frag_vert_light );

	out_color = ambient;
	out_color += vec4( vec3( frag_diffuse.xyz ) * ( 1.0 - shadow ), 0.0 );

	diff_emitter = vec3( pos_camera - vert_model );
	len_emitter = length( diff_emitter );

	if( len_emitter <= size_torch ) {
		norm_emitter = normalize( diff_emitter );
		grad_emitter = 
			clamp( dot( frag_norm, norm_emitter ), 0.0, 1.0 ) * 
			clamp( 1.0 - ( len_emitter * len_emitter ) / ( size_torch * size_torch ), 0.0, 1.0 );
		out_color += 
		vec4( 1.0f, 0.5, 0.0, 0.0 ) * 
		vec4( grad_emitter, grad_emitter, grad_emitter, 1.0 );
	}

	for( int i = 0; i < num_emitters.x; i++ ) {
		diff_emitter = vec3( list_pos[ i ] - vert_model );
		len_emitter = length( diff_emitter );

		if( len_emitter <= list_radius[ i ].x ) {
			norm_emitter = normalize( diff_emitter );
			grad_emitter = 
				clamp( dot( frag_norm, norm_emitter ), 0.0, 1.0 ) * 
				clamp( 1.0 - len_emitter / list_radius[ i ].x, 0.0, 1.0 );
			out_color += list_color[ i ] * vec4( grad_emitter, grad_emitter, grad_emitter, 1.0 );
		}
	}

	out_color = texture( frag_sampler, frag_uv ) * out_color * frag_color;
}