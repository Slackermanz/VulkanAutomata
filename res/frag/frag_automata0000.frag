//	----    ----    ----    ----    ----    ----    ----    ----
//
//	Shader developed by Slackermanz:
//
//		https://slackermanz.com
//
//		Discord:	Slackermanz#3405
//		Github:		https://github.com/Slackermanz
//		Twitter:	https://twitter.com/slackermanz
//		YouTube:	https://www.youtube.com/c/slackermanz
//		Shadertoy: 	https://www.shadertoy.com/user/SlackermanzCA
//		Reddit:		https://old.reddit.com/user/slackermanz
//
//		Communities:
//			Reddit:		https://old.reddit.com/r/cellular_automata
//			Discord:	https://discord.com/invite/J3phjtD
//			Discord:	https://discord.gg/BCuYCEn
//
//	Lenia test code based on the work of Bert Chan:
//		Twitter:	https://twitter.com/BertChakovsky
//		Github:		https://github.com/Chakazul/Lenia
//
//	----    ----    ----    ----    ----    ----    ----    ----

#version 460
#define PI 3.14159265359

//	----    ----    ----    ----    ----    ----    ----    ----

layout(location 	=  0) out 		vec4 out_col;
layout(binding 		=  1) uniform 	sampler2D txdata;
layout(binding 		=  0) uniform 	UniBuf {
	uint v0;  uint v1;  uint v2;  uint v3;	uint v4;  uint v5;  uint v6;  uint v7;
	uint v8;  uint v9;  uint v10; uint v11;	uint v12; uint v13; uint v14; uint v15;
	uint v16; uint v17; uint v18; uint v19;	uint v20; uint v21; uint v22; uint v23;
	uint v24; uint v25; uint v26; uint v27;	uint v28; uint v29; uint v30; uint v31;
	uint v32; uint v33; uint v34; uint v35;	uint v36; uint v37; uint v38; uint v39;
	uint v40; uint v41; uint v42; uint v43;	uint v44; uint v45; uint v46; uint v47;
	uint v48; uint v49; uint v50; uint v51;	uint v52; uint v53; uint v54; uint v55;
	uint v56; uint v57; uint v58; uint v59;	uint v60; uint v61; uint v62; uint v63; } ub;

uint u32_upk(uint u32, uint bts, uint off) { return (u32 >> off) & ((1 << bts)-1); }

vec3 gdc( ivec2 of, sampler2D tx ) {
	const	float 	fx 		= gl_FragCoord[0];
	const	float 	fy 		= gl_FragCoord[1];
	const	uint	wx 		= u32_upk(ub.v63, 12,  0);
	const	uint	wy 		= u32_upk(ub.v63, 12, 12);
	const	uint	wv 		= u32_upk(ub.v63,  4, 24);
	const	vec2	dc		= vec2( wx/wv, wy/wv );
	const	float	cx		= mod( fx+of[0], dc[0] ) + floor( fx/dc[0] ) * dc[0];
	const	float	cy		= mod( fy+of[1], dc[1] ) + floor( fy/dc[1] ) * dc[1];
	const	vec4 	pxdata 	= texelFetch( tx, ivec2(cx, cy), 0);
	return 	vec3( pxdata[0], pxdata[1], pxdata[2] ); }

float gdv( ivec2 of, sampler2D tx, int c ) {
	const	vec3 pxdata = gdc( of, tx );
	return 	pxdata[c]; }

float bell(float x, float m, float s) { return exp(-(x-m)*(x-m)/s/s/2.0); }  // bell-shaped curve

vec2 nbhd( ivec2 r, sampler2D tx, int c ) {
	const	uint	tmx = 65536u;
	const	uint	chk = 2147483648u / (
					( 	uint(float(r[0])*float(r[0])*PI + float(r[0])*PI + PI	)
					- 	uint(float(r[1])*float(r[1])*PI + float(r[1])*PI		) ) * 128 );
	const	float	psn = (chk >= tmx) ? float(tmx) : float(chk);
			float 	d = 0.0;
			float 	a = 0.0;
			float 	b = 0.0;
			float	t = 0.0;
	for(float i = -r[0]; i <= r[0]; i+=1.0) {
		for(float j = -r[0]; j <= r[0]; j+=1.0) {
					d = round(sqrt(i*i+j*j));
			float	w = bell(d/r[0], 0.5, 0.15);
			if( d <= r[0] && d > r[1] ) {
				t  = gdv( ivec2(i,j), tx, c ) * w * psn;
				a += t - fract(t);
				b += w * psn; } } }
	return vec2(a, b); }

