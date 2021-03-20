#version 460
//	Shader developed by Slackermanz:
//		https://www.reddit.com/user/slackermanz/
//		https://github.com/Slackermanz/VulkanAutomata
//		https://www.youtube.com/channel/UCmoNsNuM0M9VsIXfm2cHPiA/videos
//		https://www.shadertoy.com/user/SlackermanzCA
//		https://discord.gg/BCuYCEn
layout(location = 0) out vec4 out_col;
layout(binding = 0) uniform UniBuf {
	uint wsize;
	uint frame;
	uint minfo;
} ub;
layout(binding = 1) uniform sampler2D txdata;

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

float gdv(float x, float y, int v) {
//	Get Div Value: Return the value of a specified pixel
//		x, y : 	Relative integer-spaced coordinates to origin [ 0.0, 0.0 ]
//		v	 :	Colour channel [ 0, 1, 2 ]
	ivec4	dm		= wsize_unpack(ub.wsize);
	vec4 	fc 		= gl_FragCoord;
	vec2	dc		= vec2( dm[0]/dm[2], dm[1]/dm[2] );
	float	cx		= mod(fc[0]+x, dc[0]) + floor(fc[0]/dc[0])*dc[0];
	float	cy		= mod(fc[1]+y, dc[1]) + floor(fc[1]/dc[1])*dc[1];
	vec4 	pxdata 	= texelFetch( txdata, ivec2(cx, cy), 0);
	return 	pxdata[v]; }

vec3 nhd( vec2 nbhd, vec2 ofst, float psn, float thr, int col ) {
//	Neighbourhood: Return information about the specified group of pixels
	float dist 		= 0.0;
	float cval 		= 0.0;
	float c_total 	= 0.0;
	float c_valid 	= 0.0;
	float c_value 	= 0.0;
	for(float i = -nbhd[0]; i <= nbhd[0]; i += 1.0) {
		for(float j = -nbhd[0]; j <= nbhd[0]; j += 1.0) {
			dist = round(sqrt(i*i+j*j));
			if( dist <= nbhd[0] && dist > nbhd[1] && dist != 0.0 ) {
				cval = gdv(i+ofst[0],j+ofst[1],col);
				c_total += psn;
				if( cval > thr ) {
					c_valid += psn;
					cval = psn * cval;
					c_value += cval-fract(cval); } } } } 
	return vec3( c_value, c_valid, c_total ); }

vec3 place(vec3 col_place, ivec4 mi) {
//	Place: Overwrite the provided colour channels at cursor location
	const 	vec4	fc 			= gl_FragCoord;
			float 	place_size 	= 18.0;
			float 	distx 		= (fc[0]-mi[0]) * (fc[0]-mi[0]);
			float 	disty 		= (fc[1]-mi[1]) * (fc[1]-mi[1]);
			float 	dist  		= sqrt(distx+disty);
	if( fc[0] > mi[0] - place_size && fc[0] < mi[0] + place_size ) {
		if( fc[1] > mi[1] - place_size && fc[1] < mi[1] + place_size ) {
			if(dist < place_size) {
				col_place[0] = (mi[2]==1) ? 1.0 : 0.0; 
				col_place[1] = col_place[0];
				col_place[2] = col_place[0];} } }
	if( fc[0] > mi[0] - place_size*2.4 && fc[0] < mi[0] + place_size*2.4) {
		if( fc[1] > mi[1] - place_size*0.25 && fc[1] < mi[1] + place_size*0.25) {
			col_place[0] = (mi[2]==1) ? 1.0 : 0.0; 
			col_place[1] = col_place[0];
			col_place[2] = col_place[0];} }
	if( fc[0] > mi[0] - place_size*0.25 && fc[0] < mi[0] + place_size*0.25) {
		if( fc[1] > mi[1] - place_size*2.4 && fc[1] < mi[1] + place_size*2.4) {
			col_place[0] = (mi[2]==1) ? 1.0 : 0.0; 
			col_place[1] = col_place[0];
			col_place[2] = col_place[0];} }
	if( fc[0] > mi[0] - place_size*0.2 && fc[0] < mi[0] + place_size*0.2) {
		if( fc[1] > mi[1] - place_size*0.2 && fc[1] < mi[1] + place_size*0.2) {
			if(dist < place_size*0.2) {
				col_place[0] = (mi[2]==1) ? 0.0 : 1.0; 
				col_place[1] = col_place[0];
				col_place[2] = col_place[0];} } }
	return col_place; }

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
float reseed(float seed) {
	vec4	fc = gl_FragCoord;
	float 	r0 = get_lump(fc[0], fc[1], 2.0, 	19.0 + mod(ub.frame+seed,17.0), 23.0 + mod(ub.frame+seed,43.0));
	float 	r1 = get_lump(fc[0], fc[1], 12.0, 	13.0 + mod(ub.frame+seed,29.0), 17.0 + mod(ub.frame+seed,31.0));
	float 	r2 = get_lump(fc[0], fc[1], 4.0, 	13.0 + mod(ub.frame+seed,11.0), 51.0 + mod(ub.frame+seed,37.0));
	return clamp((r0+r1)-r2,0.0,1.0); }

