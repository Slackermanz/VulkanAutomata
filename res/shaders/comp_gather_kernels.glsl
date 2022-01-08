#version 460

const int WG_SIZE = 32;

layout(local_size_x = WG_SIZE, local_size_y = WG_SIZE) in;

layout( set = 0, binding = 0, r8 ) uniform image2D img_in;
layout( set = 0, binding = 1, r8 ) uniform image3D kernel_out;
layout(	set = 0, binding = 2	 ) uniform unform_buffer {
	uint v0;  uint v1;  uint v2;  uint v3;	uint v4;  uint v5;  uint v6;  uint v7;
	uint v8;  uint v9;  uint v10; uint v11;	uint v12; uint v13; uint v14; uint v15;
	uint v16; uint v17; uint v18; uint v19;	uint v20; uint v21; uint v22; uint v23;
	uint v24; uint v25; uint v26; uint v27;	uint v28; uint v29; uint v30; uint v31;
	uint v32; uint v33; uint v34; uint v35;	uint v36; uint v37; uint v38; uint v39;
	uint v40; uint v41; uint v42; uint v43;	uint v44; uint v45; uint v46; uint v47;
	uint v48; uint v49; uint v50; uint v51;	uint v52; uint v53; uint v54; uint v55;
	uint v56; uint v57; uint v58; uint v59;	uint v60; uint v61; uint v62; uint v63; } ub;

vec4 nb_load( ivec2 of ) {
	return imageLoad( img_in, ( ivec2(gl_GlobalInvocationID.xy) + of + ivec2(512,512) ) & ( ivec2(512,512) - 1 ) ); }

void main() {

	for(int i = -int(ub.v0); i < int(ub.v0)+1; i++) {
		for(int j = -int(ub.v0); j < int(ub.v0)+1; j++) {
			imageStore( kernel_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (i+ub.v0)+(j+ub.v0)*(ub.v0*2+1)), nb_load(ivec2(i,j)) ); } }

//	imageStore( kernel_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), nb_load(ivec2(0,0)) );

}
