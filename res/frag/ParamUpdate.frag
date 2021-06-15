#version 460
//	Shader developed by Slackermanz:
//		https://twitter.com/slackermanz
//		https://github.com/Slackermanz/VulkanAutomata
//		https://www.youtube.com/channel/UCmoNsNuM0M9VsIXfm2cHPiA/videos
//		https://www.shadertoy.com/user/SlackermanzCA
layout(location 	=  0) out 		vec4 out_col;
layout(binding 		=  0) uniform 	UniBuf_WF {
	uint wsize;
	uint frame;
	uint minfo; } ub;
layout(binding 		=  1) uniform 	sampler2D txdata;
layout(binding 		=  2) uniform 	sampler2D txrule;

ivec4 wsize_unpack(uint ui32) {
	ivec4 	wsize;
			wsize[0] = int(	 ui32 			& uint(0x00000FFF) );
			wsize[1] = int( (ui32 >> 12) 	& uint(0x00000FFF) );
			wsize[2] = int( (ui32 >> 24)	& uint(0x0000000F) );
			wsize[3] = int( (ui32 >> 28)	& uint(0x0000000F) );
	return 	wsize; }

ivec4 minfo_unpack(uint ui32) {
	ivec4 	minfo;
			minfo[0] = int( (ui32 >>  4) 	& uint(0x00000FFF) );
			minfo[1] = int(	(ui32 >> 16) 	& uint(0x00000FFF) );
			minfo[2] = int(	 ui32 		 	& uint(0x0000000F) );
			minfo[3] = int(	(ui32 >> 28) 	& uint(0x0000000F) );
	return 	minfo; }

float gdv(ivec2 off, int v) {
//	Get Div Value: Return the value of a specified pixel
//		x, y : 	Relative integer-spaced coordinates to origin [ 0.0, 0.0 ]
//		v	 :	Colour channel [ 0, 1, 2 ]
	ivec4	dm		= wsize_unpack(ub.wsize);
	vec4 	fc 		= gl_FragCoord;
	vec2	dc		= vec2( dm[0]/dm[2], dm[1]/dm[2] );
	float	cx		= mod(fc[0]+off[0], dc[0]) + floor(fc[0]/dc[0])*dc[0];
	float	cy		= mod(fc[1]+off[1], dc[1]) + floor(fc[1]/dc[1])*dc[1];
	vec4 	pxdata 	= texelFetch( txdata, ivec2(cx, cy), 0);
	return 	pxdata[v]; }

float data_val(ivec2 off, int v) {
//	Get Div Value: Return the value of a specified pixel
//		x, y : 	Relative integer-spaced coordinates to origin [ 0.0, 0.0 ]
//		v	 :	Colour channel [ 0, 1, 2 ]
	ivec4	dm		= wsize_unpack(ub.wsize);
	vec4 	fc 		= gl_FragCoord;
	vec2	dc		= vec2( dm[0]/dm[2], dm[1]/dm[2] );
	float	cx		= mod(fc[0]+off[0], dc[0]) + floor(fc[0]/dc[0])*dc[0];
	float	cy		= mod(fc[1]+off[1], dc[1]) + floor(fc[1]/dc[1])*dc[1];
	vec4 	pxdata 	= texelFetch( txdata, ivec2(cx, cy), 0);
	return 	pxdata[v]; }

float rule_val(ivec2 off, int v) {
//	Get Div Value: Return the value of a specified pixel
//		x, y : 	Relative integer-spaced coordinates to origin [ 0.0, 0.0 ]
//		v	 :	Colour channel [ 0, 1, 2 ]
	ivec4	dm		= wsize_unpack(ub.wsize);
	vec4 	fc 		= gl_FragCoord;
	vec2	dc		= vec2( dm[0]/dm[2], dm[1]/dm[2] );
	float	cx		= mod(fc[0]+off[0], dc[0]) + floor(fc[0]/dc[0])*dc[0];
	float	cy		= mod(fc[1]+off[1], dc[1]) + floor(fc[1]/dc[1])*dc[1];
	vec4 	pxdata 	= texelFetch( txrule, ivec2(cx, cy), 0);
	return 	pxdata[v]; }

