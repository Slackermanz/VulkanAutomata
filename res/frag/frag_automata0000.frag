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

//	----    ----    ----    ----    ----    ----    ----    ----

const uint MAX_RADIUS = 16;

//	----    ----    ----    ----    ----    ----    ----    ----

uint u32_upk(uint u32, uint bts, uint off) { return (u32 >> off) & ((1u << bts)-1u); }

float  tp(uint n, float s) 			{ return ((n+1)/256.0) * ((s*0.5)/128.0); }
float bsn(uint v, uint  o) 			{ return float(u32_upk(v,1u,o)*2u)-1.0; }
float utp(uint v, uint  w, uint o) 	{ return tp(u32_upk(v,w,w*o), 105.507401); }

vec4  gdv( ivec2 of, sampler2D tx ) {
	of 		= ivec2(gl_FragCoord) + of;
	of[0] 	= (of[0] + textureSize(tx,0)[0]) & (textureSize(tx,0)[0]-1);
	of[1] 	= (of[1] + textureSize(tx,0)[1]) & (textureSize(tx,0)[1]-1);
	return 	texelFetch( tx, of, 0); }

vec4[2] nbhd( vec2 r, sampler2D tx ) {
//	Precision limit of signed float32 for [n] neighbors (symmetry preservation)
	uint	chk = 2147483648u /
			(	( 	uint( r[0]*r[0]*PI + r[0]*PI + PI	)
				- 	uint( r[1]*r[1]*PI + r[1]*PI		) ) * 128u );
	float	psn = (chk >= 65536u) ? 65536.0 : float(chk);
	vec4	a = vec4(0.0,0.0,0.0,0.0);
	vec4 	b = vec4(0.0,0.0,0.0,0.0);
	for(float i = -r[0]; i <= r[0]; i+=1.0) {
		for(float j = -r[0]; j <= r[0]; j+=1.0) {
			float	d = round(sqrt(i*i+j*j));
			float	w = 1.0;
			if( d <= r[0] && d > r[1] ) {
				vec4 t  = gdv( ivec2(i,j), tx ) * w * psn;
					 a += t - fract(t);
					 b += w * psn; } } }
	return vec4[2](a, b); }

vec4 bitring(vec4[MAX_RADIUS][2] rings, uint bits) {
	vec4 sum = vec4(0.0,0.0,0.0,0.0);
	vec4 tot = vec4(0.0,0.0,0.0,0.0);
	for(int i = 0; i < MAX_RADIUS; i++) {
		if(u32_upk(bits, 1, i) == 1) { sum += rings[i][0]; tot += rings[i][1]; } }
	return sum / tot; }

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
	const	float 	r0 = get_lump(fx, fy,  6.0, 19.0 + mod(ub.v63+seed,17.0), 23.0 + mod(ub.v63+seed,43.0));
	const	float 	r1 = get_lump(fx, fy, 24.0, 13.0 + mod(ub.v63+seed,29.0), 17.0 + mod(ub.v63+seed,31.0));
	const	float 	r2 = get_lump(fx, fy, 12.0, 13.0 + mod(ub.v63+seed,11.0), 51.0 + mod(ub.v63+seed,37.0));
	const	float 	r3 = get_lump(fx, fy, 18.0, 29.0 + mod(ub.v63+seed, 7.0), 61.0 + mod(ub.v63+seed,28.0));
	return clamp( sqrt((r0+r1)*r3*2.0)-r2 , 0.0, 1.0); }

void main() {

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Setup
//	----    ----    ----    ----    ----    ----    ----    ----

	const 	ivec2	origin  = ivec2(0, 0);
	const	float 	mnp 	= 1.0 / 65536.0;			//	Minimum value of a precise step for 16-bit channel
	const	vec4	ref_c	= gdv( origin, txdata );	//	Origin value references

//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----

	const	uint[4] scd = uint[4]
	(	32766u, 		3472619019u, 	32752u, 		3675719802u 	);

	const	uint[8] ubv = uint[8]
	(	39879523u, 		526972426u, 	2727874005u, 	1461826227u, 
		1300644632u, 	1298224u, 		95419984u, 		823214418u		);

	const	uint[1] ubi = uint[1]
	(	2390857921u 													);

//	Parameters
	const	float 	s  = mnp *  64.0 *  96.0;
	const	float 	n  = mnp *  64.0 *  16.0;

//	Output Values
	vec4 res_c = ref_c;

//	NH Rings
	vec4[MAX_RADIUS][2] nh_rings_c;
	for(int i = 0; i < MAX_RADIUS; i++) {
		nh_rings_c[i] = nbhd( vec2(i+1,i), txdata ); }

	vec4[4] nhv_c;
	for(int i = 0; i < 4; i++) {
		nhv_c[i] = bitring(nh_rings_c, scd[i]); }

//	----    ----    ----    ----    ----    ----    ----    ----
//	Update Functions
//	----    ----    ----    ----    ----    ----    ----    ----

	const int bt = 8;		// Update Range Bits
	const int mt = 4;		// Neighborhoods
	const int ut = bt/mt;	// Range  UBV index
	const int st = ut*2;	// Update UBI offset
	const int ct = 3;		// Color Channels Used

	for(int i = 0; i < mt; i++) {
		for(int j = 0; j < ct; j++) {
			if( nhv_c[i][j] >= utp(ubv[i*ut+0],bt,0) && nhv_c[i][j] <= utp(ubv[i*ut+0],bt,1) ) { res_c[j] += bsn(ubi[0], i*st+0)*s; }
			if( nhv_c[i][j] >= utp(ubv[i*ut+0],bt,2) && nhv_c[i][j] <= utp(ubv[i*ut+0],bt,3) ) { res_c[j] += bsn(ubi[0], i*st+1)*s; }
			if( nhv_c[i][j] >= utp(ubv[i*ut+1],bt,0) && nhv_c[i][j] <= utp(ubv[i*ut+1],bt,1) ) { res_c[j] += bsn(ubi[0], i*st+2)*s; }
			if( nhv_c[i][j] >= utp(ubv[i*ut+1],bt,2) && nhv_c[i][j] <= utp(ubv[i*ut+1],bt,3) ) { res_c[j] += bsn(ubi[0], i*st+3)*s; } } }

	res_c -= n;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	if(ub.v63 <= 0) {
		res_c[0] = reseed(0); 
		res_c[1] = reseed(1); 
		res_c[2] = reseed(2); }

	out_col = res_c;

}


