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
//			Reddit:	https://old.reddit.com/r/cellular_automata
//			Discord Servers:
//				Artificial Life: 	https://discord.gg/7qvBBVca7u
//				Emergence:			https://discord.com/invite/J3phjtD
//				ConwayLifeLounge:	https://discord.gg/BCuYCEn
//
//	----    ----    ----    ----    ----    ----    ----    ----

#version 460
#define PI 3.14159265359
#define LN 2.71828182846

//	----    ----    ----    ----    ----    ----    ----    ----

layout(location 	=  0) out 		vec4 		out_col;
layout(binding 		=  1) uniform 	sampler2D 	txdata;
layout(binding 		=  0) uniform 	UniBuf {
	uint v0;  uint v1;  uint v2;  uint v3;	uint v4;  uint v5;  uint v6;  uint v7;
	uint v8;  uint v9;  uint v10; uint v11;	uint v12; uint v13; uint v14; uint v15;
	uint v16; uint v17; uint v18; uint v19;	uint v20; uint v21; uint v22; uint v23;
	uint v24; uint v25; uint v26; uint v27;	uint v28; uint v29; uint v30; uint v31;
	uint v32; uint v33; uint v34; uint v35;	uint v36; uint v37; uint v38; uint v39;
	uint v40; uint v41; uint v42; uint v43;	uint v44; uint v45; uint v46; uint v47;
	uint v48; uint v49; uint v50; uint v51;	uint v52; uint v53; uint v54; uint v55;
	uint v56; uint v57; uint v58; uint v59;	uint v60; uint v61; uint v62; uint v63; } ub;

//	----    ----    ----    ----    ----    ----    ----    ----

const uint MAX_RADIUS = 12u;

//	----    ----    ----    ----    ----    ----    ----    ----

uint u32_upk(uint u32, uint bts, uint off) { return (u32 >> off) & ((1u << bts)-1u); }

float  tp(uint n, float s) 			{ return (float(n+1u)/256.0) * ((s*0.5)/128.0); }
float bsn(uint v, uint  o) 			{ return float(u32_upk(v,1u,o)*2u)-1.0; }
float utp(uint v, uint  w, uint o) 	{ return tp(u32_upk(v,w,w*o), uintBitsToFloat(ub.v62)); }

vec4  sigm(vec4  x, float w) { return 1.0 / ( 1.0 + exp( (-w*2.0 * x * (PI/2.0)) + w * (PI/2.0) ) ); }
float hmp2(float x, float w) { return 3.0*((x-0.5)*(x-0.5))+0.25; }

vec4  gdv( ivec2 of, sampler2D tx ) {
	of 		= ivec2(gl_FragCoord) + of;
	of[0] 	= (of[0] + textureSize(tx,0)[0]) & (textureSize(tx,0)[0]-1);
	of[1] 	= (of[1] + textureSize(tx,0)[1]) & (textureSize(tx,0)[1]-1);
	return 	texelFetch( tx, of, 0); }

vec4[2] nbhd( vec2 r, sampler2D tx ) {
//	Precision limit of signed float32 for [n] neighbors in a 16 bit texture (symmetry preservation)
	uint	chk = 2147483648u /
			(	( 	uint( r[0]*r[0]*PI + r[0]*PI + PI	)
				- 	uint( r[1]*r[1]*PI + r[1]*PI		) ) * 128u );
	float	psn = (chk >= 65536u) ? 65536.0 : float(chk);
	vec4	a = vec4(0.0,0.0,0.0,0.0);
	vec4 	b = vec4(0.0,0.0,0.0,0.0);
	for(float i = 0.0; i <= r[0]; i++) {
		for(float j = 1.0; j <= r[0]; j++) {
			float	d = round(sqrt(i*i+j*j));
			float	w = 1.0;
			if( d <= r[0] && d > r[1] ) {
					 b 	+= w * psn * 4.0;
				vec4 t0  = gdv( ivec2( i, j), tx ) * w * psn; a += t0 - fract(t0);
				vec4 t1  = gdv( ivec2( j,-i), tx ) * w * psn; a += t1 - fract(t1);
				vec4 t2  = gdv( ivec2(-i,-j), tx ) * w * psn; a += t2 - fract(t2);
				vec4 t3  = gdv( ivec2(-j, i), tx ) * w * psn; a += t3 - fract(t3); } } }
	return vec4[2](a, b); }

vec4 bitring(vec4[MAX_RADIUS][2] rings, uint bits, uint of) {
	vec4 sum = vec4(0.0,0.0,0.0,0.0);
	vec4 tot = vec4(0.0,0.0,0.0,0.0);
	for(uint i = 0u; i < MAX_RADIUS; i++) {
		if(u32_upk(bits, 1u, i+of) == 1u) { sum += rings[i][0]; tot += rings[i][1]; } }
	return sigm( (sum / tot), LN ); }

//	----    ----    ----    ----    ----    ----    ----    ----

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
	float 	fx = gl_FragCoord[0];
	float 	fy = gl_FragCoord[1];
	float 	r0 = get_lump(fx, fy,  6.0, 19.0 + mod(ub.v63+seed,17.0), 23.0 + mod(ub.v63+seed,43.0));
	float 	r1 = get_lump(fx, fy, 24.0, 13.0 + mod(ub.v63+seed,29.0), 17.0 + mod(ub.v63+seed,31.0));
	float 	r2 = get_lump(fx, fy, 12.0, 13.0 + mod(ub.v63+seed,11.0), 51.0 + mod(ub.v63+seed,37.0));
	float 	r3 = get_lump(fx, fy, 18.0, 29.0 + mod(ub.v63+seed, 7.0), 61.0 + mod(ub.v63+seed,28.0));
	return clamp( sqrt((r0+r1)*r3*2.0)-r2 , 0.0, 1.0); }

