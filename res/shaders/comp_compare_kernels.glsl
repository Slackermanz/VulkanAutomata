#version 460

const int WG_SIZE = 32;

layout(local_size_x = WG_SIZE, local_size_y = WG_SIZE) in;

layout( set = 0, binding = 0, r8 ) uniform image2D img_in;
layout( set = 0, binding = 1, r8 ) uniform image2D img_out;
layout(	set = 0, binding = 2	 ) uniform unform_buffer {
	uint v0;  uint v1;  uint v2;  uint v3;	uint v4;  uint v5;  uint v6;  uint v7;
	uint v8;  uint v9;  uint v10; uint v11;	uint v12; uint v13; uint v14; uint v15;
	uint v16; uint v17; uint v18; uint v19;	uint v20; uint v21; uint v22; uint v23;
	uint v24; uint v25; uint v26; uint v27;	uint v28; uint v29; uint v30; uint v31;
	uint v32; uint v33; uint v34; uint v35;	uint v36; uint v37; uint v38; uint v39;
	uint v40; uint v41; uint v42; uint v43;	uint v44; uint v45; uint v46; uint v47;
	uint v48; uint v49; uint v50; uint v51;	uint v52; uint v53; uint v54; uint v55;
	uint v56; uint v57; uint v58; uint v59;	uint v60; uint v61; uint v62; uint v63; } ub;

