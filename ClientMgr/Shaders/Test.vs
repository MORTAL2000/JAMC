#version 430

uniform mat4 mat_view;
uniform mat4 mat_perspective;

uniform vec3 pos_camera;
uniform vec3 pos_light;

uniform float fade_max;
uniform float fade_min;

uniform vec4 color_ambient;
uniform vec4 color_diffuse;

uniform float time;

const int max_emitters = 128;

uniform float num_emitters;
uniform vec3 emitters_pos[ max_emitters ];
uniform vec3 emitters_color[ max_emitters ];
uniform float emitters_radius[ max_emitters ];

layout( location = 0 ) in vec4 vert;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec3 norm;
layout( location = 3 ) in vec2 uv;

out vec4 frag_color;
out vec2 frag_uv;

vec4 color_light = vec4( 0.855, 0.647, 0.125, 1.0 );
float len_light = 50.0;

void main() {
	gl_Position = mat_perspective * mat_view * vert;

	vec4 vert_view = mat_view * vert;
	vec3 vert_diff = pos_light - vec3( vert_view );
	float len_camera = length( vert_view );
	float len_camera_2d = length( vert_view.xz );
 
	frag_color = color_ambient;

	float grad_diffuse = clamp( max( dot( norm, normalize( vert_diff ) ), 0.0 ), 0.0, 1.0 );
	frag_color += color_diffuse * grad_diffuse;

	/*{
		float grad_light = 1.0 - clamp( len_camera, 0.0, len_light ) / len_light;
		color_light = vec4( 
			0.5 + sin( len_camera + time / 100.0f ) * 0.5,
			0.5 + cos( len_camera + time / 100.0f ) * 0.5,
			0.5 + sin( len_camera + time / 100.0f + 3.14 ) * 0.5,
			1.0
		);
		frag_color += color_light * grad_light;
	}*/

	for( int i = 0; i < num_emitters; i++ ) {
		float grad_light = 1.0 - clamp( distance( vec3( vert ), emitters_pos[ i ] ), 0.0, emitters_radius[ i ] ) / emitters_radius[ i ];
		frag_color += vec4( emitters_color[ i ], 0.0 ) * grad_light;
	}

	frag_color *= color;

	float grad_fade = 1.0 - clamp( len_camera_2d - fade_min, 0.0, ( fade_max - fade_min ) ) / ( fade_max - fade_min );
	frag_color.a = grad_fade;

	frag_uv = uv;
}