vec3 nhd_rule( ivec2 nbhd, ivec2 ofst, float psn, float thr, int col ) {
//	Neighbourhood: Return information about the specified group of pixels
	float dist 		= 0.0;
	float cval 		= 0.0;
	float c_total 	= 0.0;
	float c_valid 	= 0.0;
	float c_value 	= 0.0;
	for(float i = -nbhd[0]; i <= nbhd[0]; i+=1.0) {
		for(float j = -nbhd[0]; j <= nbhd[0]; j+=1.0) {
			dist = round(sqrt(i*i+j*j));
			if( dist <= nbhd[0] && dist > nbhd[1] && dist != 0.0 ) {
				cval = rule_val(ivec2(i+ofst[0],j+ofst[1]),col);
				c_total += psn;
				if( cval > thr ) {
					c_valid += psn;
					cval = psn * cval;
					c_value += cval-fract(cval); } } } }
	return vec3( c_value, c_valid, c_total ); }

vec3 nhd_data( ivec2 nbhd, ivec2 ofst, float psn, float thr, int col ) {
//	Neighbourhood: Return information about the specified group of pixels
	float dist 		= 0.0;
	float cval 		= 0.0;
	float c_total 	= 0.0;
	float c_valid 	= 0.0;
	float c_value 	= 0.0;
	for(float i = -nbhd[0]; i <= nbhd[0]; i+=1.0) {
		for(float j = -nbhd[0]; j <= nbhd[0]; j+=1.0) {
			dist = round(sqrt(i*i+j*j));
			if( dist <= nbhd[0] && dist > nbhd[1] && dist != 0.0 ) {
				cval = data_val(ivec2(i+ofst[0],j+ofst[1]),col);
				c_total += psn;
				if( cval > thr ) {
					c_valid += psn;
					cval = psn * cval;
					c_value += cval-fract(cval); } } } }
	return vec3( c_value, c_valid, c_total ); }

//	Used to reseed the surface with lumpy noise
float get_xc(float x, float y, float xmod) {
	float sq = sqrt(mod(x*y+y, xmod)) / sqrt(xmod);
	float xc = mod((x*x)+(y*y), xmod) / xmod;
	return clamp((sq+xc)*0.5, 0.0, 1.0); }
float shuffle(float x, float y, float xmod, float val) {
	val = val * mod( x*y + x, xmod );
	return (val-floor(val)); }
float get_xcn(float x, float y, float xm0, float xm1, float ox, float oy) {
	float  xc = get_xc(x+ox, y+oy, xm0);
	return shuffle(x+ox, y+oy, xm1, xc); }
float get_lump(float x, float y, float nhsz, float xm0, float xm1) {
	float 	nhsz_c 	= 0.0;
	float 	xcn 	= 0.0;
	float 	nh_val 	= 0.0;
	for(float i = -nhsz; i <= nhsz; i += 1.0) {
		for(float j = -nhsz; j <= nhsz; j += 1.0) {
			nh_val = round(sqrt(i*i+j*j));
			if(nh_val <= nhsz) {
				xcn = xcn + get_xcn(x, y, xm0, xm1, i, j);
				nhsz_c = nhsz_c + 1.0; } } }
	float 	xcnf 	= ( xcn / nhsz_c );
	float 	xcaf	= xcnf;
	for(float i = 0.0; i <= nhsz; i += 1.0) {
			xcaf 	= clamp((xcnf*xcaf + xcnf*xcaf) * (xcnf+xcnf), 0.0, 1.0); }
	return xcaf; }
float reseed(int seed) {
	vec4	fc = gl_FragCoord;
	float 	r0 = get_lump(fc[0], fc[1], 12.0, 19.0 + mod(ub.frame+seed,17.0), 23.0 + mod(ub.frame+seed,43.0));
	float 	r1 = get_lump(fc[0], fc[1], 32.0, 13.0 + mod(ub.frame+seed,29.0), 17.0 + mod(ub.frame+seed,31.0));
	float 	r2 = get_lump(fc[0], fc[1], 22.0, 13.0 + mod(ub.frame+seed,11.0), 51.0 + mod(ub.frame+seed,37.0));
	float 	r3 = get_lump(fc[0], fc[1], 18.0, 29.0 + mod(ub.frame+seed, 7.0), 61.0 + mod(ub.frame+seed,28.0));
	return clamp( sqrt((r0+r1)*r3*2.0)-r2 , 0.0, 1.0); }