void main() {

/*	float diff2 = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			float self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1)) ).r;
			float diff = 0.0;
			for(int i = 0; i < 512; i++) {
				for(int j = 0; j < 512; j++) {
					diff += abs(self - imageLoad( img_in, ivec3(i, j, (kx+r)+(ky+r)*(r*2+1)) ).r) / (512.0*512.0); } }
			diff2 += diff / ((r*2+1)*(r*2+1)); } }
//			imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+1)+(ky+1)*3), vec4(diff,0.0,0.0,0.0) ); } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(diff2,0.0,0.0,0.0) );/**/

//	No Operation
/*	float self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0) ).r;
	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(self,0.0,0.0,0.0) );/**/

//	Per-Pixel Frequency
/*	float self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ((r*2+1)*(r*2+1))/2) ).r;
	float diff = 0.0;
	for(int i = 0; i < 512; i++) {
		for(int j = 0; j < 512; j++) {
			diff += abs(self - imageLoad( img_in, ivec3(i, j, ((r*2+1)*(r*2+1))/2) ).r) / (512.0*512.0); } }
	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(diff,0.0,0.0,0.0) );/**/

//	Edge Detection
/*	float self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ((r*2+1)*(r*2+1))/2 ) ).r;
	float diff = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			diff += abs(self - imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1)) ).r) / ((r*2+1)*(r*2+1)*0.5); } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(diff,0.0,0.0,0.0) );/**/

//	Contrast
/*	float valu = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			valu += (imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r - 0.5) * 2.0; } }
	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4((valu+1.0)/2.0,0.0,0.0,0.0) );/**/

//	Grab Kernel at location
/*	float self = 0.0;
	float diff = 0.0;
	float rslt = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r;
			diff = imageLoad( img_in, ivec3(30, 300, (kx+r)+(ky+r)*(r*2+1)) ).r;
			rslt+= abs(self-diff)/((r*2+1)*(r*2+1)*0.5); } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(rslt,0.0,0.0,0.0) );/**/

//	Grab Kernel at location
/*	int rd = int(ub.v0);
	int xx = int(ub.v1)%512;
	int yy = int(ub.v2)%512;
	float self = 0.0;
	float diff = 0.0;
	float rslt = 0.0;

	float totl = (rd*2+1)*(rd*2+1);

	float self2 = 0.0;
	float diff2 = 0.0;
	for(int kx = -rd; kx < rd+1; kx++) {
		for(int ky = -rd; ky < rd+1; ky++) {
			self2 += imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r / totl;
			diff2 += imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r / totl; } }

	for(int kx = -rd; kx < rd+1; kx++) {
		for(int ky = -rd; ky < rd+1; ky++) {
			self = imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r - self2;
			diff = imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r - diff2;
			rslt+= ( 1.0 - ( abs(abs(self)-abs(diff)) )) / totl; } }

	imageStore( img_out, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4( rslt, 0.0, 0.0, 0.0 ) );

	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
			imageStore( img_out, ivec2(rd+kx+rd*2.0*0.0+1.0, rd+ky+rd*2.0*0.0+1.0), 	imageLoad( img_in, ivec2(xx+kx, yy+ky) ) );
			imageStore( img_out, ivec2(rd+kx+rd*2.0*1.0+3.0, rd+ky+rd*2.0*0.0+1.0), abs(imageLoad( img_in, ivec2(xx+kx, yy+ky) ) - diff2) ); } }
/**/

//	Seek similar gradient intensity
/*	float self = 0.0;
	float diff = 0.0;
	vec2 slim = vec2(1.0,0.0);
	vec2 dlim = vec2(1.0,0.0);
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r;
			diff = imageLoad( img_in, ivec3(365, 230, (kx+r)+(ky+r)*(r*2+1)) ).r;
			if(self < slim[0]) { slim[0] = self; }
			if(diff < dlim[0]) { dlim[0] = diff; }
			if(self > slim[1]) { slim[1] = self; }
			if(diff > dlim[1]) { dlim[1] = diff; } } }

	float rslt = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = slim[1]-slim[0];
			diff = dlim[1]-dlim[0];
			rslt+= abs(self-diff)/((r*2+1)*(r*2+1)); } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(rslt,0.0,0.0,0.0) );/**/

//	
/*	float self = 0.0;
	float diff = 0.0;
	vec2 slim = vec2(1.0,0.0);
	vec2 dlim = vec2(1.0,0.0);
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r;
			diff = imageLoad( img_in, ivec3(365, 230, (kx+r)+(ky+r)*(r*2+1)) ).r;
			if(self < slim[0]) { slim[0] = self; }
			if(diff < dlim[0]) { dlim[0] = diff; }
			if(self > slim[1]) { slim[1] = self; }
			if(diff > dlim[1]) { dlim[1] = diff; } } }

	self = 0.0;
	diff = 0.0;
	float rslt = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r;
			self-= slim[0];
			self*= 1.0/(slim[0]+slim[1]);
			diff = imageLoad( img_in, ivec3(365, 230, (kx+r)+(ky+r)*(r*2+1)) ).r;
			diff-= dlim[0];
			diff*= 1.0/(dlim[0]+dlim[1]);
			rslt+= abs(self-diff)/((r*2+1)*(r*2+1)); } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(rslt,0.0,0.0,0.0) );/**/

//	
/*	float self = 0.0;
	float self2 = 0.0;
	float diff = 0.0;
	float diff2 = 0.0;
	float rslt = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r;
			self2 += self;
			diff = imageLoad( img_in, ivec3(365, 230, (kx+r)+(ky+r)*(r*2+1) ) ).r;
			diff2 += diff; } }
			rslt+= abs(self2-diff2)/((r*2+1)*(r*2+1));

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(rslt,0.0,0.0,0.0) );/**/


//	Grab Kernel at location, convolve whole image with it
/*	float self = 0.0;
	float diff = 0.0;
	float rslt = 0.0;
	float ssum = 0.0;
	float dsum = 0.0;
	//float self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ((r*2+1)*(r*2+1))/2) ).r;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = (imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r-0.0)*1.0;
			diff = (imageLoad( img_in, ivec3(365, 230, (kx+r)+(ky+r)*(r*2+1)) ).r-0.5)*2.0;
			ssum += self;
			dsum += diff;
			rslt += (self * diff); } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(rslt/dsum,0.0,0.0,0.0) );/**/


//
/*	float self = 0.0;
	float diff = 0.0;
	float rslt = 0.0;
	float ssum = 0.0;
	float dsum = 0.0;
	//float self = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ((r*2+1)*(r*2+1))/2) ).r;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self = (imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r-0.0)*1.0;
			diff = (imageLoad( img_in, ivec3(365, 230, (kx+r)+(ky+r)*(r*2+1)) ).r-0.5)*2.0;
			ssum += self;
			dsum += diff/((r*2+1)*(r*2+1));
			rslt += (self * diff); } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(rslt*dsum,0.0,0.0,0.0) );/**/




//	best so far
/*	float orig = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ((r*2+1)*(r*2+1))/2) ).r;

	float totl = ((r*2+1)*(r*2+1));
	float self = 0.0;
	float diff = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self += imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r / totl;
			diff += imageLoad( img_in, ivec3(270, 330, (kx+r)+(ky+r)*(r*2+1)) ).r / totl; } }

	float rslt = 0.0;
	float self2 = 0.0;
	float diff2 = 0.0;
	for(int kx = -r; kx < r+1; kx++) {
		for(int ky = -r; ky < r+1; ky++) {
			self2 = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r - self;
			diff2 = imageLoad( img_in, ivec3(270, 330, (kx+r)+(ky+r)*(r*2+1)) ).r - diff;
			rslt += (1.0 - abs(self2 - diff2)) / totl; } }

	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(rslt*orig,0.0,0.0,0.0) );/**/

//	270, 330
//	365, 230
//	230, 110
//	175, 340
//	490, 30
//	20, 310
//	394, 104
//	165, 390

//	Incredible! Seems to have trouble with larger convs - too dark. Might be correct behaviour?
/*	int r = int(ub.v0);

	float orig = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ((r*2+1)*(r*2+1))/2) ).r;
	float totl = ((r*2+1)*(r*2+1));
	float self = 0.0;
	for(int kx = -int(r); kx < r+1; kx++) {
		for(int ky = -int(r); ky < r+1; ky++) {
			self += imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r / totl; } }

	float match = 0.0;
	int size = 0;
//	int xx = int(ub.v63)%512;
//	int yy = (int(ub.v63)/512)*1;
	int xx = int(ub.v1)%512;
	int yy = int(ub.v2)%512;
	for(int i = xx-size; i < xx+1+size; i++) {
		for(int j = yy-size; j < yy+1+size; j++) {
			float diff = 0.0;
			for(int kx = -int(r); kx < r+1; kx++) {
				for(int ky = -int(r); ky < r+1; ky++) {
					diff += imageLoad( img_in, ivec3(i, j, (kx+r)+(ky+r)*(r*2+1)) ).r / totl; } }
			float self2 = 0.0;
			float diff2 = 0.0;
			float rslt = 1.0;
			for(int kx = -int(r); kx < r+1; kx++) {
				for(int ky = -int(r); ky < r+1; ky++) {
					self2 = imageLoad( img_in, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, (kx+r)+(ky+r)*(r*2+1) ) ).r - self;
					diff2 = imageLoad( img_in, ivec3(i, j, (kx+r)+(ky+r)*(r*2+1)) ).r - diff;
					rslt *= 1.0 - abs(self2 - diff2); } }
			match += rslt / ((size*2+1)*(size*2+1)); } }


	imageStore( img_out, ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, 0), vec4(match,0.0,0.0,0.0) );/**/


//	WOW!
/*	int rd = int(ub.v0);

	float orig = imageLoad( img_in, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y) ).r;
	float totl = ((rd*2+1)*(rd*2+1));
	float self = 0.0;
	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
			self += imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r / totl; } }

	float match = 0.0;
	int size = 0;
	int xx = int(ub.v1)%512;
	int yy = int(ub.v2)%512;
	for(int i = xx-size; i < xx+1+size; i++) {
		for(int j = yy-size; j < yy+1+size; j++) {
			float diff = 0.0;
			for(int kx = -int(rd); kx < rd+1; kx++) {
				for(int ky = -int(rd); ky < rd+1; ky++) {
					diff += imageLoad( img_in, ivec2(i+kx, j+ky) ).r / totl; } }
			float self2 = 0.0;
			float diff2 = 0.0;
			float rslt = 0.0;
			for(int kx = -int(rd); kx < rd+1; kx++) {
				for(int ky = -int(rd); ky < rd+1; ky++) {
					self2 = imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r - self;
					diff2 = imageLoad( img_in, ivec2(i+kx, j+ky) ).r - diff;
					rslt += (1.0 - abs(self2 - diff2)) / totl; } }
			match += rslt / ((size*2+1)*(size*2+1)); } }


	imageStore( img_out, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(pow(match,8.0),0.0,0.0,0.0) );/**/

//	Bestest
	int rd = int(ub.v0);

	float dist = 2.0;
	float dadd = 1.0;
	float ddis = 0.0;
	float dpow = 1.0;

	float orig = imageLoad( img_in, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y) ).r;
