#version 430

uniform vec3 pos_light;
uniform sampler2D frag_sample;

uniform float fade_max;
uniform float fade_min;

uniform vec4 color_ambient;
uniform vec4 color_diffuse;

uniform float time;

in vec4 frag_color;
in vec3 frag_norm;
in vec2 frag_uv;
in vec3 view_vert;
in vec3 diff_camera;

out vec4 out_color;

vec4 color_light = vec4( 0.855, 0.647, 0.125, 1.0 );
float len_light = 50.0;

void main() {
	float len_camera = length( diff_camera );
	float len_camera_2d = length( diff_camera.xz );
	vec3 diff_vert = pos_light - view_vert;
	vec4 color_texture = texture2D( frag_sample, frag_uv );

	vec4 frag_ambient = color_ambient * color_texture * frag_color;
	out_color = frag_ambient;

	float grad_diffuse = clamp( max( dot( frag_norm, normalize( diff_vert ) ), 0.0 ), 0.0, 1.0 );
	vec4 frag_diffuse = ( color_diffuse * grad_diffuse ) * color_texture * frag_color;
	out_color += frag_diffuse;
	
	float grad_light = 1.0 - clamp( len_camera, 0.0, len_light ) / len_light;
	vec4 frag_light = color_light * color_texture * frag_color * grad_light;
	out_color += frag_light;

	float grad_fade = clamp(  len_camera_2d - fade_min, 0.0, ( fade_max - fade_min ) ) / ( fade_max - fade_min );
	grad_fade = 1.0 - grad_fade;
	out_color.a *= grad_fade;
}