//	----    ----    ----    ----    ----    ----    ----    ----

//	Used to reseed the surface with lumpy noise
float get_xc(float x, float y, float xmod) {
	const	float sq = sqrt(mod(x*y+y, xmod)) / sqrt(xmod);
	const	float xc = mod((x*x)+(y*y), xmod) / xmod;
	return clamp((sq+xc)*0.5, 0.0, 1.0); }
float shuffle(float x, float y, float xmod, float val) {
	val = val * mod( x*y + x, xmod );
	return (val-floor(val)); }
float get_xcn(float x, float y, float xm0, float xm1, float ox, float oy) {
	const	float  xc = get_xc(x+ox, y+oy, xm0);
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
	const	float 	fx = gl_FragCoord[0];
	const	float 	fy = gl_FragCoord[1];
	const	float 	r0 = get_lump(fx, fy,  6.0, 19.0 + mod(ub.v61+seed,17.0), 23.0 + mod(ub.v61+seed,43.0));
	const	float 	r1 = get_lump(fx, fy, 24.0, 13.0 + mod(ub.v61+seed,29.0), 17.0 + mod(ub.v61+seed,31.0));
	const	float 	r2 = get_lump(fx, fy, 12.0, 13.0 + mod(ub.v61+seed,11.0), 51.0 + mod(ub.v61+seed,37.0));
	const	float 	r3 = get_lump(fx, fy, 18.0, 29.0 + mod(ub.v61+seed, 7.0), 61.0 + mod(ub.v61+seed,28.0));
	return clamp( sqrt((r0+r1)*r3*2.0)-r2 , 0.0, 1.0); }

void main() {

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Setup
//	----    ----    ----    ----    ----    ----    ----    ----

	const	float	fx 		= gl_FragCoord[0];
	const	float	fy 		= gl_FragCoord[1];
	const 	ivec2	origin  = ivec2(0, 0);
	const	float 	mnp 	= 1.0 / 65536.0;			//	Minimum value of a precise step for 16-bit channel
	const	uint	wx 		= u32_upk(ub.v63, 12,  0);
	const	uint	wy 		= u32_upk(ub.v63, 12, 12);
	const	uint	wv 		= u32_upk(ub.v63,  4, 24);
	const	uint	wm 		= u32_upk(ub.v63,  4, 28);
	const	uint	mx 		= u32_upk(ub.v62, 12,  4);
	const	uint	my 		= u32_upk(ub.v62, 12, 16);
	const	uint	mb 		= u32_upk(ub.v62,  4,  0);
	const	uint	mc 		= u32_upk(ub.v62,  4, 28);
	const	float 	pidx	= floor((fx*wv)/wx)			//	Panel Division Index
							+ floor((fy*wv)/wy)*wv;
	const	vec3	ref_c	= gdc( origin, txdata );	//	Origin value references

//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----

//	Parameters
	const float dt 		= 0.020;    // time step
	const float mu 		= 0.165;    // mu = growth center
	const float sigma 	= 0.014;	// sigma = growth width

//	Output Values
	vec3 res_c = ref_c;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Transition Functions
//	----    ----    ----    ----    ----    ----    ----    ----

	vec2  avgnh		= nbhd(ivec2(15,0), txdata, 0);
	float avg		= avgnh[0] / avgnh[1];
	float growth 	= bell(avg, mu, sigma) * 2.0 - 1.0;
	float c 		= clamp(ref_c[0] + dt * growth,0.0,1.0);

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	res_c[0] = c;
	res_c[1] = res_c[0];
	res_c[2] = res_c[0];

	if(ub.v61 <= 0 || mc == 1) {
		res_c[0] = reseed(0); 
		res_c[1] = reseed(1); 
		res_c[2] = reseed(2); }

	out_col = vec4(res_c, 1.0);

}


