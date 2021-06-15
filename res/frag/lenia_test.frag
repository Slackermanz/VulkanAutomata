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
const uint MAX_NH_SIZE = 16;

//	----    ----    ----    ----    ----    ----    ----    ----

layout(constant_id 	=  0) const uint SCUI00 = 0;	layout(constant_id 	=  1) const uint SCUI01 = 0;
layout(constant_id 	=  2) const uint SCUI02 = 0;	layout(constant_id 	=  3) const uint SCUI03 = 0;
layout(constant_id 	=  4) const uint SCUI04 = 0;	layout(constant_id 	=  5) const uint SCUI05 = 0;
layout(constant_id 	=  6) const uint SCUI06 = 0;	layout(constant_id 	=  7) const uint SCUI07 = 0;
layout(constant_id 	=  8) const uint SCUI08 = 0;	layout(constant_id 	=  9) const uint SCUI09 = 0;
layout(constant_id 	= 10) const uint SCUI10 = 0;	layout(constant_id 	= 11) const uint SCUI11 = 0;
layout(constant_id 	= 12) const uint SCUI12 = 0;	layout(constant_id 	= 13) const uint SCUI13 = 0;
layout(constant_id 	= 14) const uint SCUI14 = 0;	layout(constant_id 	= 15) const uint SCUI15 = 0;
layout(constant_id 	= 16) const uint SCUI16 = 0;	layout(constant_id 	= 17) const uint SCUI17 = 0;
layout(constant_id 	= 18) const uint SCUI18 = 0;	layout(constant_id 	= 19) const uint SCUI19 = 0;
layout(constant_id 	= 20) const uint SCUI20 = 0;	layout(constant_id 	= 21) const uint SCUI21 = 0;
layout(constant_id 	= 22) const uint SCUI22 = 0;	layout(constant_id 	= 23) const uint SCUI23 = 0;
layout(constant_id 	= 24) const uint SCUI24 = 0;	layout(constant_id 	= 25) const uint SCUI25 = 0;
layout(constant_id 	= 26) const uint SCUI26 = 0;	layout(constant_id 	= 27) const uint SCUI27 = 0;
layout(constant_id 	= 28) const uint SCUI28 = 0;	layout(constant_id 	= 29) const uint SCUI29 = 0;
layout(constant_id 	= 30) const uint SCUI30 = 0;	layout(constant_id 	= 31) const uint SCUI31 = 0;
layout(constant_id 	= 32) const uint SCUI32 = 0;	layout(constant_id 	= 33) const uint SCUI33 = 0;
layout(constant_id 	= 34) const uint SCUI34 = 0;	layout(constant_id 	= 35) const uint SCUI35 = 0;
layout(constant_id 	= 36) const uint SCUI36 = 0;	layout(constant_id 	= 37) const uint SCUI37 = 0;
layout(constant_id 	= 38) const uint SCUI38 = 0;	layout(constant_id 	= 39) const uint SCUI39 = 0;
layout(constant_id 	= 40) const uint SCUI40 = 0;	layout(constant_id 	= 41) const uint SCUI41 = 0;
layout(constant_id 	= 42) const uint SCUI42 = 0;	layout(constant_id 	= 43) const uint SCUI43 = 0;
layout(constant_id 	= 44) const uint SCUI44 = 0;	layout(constant_id 	= 45) const uint SCUI45 = 0;
layout(constant_id 	= 46) const uint SCUI46 = 0;	layout(constant_id 	= 47) const uint SCUI47 = 0;

layout(location 	=  0) out 		vec4 out_col;
layout(binding 		=  1) uniform 	sampler2D txdata;
layout(binding 		=  2) uniform 	sampler2D txpara;
layout(binding 		=  0) uniform 	UniBuf {
	uint wsize;
	uint frame;
	uint minfo;
	uint i0;  uint i1;  uint i2;  uint i3;
	uint v0;  uint v1;  uint v2;  uint v3;	uint v4;  uint v5;  uint v6;  uint v7;
	uint v8;  uint v9;  uint v10; uint v11;	uint v12; uint v13; uint v14; uint v15;
	uint v16; uint v17; uint v18; uint v19;	uint v20; uint v21; uint v22; uint v23;
	uint v24; uint v25; uint v26; uint v27;	uint v28; uint v29; uint v30; uint v31;
	uint v32; uint v33; uint v34; uint v35;	uint v36; uint v37; uint v38; uint v39;
	uint v40; uint v41; uint v42; uint v43;	uint v44; uint v45; uint v46; uint v47;
	float scale;
	float zoom; } ub;

