#version 430

const vec4 offset_face[ 6 ][ 4 ] = {
	{ vec4( 0, 0, 1, 0 ), vec4( 1, 0, 1, 0 ), vec4( 1, 1, 1, 0 ), vec4( 0, 1, 1, 0 ) },
	{ vec4( 1, 0, 0, 0 ), vec4( 0, 0, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 1, 1, 0, 0 ) },

	{ vec4( 1, 0, 1, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 1, 0, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 0, 0, 1, 0 ), vec4( 0, 1, 1, 0 ), vec4( 0, 1, 0, 0 ) }, 

	{ vec4( 1, 1, 0, 0 ), vec4( 0, 1, 0, 0 ), vec4( 0, 1, 1, 0 ), vec4( 1, 1, 1, 0 ) }, 
	{ vec4( 0, 0, 0, 0 ), vec4( 1, 0, 0, 0 ), vec4( 1, 0, 1, 0 ), vec4( 0, 0, 1, 0 ) }
};

const vec3 norm_face[ 6 ] = {
	vec3( 0, 0, 1 ),
	vec3( 0, 0, -1 ),
	vec3( 1, 0, 0 ),
	vec3( -1, 0, 0 ),
	vec3( 0, 1, 0 ),
	vec3( 0, -1, 0 )
};

const vec2 uvs_face[ 4 ] = {
	vec2( 0, 0 ),
	vec2( 1, 0 ),
	vec2( 1, 1 ),
	vec2( 0, 1 )
};

const vec4 scale_face[ 6 ] = {
	vec4( 1, 0, 0, 0 ),
	vec4( 1, 0, 0, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 

	vec4( 0, 0, 1, 0 ), 
	vec4( 0, 0, 1, 0 ), 
};

const vec2 scale_uvs[ 6 ] = {
	vec2( 1, 0 ),
	vec2( 1, 0 ),

	vec2( 1, 0 ),
	vec2( 1, 0 ),

	vec2( 0, 1 ),
	vec2( 0, 1 )
};

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

const uint MAX_CASCADES = 8;
uniform mat4 mat_light[ MAX_CASCADES ];
uniform uint num_cascades;

uniform float dist_fade = 250.0;
uniform float dist_fade_cutoff = 300.0;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in V_DATA {
	uint data1;
	uint data2;
	vec3 vec_model;
} v_in[];

out G_DATA {
	vec4 color;
	vec3 uvs;
	flat vec3 norm;

	float dot_sun;
	flat vec4 diffuse;

	vec4 vert_model;
	vec4 vert_view;
	vec4 vert_light[ MAX_CASCADES ];
} g_out;

void main() {
	vec4 vert;
	vert.x = float( ( v_in[ 0 ].data1 >> 0u ) & 31u );
	vert.y = float( ( v_in[ 0 ].data1 >> 5u ) & 63u );
	vert.z = float( ( v_in[ 0 ].data1 >> 11u ) & 31u );
	vert.w = 1;

	vec4 color;
	color.r = float( ( v_in[ 0 ].data1 >> 16u ) & 31u ) / 31.0f;
	color.g = float( ( v_in[ 0 ].data1 >> 21u ) & 31u ) / 31.0f;
	color.b = float( ( v_in[ 0 ].data1 >> 26u ) & 31u ) / 31.0f;
	color.a = float( ( ( v_in[ 0 ].data1 >> 31u ) & 1u ) + ( ( ( v_in[ 0 ].data2 >> 0u ) & 15u ) << 1u ) ) / 31.0f;

	uint tex1 = ( v_in[ 0 ].data2 >> 4 ) & 1023;

	uint orient = ( v_in[ 0 ].data2 >> 24 ) & 7;
	uint scale = ( v_in[ 0 ].data2 >> 27 ) & 31;

	g_out.vert_model = vec4( v_in[ 0 ].vec_model, 0 ) + vert;
	g_out.vert_view = mat_view * g_out.vert_model;

	g_out.norm = norm_face[ orient ];

	g_out.dot_sun = dot( g_out.norm, normalize( pos_sun.xyz ) );

	float grad_diffuse = clamp( g_out.dot_sun, 0.0, 1.0 );
	g_out.diffuse = diffuse * vec4( grad_diffuse, grad_diffuse, grad_diffuse, 1.0 );

	float grad_fade = max( 0, distance( g_out.vert_model.xz, pos_camera.xz ) - dist_fade );
	grad_fade = min( 1, grad_fade / ( dist_fade_cutoff - dist_fade ) );
	grad_fade = 1.0 - grad_fade;

	color.a *= grad_fade;
	g_out.color = color;

	g_out.vert_model = vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 0 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale );
	g_out.vert_view = mat_view * g_out.vert_model;
	g_out.vert_light[ 0 ] = mat_light[ 0 ] * g_out.vert_model;
	g_out.vert_light[ 1 ] = mat_light[ 1 ] * g_out.vert_model;
	g_out.vert_light[ 2 ] = mat_light[ 2 ] * g_out.vert_model;
	g_out.vert_light[ 3 ] = mat_light[ 3 ] * g_out.vert_model;
	gl_Position = mat_perspective * g_out.vert_view;
	g_out.uvs = vec3( uvs_face[ 0 ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex1 );
	EmitVertex();

	g_out.vert_model = vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 1 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale );
	g_out.vert_view = mat_view * g_out.vert_model;
	g_out.vert_light[ 0 ] = mat_light[ 0 ] * g_out.vert_model;
	g_out.vert_light[ 1 ] = mat_light[ 1 ] * g_out.vert_model;
	g_out.vert_light[ 2 ] = mat_light[ 2 ] * g_out.vert_model;
	g_out.vert_light[ 3 ] = mat_light[ 3 ] * g_out.vert_model;
	gl_Position = mat_perspective * g_out.vert_view;
	g_out.uvs = vec3( uvs_face[ 1 ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex1 );
	EmitVertex();

	g_out.vert_model = vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 3 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale );
	g_out.vert_view = mat_view * g_out.vert_model;
	g_out.vert_light[ 0 ] = mat_light[ 0 ] * g_out.vert_model;
	g_out.vert_light[ 1 ] = mat_light[ 1 ] * g_out.vert_model;
	g_out.vert_light[ 2 ] = mat_light[ 2 ] * g_out.vert_model;
	g_out.vert_light[ 3 ] = mat_light[ 3 ] * g_out.vert_model;
	gl_Position = mat_perspective * g_out.vert_view;
	g_out.uvs = vec3( uvs_face[ 3 ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex1 );
	EmitVertex();

	g_out.vert_model = vec4( v_in[ 0 ].vec_model, 0 ) + vert + offset_face[ orient ][ 2 ] * ( vec4( 1, 1, 1, 0 ) + scale_face[ orient ] * scale );
	g_out.vert_view = mat_view * g_out.vert_model;
	g_out.vert_light[ 0 ] = mat_light[ 0 ] * g_out.vert_model;
	g_out.vert_light[ 1 ] = mat_light[ 1 ] * g_out.vert_model;
	g_out.vert_light[ 2 ] = mat_light[ 2 ] * g_out.vert_model;
	g_out.vert_light[ 3 ] = mat_light[ 3 ] * g_out.vert_model;
	gl_Position = mat_perspective * g_out.vert_view;
	g_out.uvs = vec3( uvs_face[ 2 ] * ( vec2( 1, 1 ) + scale_uvs[ orient ] * scale ), tex1 );
	EmitVertex();
	
	EndPrimitive();
}