//	float totl = ((rd*2+1)*(rd*2+1));
	float totl = 0.0;
	float self = 0.0;

	bool bypass = false;

	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
//			dist = pow(1.0 - (sqrt(kx*kx+ky*ky+1.0) / (rd + dadd) ) * ddis, dpow);
			if(round(dist) >= 0.0 || bypass) { totl += 1.0; } } }

	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
//			float dist = 1.0 / sqrt(kx*kx+ky*ky+1.0);
//			dist = pow(1.0 - (sqrt(kx*kx+ky*ky+1.0) / (rd + dadd) ) * ddis, dpow);
			if(round(dist) >= 0.0 || bypass){
			self += (imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r * dist) / (totl * dist); } } }

	int xx = int(ub.v1)%512;
	int yy = int(ub.v2)%512;
	float diff = 0.0;
	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
//			float dist = 1.0 / sqrt(kx*kx+ky*ky+1.0);
//			dist = pow(1.0 - (sqrt(kx*kx+ky*ky+1.0) / (rd + dadd) ) * ddis, dpow);
			if(round(dist) >= 0.0 || bypass){
			diff += (imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r * dist) / (totl * dist); } } }

	float self2 = 0.0;
	float diff2 = 0.0;
	float rslt = 0.0;
	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
//			float dist = 1.0 / sqrt(kx*kx+ky*ky+1.0);
//			dist = pow(1.0 - (sqrt(kx*kx+ky*ky+1.0) / (rd + dadd) ) * ddis, dpow);
			if(round(dist) >= 0.0 || bypass){
//			self2 = (imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r * dist) - self * dist;
//			diff2 = (imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r * dist) - diff * dist;
			self2 = (sign((imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r * dist) - self * dist)/2.0)+0.5;
			diff2 = (sign((imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r * dist) - diff * dist)/2.0)+0.5;
			rslt += ( ( 1.0 ) - abs(self2 - diff2)) / (totl * 1); } } }