uint u32_upk(uint u32, uint bts, uint off) { return (u32 >> off) & ((1 << bts)-1); }

float tp(uint n) {
	const	uint	wx 		= u32_upk(ub.wsize, 12,  0);
	const	uint	wm 		= u32_upk(ub.wsize,  4, 28);
			float	pscale	= ub.scale * 0.5;
	if(wm == 1) {	pscale = (((gl_FragCoord[0] / wx) + ub.zoom) * (ub.scale / (1.0 + ub.zoom * 2.0))) * 1.0; }
return ((n+1)/256.0) * (pscale/128.0); }

vec3 gdc( ivec2 of, sampler2D tx ) {
	const	float 	fx 		= gl_FragCoord[0];
	const	float 	fy 		= gl_FragCoord[1];
	const	uint	wx 		= u32_upk(ub.wsize, 12,  0);
	const	uint	wy 		= u32_upk(ub.wsize, 12, 12);
	const	uint	wv 		= u32_upk(ub.wsize,  4, 24);
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
//	----    ----    ----    ----    ----    ----    ----    ----
vec3 place( vec3 col ) {
	const	float 	fx 		= gl_FragCoord[0];
	const	float 	fy 		= gl_FragCoord[1];
	const	uint	mx 		= u32_upk(ub.minfo, 12,  4);
	const	uint	my 		= u32_upk(ub.minfo, 12, 16);
	const	uint	mb 		= u32_upk(ub.minfo,  4,  0);
	const	float 	psize 	= 28.0;
	const	float 	distx 	= (fx-mx) * (fx-mx);
	const	float 	disty 	= (fy-my) * (fy-my);
	const	float 	dist  	= sqrt(distx+disty);
	const	float 	colval	= (mb == 1) ? 1.0 : 0.0;
	if(dist <= psize) { col[0] = colval; col[1] = colval; col[2] = colval; }
	return col; }

vec3 sym_seed( vec3 col ) {
	const	float 	fx 		= gl_FragCoord[0];
	const	float 	fy 		= gl_FragCoord[1];
	const	uint	wx 		= u32_upk(ub.wsize, 12,  0);
	const	uint	wy 		= u32_upk(ub.wsize, 12, 12);
	const	float 	psize 	= 96.0;
	const	float 	distx 	= (fx-wx/2) * (fx-wx/2);
	const	float 	disty 	= (fy-wy/2) * (fy-wy/2);
	const	float 	dist  	= sqrt(distx+disty);
	if(dist <= psize) 		{ col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; }
	if(dist <= psize*0.9) 	{ col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; }
	if(dist <= psize*0.8) 	{ col[0] = 1.0; col[1] = 0.0; col[2] = 0.0; }
	if(dist <= psize*0.7) 	{ col[0] = 0.0; col[1] = 1.0; col[2] = 0.0; }
	if(dist <= psize*0.6) 	{ col[0] = 0.0; col[1] = 0.0; col[2] = 1.0; }
	if(dist <= psize*0.6) 	{ col[0] = 1.0; col[1] = 1.0; col[2] = 0.0; }
	if(dist <= psize*0.5) 	{ col[0] = 1.0; col[1] = 0.0; col[2] = 1.0; }
	if(dist <= psize*0.4) 	{ col[0] = 0.0; col[1] = 1.0; col[2] = 1.0; }
	if(dist <= psize*0.3) 	{ col[0] = 0.5; col[1] = 0.5; col[2] = 0.5; }
	if(dist <= psize*0.2) 	{ col[0] = 0.0; col[1] = 0.0; col[2] = 1.0; }
	return col; }
//	----    ----    ----    ----    ----    ----    ----    ----
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
	const	float 	r0 = get_lump(fx, fy,  6.0, 19.0 + mod(ub.frame+seed,17.0), 23.0 + mod(ub.frame+seed,43.0));
	const	float 	r1 = get_lump(fx, fy, 24.0, 13.0 + mod(ub.frame+seed,29.0), 17.0 + mod(ub.frame+seed,31.0));
	const	float 	r2 = get_lump(fx, fy, 12.0, 13.0 + mod(ub.frame+seed,11.0), 51.0 + mod(ub.frame+seed,37.0));
	const	float 	r3 = get_lump(fx, fy, 18.0, 29.0 + mod(ub.frame+seed, 7.0), 61.0 + mod(ub.frame+seed,28.0));
	return clamp( sqrt((r0+r1)*r3*2.0)-r2 , 0.0, 1.0); }