void main() {
	vec4	fc 		= gl_FragCoord;
	ivec4 	minfo 	= minfo_unpack(ub.minfo);
	float 	psn		= 65536.0;						//	Precision
	float 	mnp 	= 1.0 / psn;					//	Minimum Precise Value Step

	float 	ref_r 		= gdv( 0.0, 0.0, 0 );
	float 	ref_g 		= gdv( 0.0, 0.0, 1 );
	float 	ref_b 		= gdv( 0.0, 0.0, 2 );

//	Transition speed
	float 	s 		= 1.0;
	float 	sa 		= 1.0;
	float 	sb 		= 0.5;
	float	f		= 0.055;
	float	k		= 0.062;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Transition Functions
//	----    ----    ----    ----    ----    ----    ----    ----

	float[8] lapa;
		lapa[0] = gdv( -1.0,  0.0, 0 ) * 0.2  * psn;
		lapa[1] = gdv(  1.0,  0.0, 0 ) * 0.2  * psn;
		lapa[2] = gdv(  0.0, -1.0, 0 ) * 0.2  * psn;
		lapa[3] = gdv(  0.0,  1.0, 0 ) * 0.2  * psn;
		lapa[4] = gdv( -1.0, -1.0, 0 ) * 0.05 * psn;
		lapa[5] = gdv(  1.0, -1.0, 0 ) * 0.05 * psn;
		lapa[6] = gdv(  1.0,  1.0, 0 ) * 0.05 * psn;
		lapa[7] = gdv( -1.0,  1.0, 0 ) * 0.05 * psn;

	float[8] lapb;
		lapb[0] = gdv( -1.0,  0.0, 1 ) * 0.2  * psn;
		lapb[1] = gdv(  1.0,  0.0, 1 ) * 0.2  * psn;
		lapb[2] = gdv(  0.0, -1.0, 1 ) * 0.2  * psn;
		lapb[3] = gdv(  0.0,  1.0, 1 ) * 0.2  * psn;
		lapb[4] = gdv( -1.0, -1.0, 1 ) * 0.05 * psn;
		lapb[5] = gdv(  1.0, -1.0, 1 ) * 0.05 * psn;
		lapb[6] = gdv(  1.0,  1.0, 1 ) * 0.05 * psn;
		lapb[7] = gdv( -1.0,  1.0, 1 ) * 0.05 * psn;

	for(int i = 0; i < 8; i++) { 
		lapa[i] = lapa[i] - fract(lapa[i]);
		lapb[i] = lapb[i] - fract(lapb[i]); }

	float 	lapa_sum = ref_r * -1.0 * psn;
	float 	lapb_sum = ref_g * -1.0 * psn;

			lapa_sum = lapa_sum - fract(lapa_sum);
			lapb_sum = lapb_sum - fract(lapb_sum);

	for(int i = 0; i < 8; i++) { 
		lapa_sum += lapa[i];
		lapb_sum += lapb[i]; }

	lapa_sum = lapa_sum / (1.0 * psn);
	lapb_sum = lapb_sum / (1.0 * psn);

	float res_r = ref_r + (sa * lapa_sum - (ref_r * ref_g * ref_g) + (f * (1.0 - ref_r)) ) * s;
	float res_g = ref_g + (sb * lapb_sum + (ref_r * ref_g * ref_g) - ((k + f) * ref_g)	 ) * s;
	float res_b = 0.0;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	if(ub.frame == 0 || minfo[2] == 2) { res_r = reseed(0.0); res_g = reseed(1.0); }

	vec3 	col = vec3( res_r, res_g, res_b );
			col = ( minfo[2] == 1 || minfo[2] == 3 ) ? place(col, minfo) : col;

	out_col = vec4(col, 1.0);

}