//TODO The similarity is low if the pattern is weak. Normalise: "rslt += (<1.0> - "
//	Cares too much about the absolute gradient - would rather match with flat space even if there's a feature.
//	Need to subtract or divide by some baseline value? Eg, the averages, 'self' or 'diff'?

	imageStore( img_out, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(pow(rslt,1.0),0.0,0.0,0.0) );

	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
//			float dist = 1.0 / sqrt(kx*kx+ky*ky+1.0);
//			dist = pow(1.0 - (sqrt(kx*kx+ky*ky+1.0) / (rd + dadd) ) * ddis, dpow);
			if(round(dist) >= 0.0 || bypass){
			imageStore( img_out, ivec2(rd+kx+rd*2.0*2.0+5.0, rd+ky+rd*2.0*0.0+1.0), 	vec4(dist,0.0,0.0,0.0) );
			imageStore( img_out, ivec2(rd+kx+rd*2.0*0.0+1.0, rd+ky+rd*2.0*0.0+1.0), 	imageLoad( img_in, ivec2(xx+kx, yy+ky) ) );
			imageStore( img_out, ivec2(rd+kx+rd*2.0*1.0+3.0, rd+ky+rd*2.0*0.0+1.0), abs(imageLoad( img_in, ivec2(xx+kx, yy+ky) ) - diff) );
			imageStore( img_out, ivec2(rd+kx+rd*2.0*0.0+1.0, rd+ky+rd*2.0*1.0+3.0), 	imageLoad( img_in, ivec2(xx+kx, yy+ky) ) * dist );
			imageStore( img_out, ivec2(rd+kx+rd*2.0*1.0+3.0, rd+ky+rd*2.0*1.0+3.0), abs(imageLoad( img_in, ivec2(xx+kx, yy+ky) ) * dist - diff * dist) ); } } }/**/

//	
/*	const float psn = 32768.0;

	int rd = int(ub.v0);
	float rslt = 0.0;

	for(int rep = 1; rep < rd+1; rep++) {
		vec2 rv2 = vec2(rep, rep-1);
		vec2 r2 = ceil(rv2 + vec2(0.5)) - vec2(0.500001);
		r2 = r2 * r2;

		float self  = 0.0;
		float selfa = 0.0;
		float selfb = 0.0;
		for(float j = 0.0; j <= rv2[0]; j++) {
			vec2 bound = sqrt(max(vec2(0),r2 - vec2(j*j)));
			for(float i = floor(bound[1])+1; i <= bound[0]; i++) {
				selfb += psn * 4.0;
				selfa += floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+i, gl_GlobalInvocationID.y+j) ).r * psn);
				selfa += floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+j, gl_GlobalInvocationID.y-i) ).r * psn);
				selfa += floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x-i, gl_GlobalInvocationID.y-j) ).r * psn);
				selfa += floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x-j, gl_GlobalInvocationID.y+i) ).r * psn); } }
		self = selfa / selfb;

		int xx = int(ub.v1)%512;
		int yy = int(ub.v2)%512;
		float diff  = 0.0;
		float diffa = 0.0;
		float diffb = 0.0;
		for(float j = 0.0; j <= rv2[0]; j++) {
			vec2 bound = sqrt(max(vec2(0),r2 - vec2(j*j)));
			for(float i = floor(bound[1])+1; i <= bound[0]; i++) {
				diffb += psn * 4.0;
				diffa += floor( imageLoad( img_in, ivec2(xx+i, yy+j) ).r * psn);
				diffa += floor( imageLoad( img_in, ivec2(xx+j, yy-i) ).r * psn);
				diffa += floor( imageLoad( img_in, ivec2(xx-i, yy-j) ).r * psn);
				diffa += floor( imageLoad( img_in, ivec2(xx-j, yy+i) ).r * psn); } }
		diff = diffa / diffb;

		float self2 = 0.0;
		float diff2 = 0.0;
		for(float j = 0.0; j <= rv2[0]; j++) {
			vec2 bound = sqrt(max(vec2(0),r2 - vec2(j*j)));
			for(float i = floor(bound[1])+1; i <= bound[0]; i++) {
				self2 = floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+i, gl_GlobalInvocationID.y+j) ).r * psn) - self * psn;
				diff2 = floor( imageLoad( img_in, ivec2(xx+i, yy+j) ).r * psn) - diff * psn;
				rslt += (psn - abs(self2 - diff2)) / selfb;
				self2 = floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+j, gl_GlobalInvocationID.y-i) ).r * psn) - self * psn;
				diff2 = floor( imageLoad( img_in, ivec2(xx+j, yy-i) ).r * psn) - diff * psn;
				rslt += (psn - abs(self2 - diff2)) / selfb;
				self2 = floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x-i, gl_GlobalInvocationID.y-j) ).r * psn) - self * psn;
				diff2 = floor( imageLoad( img_in, ivec2(xx-i, yy-j) ).r * psn) - diff * psn;
				rslt += (psn - abs(self2 - diff2)) / selfb;
				self2 = floor( imageLoad( img_in, ivec2(gl_GlobalInvocationID.x-j, gl_GlobalInvocationID.y+i) ).r * psn) - self * psn;
				diff2 = floor( imageLoad( img_in, ivec2(xx-j, yy+i) ).r * psn) - diff * psn;
				rslt += (psn - abs(self2 - diff2)) / selfb; } }
	}
	imageStore( img_out, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(pow(rslt/rd,8.0),0.0,0.0,0.0) );/**/


