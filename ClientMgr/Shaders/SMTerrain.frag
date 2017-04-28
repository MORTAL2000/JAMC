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

in G_DATA {
	vec4 color;
	vec3 uvs;
	flat vec3 norm;

	float dot_sun;
	flat vec4 diffuse;

	vec4 vert_model;
	vec4 vert_view;
	vec4 vert_light[ MAX_CASCADES ];
} g_in;

float shadow_calc( ) {
	float delta_view = length( g_in.vert_view.z );
	uint idx_shadow = 0;

	if( delta_view > 512.0 ) {
		return 0.0;
	}
	else if( delta_view > 256.0 ) {
		idx_shadow = 3;
	}
	else if( delta_view > 64.0 ) {
		idx_shadow = 2;
	}
	else if( delta_view > 16.0 ) {
		idx_shadow = 1;
	}
	else {
		idx_shadow = 0;
	}

	vec3 proj_coord = g_in.vert_light[ idx_shadow ].xyz / g_in.vert_light[ idx_shadow ].w;
	proj_coord = proj_coord * 0.5 + 0.5;

	if( proj_coord.z > 1.0 ) {
		return 0.0;
	}

	vec2 size_texel = 1.0 / textureSize( frag_shadow[ idx_shadow ], 0 );

	float depth_curr = proj_coord.z;
	float bias = max( bias_h * ( 1.0 - g_in.dot_sun ), bias_l );

	float shadow = 0.0;

	float depth_pcf = texture( frag_shadow[ idx_shadow ], proj_coord.xy ).z;
	shadow += depth_curr - bias > depth_pcf ? 1.0 : 0.0;

	if( uint( proj_coord.x * 8192 ) % 2 == uint( proj_coord.y * 8192 ) % 2 ) {
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

out vec4 out_color;

void main() {
	float shadow = 1 - shadow_calc( );
	//float shadow = 1;

	out_color = ambient;
	out_color += g_in.diffuse * shadow;

	vec3 diff_emitter = pos_camera.xyz - g_in.vert_model.xyz;
	float len_emitter = length( diff_emitter );

	//if( len_emitter <= size_torch ) {
		vec3 norm_emitter = normalize( diff_emitter );
		float grad_emitter = 
			clamp( dot( g_in.norm, norm_emitter ), 0.0, 1.0 ) * 
			clamp( 1.0 - ( len_emitter * len_emitter ) / ( size_torch * size_torch ), 0.0, 1.0 );
		out_color += 
			vec4( 1.0f, 0.5, 0.0, 0.0 ) * 
			vec4( grad_emitter, grad_emitter, grad_emitter, 1.0 );
	//}

	out_color *= texture( frag_sampler, g_in.uvs ) * g_in.color;
}