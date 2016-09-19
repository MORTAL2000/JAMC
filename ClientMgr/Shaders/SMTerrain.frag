#version 430

uniform sampler2DArray frag_sampler;

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

// Cascade Data
const uint MAX_CASCADES = 8;
uniform sampler2D frag_shadow[ MAX_CASCADES ];
uniform float depth_cascades[ MAX_CASCADES ];
uniform uint num_cascades;
uniform float bias_l;
uniform float bias_h;

const vec2 pcf_lookup[ 4 ] = {
	vec2( -1, -1 ),
	vec2( 1, -1 ),
	vec2( 1, 1 ),
	vec2( -1, 1 )
};

in FRAG_OUT {
	//flat vec3 dir_sun;
	float dot_sun_norm;
	flat vec4 frag_diffuse;

	vec3 frag_uvs[ 2 ];
	vec4 frag_color;
	flat vec3 frag_norm;

	vec4 vert_model;
	vec4 vert_view;
	vec4 vert_light[ MAX_CASCADES ];
} frag_out;

out vec4 out_color;

float shadow_calc( ) {
	float delta_view = length( frag_out.vert_view.z );
	uint idx_shadow = 0;

	if( delta_view > 256.0f ) {
		return 0.0;
	}
	else if( delta_view > 64.0f ) {
		idx_shadow = 2;
	}
	else if( delta_view > 16.0f ) {
		idx_shadow = 1;
	}
	else {
		idx_shadow = 0;
	}

	vec3 proj_coord = frag_out.vert_light[ idx_shadow ].xyz / frag_out.vert_light[ idx_shadow ].w;
	proj_coord = proj_coord * 0.5 + 0.5;

	if( proj_coord.z > 1.0 ) {
		return 0.0;
	}

	vec2 size_texel = 1.0 / textureSize( frag_shadow[ idx_shadow ], 0 );

	float depth_curr = proj_coord.z;
	float bias = max( bias_h * ( 1.0 - frag_out.dot_sun_norm ), bias_l );

	float shadow = 0.0;

	float depth_pcf = texture( frag_shadow[ idx_shadow ], proj_coord.xy ).z;
	shadow += depth_curr - bias > depth_pcf ? 1.0 : 0.0;

	if( uint( proj_coord.x * 4096 ) % 2 == uint( proj_coord.y * 4096 ) % 2 ) {
		depth_pcf = texture( frag_shadow[ idx_shadow ], proj_coord.xy + pcf_lookup[ 0 ] * size_texel ).z; 
		shadow += depth_curr - bias > depth_pcf ? 1.0 : 0.0;

		depth_pcf = texture( frag_shadow[ idx_shadow ], proj_coord.xy + pcf_lookup[ 2 ] * size_texel ).z; 
		shadow += depth_curr - bias > depth_pcf ? 1.0 : 0.0;
	}
	else {
		depth_pcf = texture( frag_shadow[ idx_shadow ], proj_coord.xy + pcf_lookup[ 1 ] * size_texel ).z; 
		shadow += depth_curr - bias > depth_pcf ? 1.0 : 0.0;

		depth_pcf = texture( frag_shadow[ idx_shadow ], proj_coord.xy + pcf_lookup[ 3 ] * size_texel ).z; 
		shadow += depth_curr - bias > depth_pcf ? 1.0 : 0.0;
	}

	return shadow / 3.0f;
}

const float size_torch = 25.0;

void main() {
	float shadow = 1 - shadow_calc( );

	out_color = ambient;
	out_color += frag_out.frag_diffuse * shadow;

	vec3 diff_emitter = pos_camera.xyz - frag_out.vert_model.xyz;
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