//	Bestest 2
/*	int xx = int(ub.v1)%512;
	int yy = int(ub.v2)%512;
	float orig = imageLoad( img_in, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y) ).r;
	float orid = imageLoad( img_in, ivec2(xx, yy) ).r;
	int rd = int(ub.v0);
	float mult = 2.0;

	float totl = 0.0;
	float self = 0.0;
	float diff = 0.0;
	float ediff = 0.0;
	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
			totl += 1.0;
			float sf = imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r;
			float df = imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r;
			self += sf;
			diff += df;
			ediff += abs(sf-df); } }

	self = self / totl;
	diff = diff / totl;

	float sdiff = 0.0;
	float ddiff = 0.0;
	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
			totl += 1.0;
			float sf = imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r;
			float df = imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r;
			sdiff += abs(sf-orig) / totl;
			ddiff += abs(df-orid) / totl; } }


	float 	odiff = abs(orig - orid);
	float 	adiff = abs(self - diff);
			ediff = ediff / totl;
	float 	fdiff = abs(odiff - adiff);

/*	float self2 = 0.0;
	float diff2 = 0.0;
	float self3 = 0.0;
	float diff3 = 0.0;
	float rslt = 0.0;
	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
			self2 = (imageLoad( img_in, ivec2(gl_GlobalInvocationID.x+kx, gl_GlobalInvocationID.y+ky) ).r * mult) - self * mult;
			diff2 = (imageLoad( img_in, ivec2(xx+kx, yy+ky) ).r * mult) - diff * mult;
			self3 += abs(self2) / totl;
			diff3 += abs(diff2) / totl;
			rslt += ( ( 1.0 ) - abs(abs(self2)-abs(diff2))) / (totl * 1); } }*/

//black = flat similarity, make it so it doesn't contribute to the detection?

//TODO The similarity is low if the pattern is weak. Normalise: "rslt += (<1.0> - "
//	Cares too much about the absolute gradient - would rather match with flat space even if there's a feature.
//	Need to subtract or divide by some baseline value? Eg, the averages, 'self' or 'diff'?

/*	imageStore( img_out, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(abs(sdiff/ddiff),0.0,0.0,0.0) );

/*	for(int kx = -int(rd); kx < rd+1; kx++) {
		for(int ky = -int(rd); ky < rd+1; ky++) {
			imageStore( img_out, ivec2(rd+kx+rd*2.0*0.0+1.0, rd+ky+rd*2.0*0.0+1.0), 	imageLoad( img_in, ivec2(xx+kx, yy+ky) ) );
			imageStore( img_out, ivec2(rd+kx+rd*2.0*1.0+3.0, rd+ky+rd*2.0*0.0+1.0), abs(imageLoad( img_in, ivec2(xx+kx, yy+ky) ) - diff) ); } }*/

}