//	----    ----    ----    ----    ----    ----    ----    ----


void main() {

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Setup
//	----    ----    ----    ----    ----    ----    ----    ----

	const	float	fx 		= gl_FragCoord[0];
	const	float	fy 		= gl_FragCoord[1];
	const 	ivec2	origin  = ivec2(0, 0);
	const	float 	mnp 	= 1.0 / 65536.0;			//	Minimum value of a precise step for 16-bit channel
	const	uint	wx 		= u32_upk(ub.wsize, 12,  0);
	const	uint	wy 		= u32_upk(ub.wsize, 12, 12);
	const	uint	wv 		= u32_upk(ub.wsize,  4, 24);
	const	uint	wm 		= u32_upk(ub.wsize,  4, 28);
	const	uint	mx 		= u32_upk(ub.minfo, 12,  4);
	const	uint	my 		= u32_upk(ub.minfo, 12, 16);
	const	uint	mb 		= u32_upk(ub.minfo,  4,  0);
	const	uint	mc 		= u32_upk(ub.minfo,  4, 28);
	const	float 	pidx	= floor((fx*wv)/wx)			//	Panel Division Index
							+ floor((fy*wv)/wy)*wv;
	const	vec3	ref_c	= gdc( origin, txdata );	//	Origin value references
	const	vec3	par_c	= gdc( origin, txpara );


//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----

//	Parameters
	const float dt 		= 0.100;    // time step
	const float mu 		= tp(u32_upk(ub.v0, 8, 0))*2.0-tp(u32_upk(ub.v4, 8, 0));
	const float sigma 	= tp(u32_upk(ub.v1, 8, 0))*2.0-tp(u32_upk(ub.v5, 8, 0));

//	Output Values
	vec3 res_c = ref_c;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Transition Functions
//	----    ----    ----    ----    ----    ----    ----    ----

	vec2  avgnh		= nbhd(ivec2(15,0), txdata, 0);
	float avg		= avgnh[0] / avgnh[1];
	float growth 	= bell(avg, mu, sigma) * tp(u32_upk(ub.v2, 8, 0))*2.0 - tp(u32_upk(ub.v3, 8, 0));
	float c 		= clamp(ref_c[0] + dt * growth,0.0,1.0);

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	res_c[0] = c;
	res_c[1] = res_c[0];
	res_c[2] = res_c[0];

	if(ub.frame == 0 || mc == 1) {
		res_c[0] = reseed(0); 
		res_c[1] = reseed(1); 
		res_c[2] = reseed(2); }

	if(mc == 2) {
		res_c[0] = 0.0;
		res_c[1] = 0.0;
		res_c[2] = 0.0; }

	if(mc == 3) { res_c = sym_seed( res_c ); }

	res_c = ( mb == 1 || mb == 3 ) ? place( res_c ) : res_c;

	if(mc == 14) {
		int subwindow = 3;
		if(fx > (wx/subwindow)*(subwindow-1) && fy > (wy/subwindow)*(subwindow-1)) {
			float wsx = ((fx) * (subwindow-1)) + wx;
			float wsy = ((fy) * (subwindow-1)) + wy;
			res_c = gdc( ivec2(wsx, wsy), txpara ); } }

	out_col = vec4(res_c, 1.0);

}