void main() {

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Setup
//	----    ----    ----    ----    ----    ----    ----    ----

	float 	psn		= 65536.0;					//	Texture Precision
	float 	mnp 	= 1.0 / psn;				//	Minimum value of a precise step
	ivec4 	minfo 	= minfo_unpack(ub.minfo);	//	Mouse State Information
	ivec2	origin  = ivec2(0,0);

//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----

	float 	ref_r 	= gdv( origin, 0 );
	float 	ref_g 	= gdv( origin, 1 );
	float 	ref_b 	= gdv( origin, 2 );

	float 	res_r 	= ref_r;
	float 	res_g 	= ref_g;
	float 	res_b 	= ref_b;

//	Parameters
	float s 	= mnp * 16.0 *  96.0;

	float n 	= mnp * 16.0 * 256.0;
	float a 	= mnp * 16.0 *  64.0;
	float b 	= mnp * 16.0 *  64.0;

	float li 	= mnp * 16.0 *   2.0;
	float lu 	= mnp * 16.0 *   2.0;
	float cy 	= mnp * 16.0 *   3.0;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Transition Functions
//	----    ----    ----    ----    ----    ----    ----    ----

	float 	ref_para_r = gdv( origin, 0 );
	float 	ref_para_g = gdv( origin, 1 );
	float 	ref_para_b = gdv( origin, 2 );

	float 	ref_rule_r = rule_val( origin, 0 );
	float 	ref_rule_g = rule_val( origin, 1 );
	float 	ref_rule_b = rule_val( origin, 2 );

	vec3 nhdt_r = nhd_data(ivec2(3,0), origin, psn, 0.0, 0);
	vec3 nhdt_g = nhd_data(ivec2(3,0), origin, psn, 0.0, 1);
	vec3 nhdt_b = nhd_data(ivec2(3,0), origin, psn, 0.0, 2);
	float  dt_r = nhdt_r[0] / nhdt_r[2];
	float  dt_g = nhdt_g[0] / nhdt_g[2];
	float  dt_b = nhdt_b[0] / nhdt_b[2];

	vec3 nhrl_r = nhd_rule(ivec2(3,0), origin, psn, 0.0, 0);
	vec3 nhrl_g = nhd_rule(ivec2(3,0), origin, psn, 0.0, 1);
	vec3 nhrl_b = nhd_rule(ivec2(3,0), origin, psn, 0.0, 2);
	float  rl_r = nhrl_r[0] / nhrl_r[2];
	float  rl_g = nhrl_g[0] / nhrl_g[2];
	float  rl_b = nhrl_b[0] / nhrl_b[2];

//	----    ----    ----    ----    ----    ----    ----    ----

	vec3 ndr0  = nhd_data(ivec2(1,0), origin, psn, 0.0, 0);
	vec3 ndr1  = nhd_data(ivec2(2,1), origin, psn, 0.0, 0);
	vec3 ndr2  = nhd_data(ivec2(3,2), origin, psn, 0.0, 0);
	vec3 ndr3  = nhd_data(ivec2(4,3), origin, psn, 0.0, 0);
	vec3 ndr4  = nhd_data(ivec2(5,4), origin, psn, 0.0, 0);
	vec3 ndr5  = nhd_data(ivec2(6,5), origin, psn, 0.0, 0);
	float dtr0 = ndr0[0] / ndr0[2];
	float dtr1 = ndr1[0] / ndr1[2];
	float dtr2 = ndr2[0] / ndr2[2];
	float dtr3 = ndr3[0] / ndr3[2];
	float dtr4 = ndr4[0] / ndr4[2];
	float dtr5 = ndr5[0] / ndr5[2];
	float vdtr0 = ref_para_r - dtr0;
	float vdtr1 = ref_para_r - dtr1;
	float vdtr2 = ref_para_r - dtr2;
	float vdtr3 = ref_para_r - dtr3;
	float vdtr4 = ref_para_r - dtr4;
	float vdtr5 = ref_para_r - dtr5;

	float 	minvar_dtr 	= vdtr0;
	float 	minvar_dtrnh = dtr0;
	int		min_dtr_i 	= 0;
	if(abs(minvar_dtr) > abs(vdtr1)) 	{ minvar_dtr = vdtr1; min_dtr_i = 1; minvar_dtrnh = dtr1; }
	if(abs(minvar_dtr) > abs(vdtr2)) 	{ minvar_dtr = vdtr2; min_dtr_i = 2; minvar_dtrnh = dtr2; }
	if(abs(minvar_dtr) > abs(vdtr3)) 	{ minvar_dtr = vdtr3; min_dtr_i = 3; minvar_dtrnh = dtr3; }
	if(abs(minvar_dtr) > abs(vdtr4)) 	{ minvar_dtr = vdtr4; min_dtr_i = 4; minvar_dtrnh = dtr4; }
	if(abs(minvar_dtr) > abs(vdtr5)) 	{ minvar_dtr = vdtr5; min_dtr_i = 5; minvar_dtrnh = dtr5; }
	if(min_dtr_i == 0) { res_r = (res_r + dtr0 * b) / ( 1.0 + b ); }
	if(min_dtr_i == 1) { res_r = (res_r + dtr1 * b) / ( 1.0 + b ); }
	if(min_dtr_i == 2) { res_r = (res_r + dtr2 * b) / ( 1.0 + b ); }
	if(min_dtr_i == 3) { res_r = (res_r + dtr3 * b) / ( 1.0 + b ); }
	if(min_dtr_i == 4) { res_r = (res_r + dtr4 * b) / ( 1.0 + b ); }
	if(min_dtr_i == 5) { res_r = (res_r + dtr5 * b) / ( 1.0 + b ); }

	float 	maxvar_dtr 	 = vdtr0;
	float 	maxvar_dtrnh = dtr0;
	int		max_dtr_i 	 = 0;
	if(abs(maxvar_dtr) < abs(vdtr1)) 	{ maxvar_dtr = vdtr1; max_dtr_i = 1; maxvar_dtrnh = dtr1; }
	if(abs(maxvar_dtr) < abs(vdtr2)) 	{ maxvar_dtr = vdtr2; max_dtr_i = 2; maxvar_dtrnh = dtr2; }
	if(abs(maxvar_dtr) < abs(vdtr3)) 	{ maxvar_dtr = vdtr3; max_dtr_i = 3; maxvar_dtrnh = dtr3; }
	if(abs(maxvar_dtr) < abs(vdtr4)) 	{ maxvar_dtr = vdtr4; max_dtr_i = 4; maxvar_dtrnh = dtr4; }
	if(abs(maxvar_dtr) < abs(vdtr5)) 	{ maxvar_dtr = vdtr5; max_dtr_i = 5; maxvar_dtrnh = dtr5; }

	if(abs(ref_rule_r - rl_r ) >= abs(ref_para_r - dt_r)) 	{ res_r = res_r + sign(minvar_dtr) * a; } 

	res_r = (res_r + maxvar_dtrnh * n) / (1.0 + n);

//	----    ----    ----    ----    ----    ----    ----    ----

	vec3 ndg0  = nhd_data(ivec2(1,0), origin, psn, 0.0, 1);
	vec3 ndg1  = nhd_data(ivec2(2,1), origin, psn, 0.0, 1);
	vec3 ndg2  = nhd_data(ivec2(3,2), origin, psn, 0.0, 1);
	vec3 ndg3  = nhd_data(ivec2(4,3), origin, psn, 0.0, 1);
	vec3 ndg4  = nhd_data(ivec2(5,4), origin, psn, 0.0, 1);
	vec3 ndg5  = nhd_data(ivec2(6,5), origin, psn, 0.0, 1);
	float dtg0 = ndg0[0] / ndg0[2];
	float dtg1 = ndg1[0] / ndg1[2];
	float dtg2 = ndg2[0] / ndg2[2];
	float dtg3 = ndg3[0] / ndg3[2];
	float dtg4 = ndg4[0] / ndg4[2];
	float dtg5 = ndg5[0] / ndg5[2];
	float vdtg0 = ref_para_g - dtg0;
	float vdtg1 = ref_para_g - dtg1;
	float vdtg2 = ref_para_g - dtg2;
	float vdtg3 = ref_para_g - dtg3;
	float vdtg4 = ref_para_g - dtg4;
	float vdtg5 = ref_para_g - dtg5;

	float 	minvar_dtg 	= vdtg0;
	float 	minvar_dtgnh = dtg0;
	int		min_dtg_i 	= 0;
	if(abs(minvar_dtg) > abs(vdtg1)) 	{ minvar_dtg = vdtg1; min_dtg_i = 1; minvar_dtgnh = dtg1; }
	if(abs(minvar_dtg) > abs(vdtg2)) 	{ minvar_dtg = vdtg2; min_dtg_i = 2; minvar_dtgnh = dtg2; }
	if(abs(minvar_dtg) > abs(vdtg3)) 	{ minvar_dtg = vdtg3; min_dtg_i = 3; minvar_dtgnh = dtg3; }
	if(abs(minvar_dtg) > abs(vdtg4)) 	{ minvar_dtg = vdtg4; min_dtg_i = 4; minvar_dtgnh = dtg4; }
	if(abs(minvar_dtg) > abs(vdtg5)) 	{ minvar_dtg = vdtg5; min_dtg_i = 5; minvar_dtgnh = dtg5; }
	if(min_dtg_i == 0) { res_g = (res_g + dtg0 * b) / ( 1.0 + b ); }
	if(min_dtg_i == 1) { res_g = (res_g + dtg1 * b) / ( 1.0 + b ); }
	if(min_dtg_i == 2) { res_g = (res_g + dtg2 * b) / ( 1.0 + b ); }
	if(min_dtg_i == 3) { res_g = (res_g + dtg3 * b) / ( 1.0 + b ); }
	if(min_dtg_i == 4) { res_g = (res_g + dtg4 * b) / ( 1.0 + b ); }
	if(min_dtg_i == 5) { res_g = (res_g + dtg5 * b) / ( 1.0 + b ); }

	float 	maxvar_dtg 	 = vdtg0;
	float 	maxvar_dtgnh = dtg0;
	int		max_dtg_i 	 = 0;
	if(abs(maxvar_dtg) < abs(vdtg1)) 	{ maxvar_dtg = vdtg1; max_dtg_i = 1; maxvar_dtgnh = dtg1; }
	if(abs(maxvar_dtg) < abs(vdtg2)) 	{ maxvar_dtg = vdtg2; max_dtg_i = 2; maxvar_dtgnh = dtg2; }
	if(abs(maxvar_dtg) < abs(vdtg3)) 	{ maxvar_dtg = vdtg3; max_dtg_i = 3; maxvar_dtgnh = dtg3; }
	if(abs(maxvar_dtg) < abs(vdtg4)) 	{ maxvar_dtg = vdtg4; max_dtg_i = 4; maxvar_dtgnh = dtg4; }
	if(abs(maxvar_dtg) < abs(vdtg5)) 	{ maxvar_dtg = vdtg5; max_dtg_i = 5; maxvar_dtgnh = dtg5; }

	if(abs(ref_rule_g - rl_g ) >= abs(ref_para_g - dt_g)) 	{ res_g = res_g + sign(minvar_dtg) * a; } 

	res_g = (res_g + maxvar_dtgnh * n) / (1.0 + n);

//	----    ----    ----    ----    ----    ----    ----    ----

	vec3 ndb0  = nhd_data(ivec2(1,0), origin, psn, 0.0, 2);
	vec3 ndb1  = nhd_data(ivec2(2,1), origin, psn, 0.0, 2);
	vec3 ndb2  = nhd_data(ivec2(4,2), origin, psn, 0.0, 2);
	vec3 ndb3  = nhd_data(ivec2(4,3), origin, psn, 0.0, 2);
	vec3 ndb4  = nhd_data(ivec2(5,4), origin, psn, 0.0, 2);
	vec3 ndb5  = nhd_data(ivec2(6,5), origin, psn, 0.0, 2);
	float dtb0 = ndb0[0] / ndb0[2];
	float dtb1 = ndb1[0] / ndb1[2];
	float dtb2 = ndb2[0] / ndb2[2];
	float dtb3 = ndb3[0] / ndb3[2];
	float dtb4 = ndb4[0] / ndb4[2];
	float dtb5 = ndb5[0] / ndb5[2];
	float vdtb0 = ref_para_b - dtb0;
	float vdtb1 = ref_para_b - dtb1;
	float vdtb2 = ref_para_b - dtb2;
	float vdtb3 = ref_para_b - dtb3;
	float vdtb4 = ref_para_b - dtb4;
	float vdtb5 = ref_para_b - dtb5;

	float 	minvar_dtb 	= vdtb0;
	float 	minvar_dtbnh = dtb0;
	int		min_dtb_i 	= 0;
	if(abs(minvar_dtb) > abs(vdtb1)) 	{ minvar_dtb = vdtb1; min_dtb_i = 1; minvar_dtbnh = dtb1; }
	if(abs(minvar_dtb) > abs(vdtb2)) 	{ minvar_dtb = vdtb2; min_dtb_i = 2; minvar_dtbnh = dtb2; }
	if(abs(minvar_dtb) > abs(vdtb3)) 	{ minvar_dtb = vdtb3; min_dtb_i = 3; minvar_dtbnh = dtb3; }
	if(abs(minvar_dtb) > abs(vdtb4)) 	{ minvar_dtb = vdtb4; min_dtb_i = 4; minvar_dtbnh = dtb4; }
	if(abs(minvar_dtb) > abs(vdtb5)) 	{ minvar_dtb = vdtb5; min_dtb_i = 5; minvar_dtbnh = dtb5; }
	if(min_dtb_i == 0) { res_b = (res_b + dtb0 * b) / ( 1.0 + b ); }
	if(min_dtb_i == 1) { res_b = (res_b + dtb1 * b) / ( 1.0 + b ); }
	if(min_dtb_i == 2) { res_b = (res_b + dtb2 * b) / ( 1.0 + b ); }
	if(min_dtb_i == 3) { res_b = (res_b + dtb3 * b) / ( 1.0 + b ); }
	if(min_dtb_i == 4) { res_b = (res_b + dtb4 * b) / ( 1.0 + b ); }
	if(min_dtb_i == 5) { res_b = (res_b + dtb5 * b) / ( 1.0 + b ); }

	float 	maxvar_dtb 	 = vdtb0;
	float 	maxvar_dtbnh = dtb0;
	int		max_dtb_i 	 = 0;
	if(abs(maxvar_dtb) < abs(vdtb1)) 	{ maxvar_dtb = vdtb1; max_dtb_i = 1; maxvar_dtbnh = dtb1; }
	if(abs(maxvar_dtb) < abs(vdtb2)) 	{ maxvar_dtb = vdtb2; max_dtb_i = 2; maxvar_dtbnh = dtb2; }
	if(abs(maxvar_dtb) < abs(vdtb3)) 	{ maxvar_dtb = vdtb3; max_dtb_i = 3; maxvar_dtbnh = dtb3; }
	if(abs(maxvar_dtb) < abs(vdtb4)) 	{ maxvar_dtb = vdtb4; max_dtb_i = 4; maxvar_dtbnh = dtb4; }
	if(abs(maxvar_dtb) < abs(vdtb5)) 	{ maxvar_dtb = vdtb5; max_dtb_i = 5; maxvar_dtbnh = dtb5; }

	if(abs(ref_rule_b - rl_b ) >= abs(ref_para_b - dt_b)) 	{ res_b = res_b + sign(minvar_dtb) * a; } 

	res_b = (res_b + maxvar_dtbnh * n) / (1.0 + n);

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	if(ub.frame == 0 || minfo[3] == 1) { 
		res_r = reseed(3); res_g = reseed(4); res_b = reseed(5); }
	if(minfo[3] == 2) { 
		res_r = 0.5; res_g = 0.5; res_b = 0.5; }
	if(minfo[3] == 3) { 
		res_r = 1.0; res_g = 1.0; res_b = 1.0; }

		res_r = 1.0; res_g = 1.0; res_b = 1.0;
//		if(ub.frame != 0 && minfo[3] != 1) { res_r = ref_r; res_g = ref_g; res_b = ref_b; }
//		res_r = ref_r; res_g = ref_g; res_b = ref_b;

	out_col = vec4(clamp(res_r,0.0,1.0), clamp(res_g,0.0,1.0), clamp(res_b,0.0,1.0), 1.0);
}


