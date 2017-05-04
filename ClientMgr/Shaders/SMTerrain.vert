#version 430

// In Data
layout( location = 0 ) in uint data1;
layout( location = 1 ) in uint data2;
layout( location = 2 ) in uint id;
layout( location = 3 ) in vec3 vec_model;

out V_DATA {
	uint data1;
	uint data2;
	vec3 vec_model;
} v_out;

void main() {
	v_out.data1 = data1;
	v_out.data2 = data2;
	v_out.vec_model = vec_model;
}