void main() {

//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----

//	NH Rings
	vec4[MAX_RADIUS][2] nh_rings_c;
	for(uint i = 0u; i < MAX_RADIUS; i++) {
		nh_rings_c[i] = nbhd( vec2(i+1u,i), txdata ); }

//	Parameters
	const	float 	mnp 	= 1.0 / 65536.0;			//	Minimum value of a precise step for 16-bit channel
	const	float 	s  		= mnp *  48.0 *  64.0;
	const	float 	n  		= mnp *  48.0 *   2.0;

//	Output Values
	vec4 res_c = gdv( ivec2(0, 0), txdata );

//	Result Values
	vec4 res_v = res_c;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Update Functions
//	----    ----    ----    ----    ----    ----    ----    ----

//	Neighborhoods
	uint[12] nb = uint[12] (
		ub.v0,  ub.v1,  ub.v2,  ub.v3,
		ub.v4,  ub.v5,  ub.v6,  ub.v7,
		ub.v8,  ub.v9,  ub.v10, ub.v11 );

//	Update Ranges
	uint[24] ur = uint[24] (
		ub.v12, ub.v13, ub.v14, ub.v15, 
		ub.v16, ub.v17, ub.v18, ub.v19,	
		ub.v20, ub.v21, ub.v22, ub.v23,
		ub.v24, ub.v25, ub.v26, ub.v27,	
		ub.v28, ub.v29, ub.v30, ub.v31, 
		ub.v32, ub.v33, ub.v34, ub.v35  );

//	Channel I/O
	uint[ 3] ch = uint[ 3] ( 2286157824u, 295261525u, 1713547946u );

//	Update Signs (+/-)
	uint[ 2] us = uint[ 2] ( ub.v36, ub.v37 );

//	Transition Function
	for(uint i = 0u; i < 24u; i++) {
		float nhv = bitring( nh_rings_c, nb[i/2u], (i & 1u) * 16u )[u32_upk( ch[i/8u], 2u, (i*4u+0u) & 31u )];
		if( nhv >= utp( ur[i], 8u, 0u) && nhv <= utp( ur[i], 8u, 1u)) {
			float h = hmp2(res_c[u32_upk( ch[i/8u], 2u, (i*4u+0u) & 31u )],1.2);
			res_v[u32_upk( ch[i/8u], 2u, (i*4u+2u) & 31u )]
			+=	bsn(us[i/16u], ((i*2u+0u) & 31u)) * s * h; }
		if( nhv >= utp( ur[i], 8u, 2u) && nhv <= utp( ur[i], 8u, 3u)) {
			float h = hmp2(res_c[u32_upk( ch[i/8u], 2u, (i*4u+0u) & 31u )],1.2);
			res_v[u32_upk( ch[i/8u], 2u, (i*4u+2u) & 31u )]
			+=	bsn(us[i/16u], ((i*2u+1u) & 31u)) * s * h; } }

//	Decay Curve
	vec4 n4 = sigm(res_v, 0.5) * n * 64.0 + n;
	res_c = res_v - n4;

//	CGOL TEST OVERRIDE
/*
	res_c = gdv( ivec2(0, 0), txdata );
	vec4[2] cgol_val = nbhd( vec2(1.0,0.0), txdata );
	vec4 sum = cgol_val[0];
	vec4 tot = cgol_val[1];
	vec4 res = sum / tot;

	if(res_c[0] >= 0.5 ) { res_c[0] = 1.0; }
	if(res_c[0] <  0.5 ) { res_c[0] = 0.0; }

	if(res_c[1] >= 0.5 ) { res_c[1] = 1.0; }
	if(res_c[1] <  0.5 ) { res_c[1] = 0.0; }

	if(res_c[2] >= 0.5 ) { res_c[2] = 1.0; }
	if(res_c[2] <  0.5 ) { res_c[2] = 0.0; }

	if(res_c[3] >= 0.5 ) { res_c[3] = 1.0; }
	if(res_c[3] <  0.5 ) { res_c[3] = 0.0; }

	if(res[0] <= (1.0 / 8.0)) { res_c[0] = 0.0; }
	if(res[0] >= (4.0 / 8.0)) { res_c[0] = 0.0; }
	if(res[0] >= (3.0 / 8.0) && res[0] <= (3.0 / 8.0) ) { res_c[0] = 1.0; }

	if(res[1] <= (1.0 / 8.0)) { res_c[1] = 0.0; }
	if(res[1] >= (4.0 / 8.0)) { res_c[1] = 0.0; }
	if(res[1] >= (3.0 / 8.0)   && res[1] <= (3.0 / 8.0) ) { res_c[1] = 1.0; }

	if(res[2] <= (1.0 / 8.0)) { res_c[2] = 0.0; }
	if(res[2] >= (4.0 / 8.0)) { res_c[2] = 0.0; }
	if(res[2] >= (3.0 / 8.0)   && res[2] <= (3.0 / 8.0) ) { res_c[2] = 1.0; }

	if(res[3] <= (1.0 / 8.0)) { res_c[3] = 0.0; }
	if(res[3] >= (4.0 / 8.0)) { res_c[3] = 0.0; }
	if(res[3] >= (3.0 / 8.0)   && res[3] <= (3.0 / 8.0) ) { res_c[3] = 1.0; }
/**/

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	if(ub.v63 <= 0) {
		res_c[0] = reseed(0); 
		res_c[1] = reseed(1); 
		res_c[2] = reseed(2); 
		res_c[3] = reseed(3); }

	res_c[3] = 1.0;

	out_col = res_c;

}


