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

layout( location = 0 ) in vec4 vert;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec3 norm;
layout( location = 3 ) in vec3 uv;

uniform mat4 mat_model;
uniform mat3 mat_norm;
uniform mat4 mat_light;
uniform vec4 entity_color;

out vec4 frag_color;
out vec3 frag_uv;

vec4 vert_model;
vec3 diff_sun;
vec3 diff_emitter;
float len_emitter;
vec3 norm_emitter;
vec3 norm_model;
float grad_diffuse;
float grad_emitter;

float size_torch = 25.0;

void main() {
	vert_model = mat_model * vert;
	diff_sun = normalize( vec3( pos_sun - vert_model ) );
	norm_model = normalize( mat_norm * norm );

	grad_diffuse = clamp( dot( norm_model, diff_sun ), 0.0, 1.0 );

	gl_Position = mat_perspective * mat_view * vert_model;
	frag_color = ambient;
	frag_color += diffuse * vec4( grad_diffuse, grad_diffuse, grad_diffuse, 1.0 );
	
	// Camera Emitter
	diff_emitter = vec3( pos_camera - vert_model );
	len_emitter = length( diff_emitter );

	if( len_emitter <= size_torch ) {
		norm_emitter = normalize( diff_emitter );
		grad_emitter = 
			clamp( dot( norm_model, norm_emitter ), 0.0, 1.0 ) * 
			clamp( 1.0 - ( len_emitter * len_emitter ) / ( size_torch * size_torch ), 0.0, 1.0 );
		frag_color += 
			vec4( 1.0f, 0.5, 0.0, 0.0 ) *
			vec4( grad_emitter, grad_emitter, grad_emitter, 1.0 );
	}

	// World Emitters
	for( int i = 0; i < num_emitters.x; i++ ) {
		diff_emitter = vec3( list_pos[ i ] - vert_model );
		len_emitter = length( diff_emitter );
		if( len_emitter <= list_radius[ i ].x ) {
			norm_emitter = normalize( diff_emitter );
			grad_emitter = 
				clamp( dot( norm_model, norm_emitter ), 0.0, 1.0 ) * 
				clamp( 1.0 - len_emitter / list_radius[ i ].x, 0.0, 1.0 );
			frag_color += 
				list_color[ i ] * 
				vec4( grad_emitter, grad_emitter, grad_emitter, 1.0 );
		}
	}
	
	frag_color *= entity_color;
	frag_uv = uv;
}