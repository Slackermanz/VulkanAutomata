#version 460

const int WG_SIZE = 32;

layout(local_size_x = WG_SIZE, local_size_y = WG_SIZE) in;

layout(set = 0, binding = 0, r8) uniform image3D kernel_in;
layout(set = 0, binding = 1, r8) uniform image2D img_out;

vec4 nb_load( ivec2 of ) {
	return imageLoad( img_in, ( ivec2(gl_GlobalInvocationID.xy) + of + ivec2(512,512) ) & ( ivec2(512,512) - 1 ) ); }

void main() {

	for(int i = -1; i < 2; i++) {
		for(int j = -1; j < 2; j++) {
			imageStore( kernel_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (i+1)+(j+1)*3), nb_load(ivec2(i,j)) ); } }

}
