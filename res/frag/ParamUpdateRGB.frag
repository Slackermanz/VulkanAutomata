#version 460
//	Shader developed by Slackermanz:
//		https://twitter.com/slackermanz
//		https://www.reddit.com/user/slackermanz/
//		https://github.com/Slackermanz/VulkanAutomata
//		https://www.youtube.com/channel/UCmoNsNuM0M9VsIXfm2cHPiA/videos
//		https://www.shadertoy.com/user/SlackermanzCA
//		https://discord.gg/BCuYCEn
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
	float 	r0 = get_lump(fc[0], fc[1],  6.0, 19.0 + mod(ub.frame+seed,17.0), 23.0 + mod(ub.frame+seed,43.0));
	float 	r1 = get_lump(fc[0], fc[1], 20.0, 13.0 + mod(ub.frame+seed,29.0), 17.0 + mod(ub.frame+seed,31.0));
	float 	r2 = get_lump(fc[0], fc[1], 14.0, 13.0 + mod(ub.frame+seed,11.0), 51.0 + mod(ub.frame+seed,37.0));
	return clamp((r0+r1)-r2,0.0,1.0); }

void main() {

	float 	psn		= 65536.0;					//	Texture Precision
	ivec4 	minfo 	= minfo_unpack(ub.minfo);	//	Mouse State Information
	ivec2	origin  = ivec2(0,0);

	float 	ref_r 	= gdv( origin, 0 );
	float 	ref_g 	= gdv( origin, 1 );
	float 	ref_b 	= gdv( origin, 2 );

	vec3	rlnh_r 	= nhd_rule(ivec2(22,21), origin, psn, 0.0, 0);
	float	rl_r_av	= rlnh_r[0] / rlnh_r[2];

	vec3	rlnh_g 	= nhd_rule(ivec2(22,21), origin, psn, 0.0, 1);
	float	rl_g_av	= rlnh_g[0] / rlnh_g[2];

	vec3	rlnh_b 	= nhd_rule(ivec2(22,21), origin, psn, 0.0, 2);
	float	rl_b_av	= rlnh_b[0] / rlnh_b[2];

	vec3	dtnh_r 	= nhd_data(ivec2(12,11), origin, psn, 0.0, 0);
	float	dt_r_av	= dtnh_r[0] / dtnh_r[2];

	vec3	dtnh_g 	= nhd_data(ivec2(12,11), origin, psn, 0.0, 1);
	float	dt_g_av	= dtnh_g[0] / dtnh_g[2];

	vec3	dtnh_b 	= nhd_data(ivec2(12,11), origin, psn, 0.0, 2);
	float	dt_b_av	= dtnh_b[0] / dtnh_b[2];

	float 	res_r 	= ref_r;
	float 	res_g 	= ref_g;
	float 	res_b 	= ref_b;

	float	vari_r = abs(dt_r_av - ref_r);
	float	vari_g = abs(dt_g_av - ref_g);
	float	vari_b = abs(dt_b_av - ref_b);

	int		varidx_max = 0;
	int		varidx_min = 0;
	float	maxvar = vari_r;
	float	minvar = vari_r;

	float 	s0 = 0.0020;
	float 	s1 = 0.0020;
	float 	li = 0.0004;
	float 	lu = 0.0015;

/*	if(minvar > vari_g) { minvar = vari_g; varidx_min = 1; }
	if(minvar > vari_b) { minvar = vari_b; varidx_min = 2; }
	if(varidx_min == 0) { res_r = res_r - s0; }
	if(varidx_min == 1) { res_g = res_g - s0; }
	if(varidx_min == 2) { res_b = res_b - s0; }

	if(maxvar < vari_g) { maxvar = vari_g; varidx_max = 1; }
	if(maxvar < vari_b) { maxvar = vari_b; varidx_max = 2; }
	if(varidx_max == 0) { res_r = res_r + s1; }
	if(varidx_max == 1) { res_g = res_g + s1; }
	if(varidx_max == 2) { res_b = res_b + s1; }
/**/
//	Interpolate
	float	inp_r = (res_r 	  	* 1.0 	+ rl_g_av 	*  li 	+ rl_b_av 	*  li	) / ( 1.0 + li * 2.0 );
	float	inp_g = (rl_r_av 	*  li 	+ res_g 	* 1.0 	+ rl_b_av 	*  li	) / ( 1.0 + li * 2.0 );
	float	inp_b = (rl_r_av 	*  li	+ rl_g_av 	*  li 	+ res_b 	* 1.0	) / ( 1.0 + li * 2.0 );
	res_r = inp_r;
	res_g = inp_g;
	res_b = inp_b;
/**/
//	Interpolate
			inp_r = (res_r 	  	* 1.0 	+ dt_g_av 	*  li 	+ dt_b_av 	*  li	) / ( 1.0 + li * 2.0 );
			inp_g = (dt_r_av 	*  li 	+ res_g 	* 1.0 	+ dt_b_av 	*  li	) / ( 1.0 + li * 2.0 );
			inp_b = (dt_r_av 	*  li	+ dt_g_av 	*  li 	+ res_b 	* 1.0	) / ( 1.0 + li * 2.0 );
	res_r = inp_r;
	res_g = inp_g;
	res_b = inp_b;
/**/
//	Unterpolate
	float	unp_r = (res_r 	  	* 1.0 	+ res_g 	* -lu 	+ res_b 	* -lu	) / ( 1.0 - lu * 2.0 );
	float	unp_g = (res_r 		* -lu 	+ res_g 	* 1.0 	+ res_b 	* -lu	) / ( 1.0 - lu * 2.0 );
	float	unp_b = (res_r 		* -lu	+ res_g 	* -lu 	+ res_b 	* 1.0	) / ( 1.0 - lu * 2.0 );
	res_r = unp_r;
	res_g = unp_g;
	res_b = unp_b;
/**/
	res_r = (res_r + dt_g_av * 0.002 + rl_r_av * 0.001) / 1.003;
	res_g = (res_g + dt_b_av * 0.002 + rl_g_av * 0.001) / 1.003;
	res_b = (res_b + dt_r_av * 0.002 + rl_b_av * 0.001) / 1.003;
/**/

//	res_r = rule_val( origin , 0 ); res_g = rule_val( origin , 1 ); res_b = rule_val( origin , 2 );

	if(ub.frame == 0 || minfo[3] == 1) { 
		res_r = reseed(0); res_g = reseed(1); res_b = reseed(2); }
	if(minfo[3] == 2) { 
		res_r = 0.0; res_g = 0.0; res_b = 0.0; }

	out_col = vec4(clamp(res_r,0.0,1.0), clamp(res_g,0.0,1.0), clamp(res_b,0.0,1.0), 1.0);
}


