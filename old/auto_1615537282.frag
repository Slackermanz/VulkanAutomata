#version 460
#extension GL_ARB_separate_shader_objects : enable
//	Shader developed by Slackermanz:
//		https://www.reddit.com/user/slackermanz/
//		https://github.com/Slackermanz/VulkanAutomata
layout(location = 0) out vec4 out_col;
layout(binding = 0) uniform sampler2D txdata;
layout(binding = 1) uniform init_ub {
    float w;		//	Application Surface Width
	float h;		//	Application Surface Height
	float seed;		//	Implementation-variable usage
	float frame;	//	Index number of all generated frames
	float clicks;	//	Sum of MW_Up (+1.0) and MW_Down (-1.0)
	float mx;		//	Relative Mouse X location
	float my;		//	Relative Mouse Y location
	float mlb;		//	LMB Pressed
	float mrb;		//	RMB Pressed
	float div;		//	Number of toroidal divisions of the surface/medium
//	32 values for mutatative / evolutionary methods
	float v0; 	float v1; 	float v2; 	float v3;
	float v4; 	float v5; 	float v6; 	float v7;
	float v8; 	float v9; 	float v10; 	float v11;
	float v12; 	float v13; 	float v14; 	float v15;
	float v16; 	float v17; 	float v18; 	float v19;
	float v20; 	float v21; 	float v22; 	float v23;
	float v24; 	float v25; 	float v26; 	float v27;
	float v28; 	float v29; 	float v30; 	float v31;
	float v32; 	float v33; 	float v34; 	float v35;
	float v36; 	float v37; 	float v38; 	float v39;
	float v40; 	float v41; 	float v42; 	float v43;
	float v44; 	float v45; 	float v46; 	float v47;
	float v48; 	float v49; 	float v50; 	float v51;
} ub;
float gdv(float x, float y, int v, float div) {
//	Get Div Value: Return the value of a specified pixel
//		x, y : 	Relative integer-spaced coordinates to origin [ 0.0, 0.0 ]
//		v	 :	Colour channel [ 0, 1, 2 ]
//		div	 :	Integer-spaced number of toroidal divisions of the surface/medium
	vec4	fc 		= gl_FragCoord;
	vec2 	dm 		= { ub.w, ub.h };
	float 	divx	= dm[0] / div;
	float 	divy	= dm[1] / div;
	float 	pxo 	= 1.0 / dm[0];
	float 	pyo 	= 1.0 / dm[1];
	float 	fcxo 	= fc[0] + x;
	float 	fcyo 	= fc[1] + y;
	float 	fcx 	= (mod(fcxo,divx) + floor(fc[0]/divx)*divx ) * pxo;
	float 	fcy 	= (mod(fcyo,divy) + floor(fc[1]/divy)*divy ) * pyo;
	vec4 	pxdata 	= texture( txdata, vec2(fcx, fcy) );
	return pxdata[v];
}
vec3 place(vec3 col_place) {
//	Place: Overwrite the provided colour channels at cursor location
	const 	vec4	fc 			= gl_FragCoord;
			float 	place_size 	= 38.0;
			float 	distx 		= (fc[0]-ub.mx) * (fc[0]-ub.mx);
			float 	disty 		= (fc[1]-ub.my) * (fc[1]-ub.my);
			float 	dist  		= sqrt(distx+disty);

	if(ub.mlb == 1.0 || ub.mrb == 1.0) {
		if( fc[0] > ub.mx - place_size && fc[0] < ub.mx + place_size ) {
			if( fc[1] > ub.my - place_size && fc[1] < ub.my + place_size ) {
				if(dist < place_size) {
					col_place[0] = ub.mlb-ub.mrb; 
					col_place[1] = col_place[0];
					col_place[2] = col_place[0];} } }

		if( fc[0] > ub.mx - place_size*2.4 && fc[0] < ub.mx + place_size*2.4) {
			if( fc[1] > ub.my - place_size*0.25 && fc[1] < ub.my + place_size*0.25) {
				col_place[0] = ub.mlb-ub.mrb; 
				col_place[1] = col_place[0];
				col_place[2] = col_place[0];} }

		if( fc[0] > ub.mx - place_size*0.25 && fc[0] < ub.mx + place_size*0.25) {
			if( fc[1] > ub.my - place_size*2.4 && fc[1] < ub.my + place_size*2.4) {
				col_place[0] = ub.mlb-ub.mrb; 
				col_place[1] = col_place[0];
				col_place[2] = col_place[0];} }

		if( fc[0] > ub.mx - place_size*0.2 && fc[0] < ub.mx + place_size*0.2) {
			if( fc[1] > ub.my - place_size*0.2 && fc[1] < ub.my + place_size*0.2) {
				if(dist < place_size*0.2) {
					col_place[0] = 1.0-(ub.mlb-ub.mrb); 
					col_place[1] = col_place[0];
					col_place[2] = col_place[0];} } } }

	return col_place;
}
vec3 nhd( vec2 nbhd, vec2 ofst, float psn, float thr, int col, float div ) {
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
				cval = gdv(i+ofst[0],j+ofst[1],col,div);
				c_total += psn;
				if( cval > thr ) {
					c_valid += psn;
					cval = psn * cval;
					c_value += cval-fract(cval); } } } } 
	return vec3( c_value, c_valid, c_total );
}
float get_xc(float x, float y, float xmod) {
//	Used to reseed the surface with noise
	float sq = sqrt(mod(x*y+y, xmod)) / sqrt(xmod);
	float xc = mod((x*x)+(y*y), xmod) / xmod;
	return clamp((sq+xc)*0.5, 0.0, 1.0);
}
float shuffle(float x, float y, float xmod, float val) {
//	Used to reseed the surface with noise
	val = val * mod( x*y + x, xmod );
	return (val-floor(val));
}
float get_xcn(float x, float y, float xm0, float xm1, float ox, float oy) {
//	Used to reseed the surface with noise
	float  xc = get_xc(x+ox, y+oy, xm0);
	return shuffle(x+ox, y+oy, xm1, xc);
}
float get_lump(float x, float y, float nhsz, float xm0, float xm1) {
//	Used to reseed the surface with noise
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
	return xcaf;
}
float reseed2(float seed) {
//	Used to reseed the surface with noise
	vec4	fc = gl_FragCoord;
	float 	r0 = get_lump(fc[0], fc[1], 2.0, 	19.0 + mod(ub.frame+seed,17.0), 23.0 + mod(ub.frame+seed,43.0));
	float 	r1 = get_lump(fc[0], fc[1], 12.0, 	13.0 + mod(ub.frame+seed,29.0), 17.0 + mod(ub.frame+seed,31.0));
	float 	r2 = get_lump(fc[0], fc[1], 4.0, 	13.0 + mod(ub.frame+seed,11.0), 51.0 + mod(ub.frame+seed,37.0));
	return clamp((r0+r1)-r2,0.0,1.0);
}
float split_ring_nh(vec2 nh, float[12] e0, float[12] e1){
	float e0_sum = 0.0;
	float e1_sum = 0.0;
	for(int i = int(nh[1]); i < int(nh[0]); i++) {
		e0_sum = e0_sum + e0[i];
		e1_sum = e1_sum + e1[i];
	}
	return e0_sum / e1_sum;
}
void main() {
//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----
	vec4	fc 		= gl_FragCoord;							// 	Frag Coords
	vec2 	dm 		= { ub.w, ub.h };						//	Surface Dimensions
	const 	int 	VMX 	= 52;							//	Evolution Variables
	const 	int 	SMX 	= 12;							//	Maximum NH Size
	float 		psn		= 250.0;							//	Precision
	float 		mnp		= 0.004;							//	Minimum Precision Value : (1.0 / psn);
	float		div		= ub.div;							//	Toroidal Surface Divisions
	float 		divi 	= floor((fc[0]*div)/(dm[0]))		
						+ floor((fc[1]*div)/(dm[1]))*div;	//	Division Index
	float 		dspace = (divi+1.0)/(div*div);
				dspace = (div == 1.0) ? 0.5 : dspace;		//	Division Weight
	float		cs		= (ub.clicks)*0.001;				//	Used for testing/debugging small value variations
	vec3 		col 	= vec3( 0.0, 0.0, 0.0 );			//	Final colour value output container

//	Uniform Buffer V number
	float[VMX] 	ubvn 	= float[VMX]
	(	ub.v0,	ub.v1,	ub.v2,	ub.v3,
		ub.v4,	ub.v5,	ub.v6,	ub.v7,
		ub.v8,	ub.v9,	ub.v10,	ub.v11,
		ub.v12,	ub.v13,	ub.v14,	ub.v15,
		ub.v16,	ub.v17,	ub.v18,	ub.v19,
		ub.v20,	ub.v21,	ub.v22,	ub.v23,
		ub.v24,	ub.v25,	ub.v26,	ub.v27,
		ub.v28,	ub.v29,	ub.v30,	ub.v31,
		ub.v32,	ub.v33,	ub.v34,	ub.v35,
		ub.v36,	ub.v37,	ub.v38,	ub.v39,
		ub.v40,	ub.v41,	ub.v42,	ub.v43,
		ub.v44,	ub.v45,	ub.v46,	ub.v47,
		ub.v48,	ub.v49,	ub.v50,	ub.v51 	);

//	Division Weighted V number
	float[VMX] 	dvmd;						
	for(int i = 0; i < VMX; i++) { dvmd[i] = ubvn[i] * dspace; }

//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----

//	Get the reference frame's origin pixel values
	float	res_r	= gdv( 0.0, 0.0, 0, div );
	float	res_g	= gdv( 0.0, 0.0, 1, div );
	float	res_b	= gdv( 0.0, 0.0, 2, div );

//	Transition speed
	float 	s 		= mnp * 24.0;
	float 	sr 		= s * dvmd[48] * 4.0;
	float 	sg 		= s * dvmd[49] * 4.0;
	float 	sb 		= s * dvmd[50] * 4.0;

//	Layer interpolation rates
	float 	li 		= mnp * 1.0;
	float 	lc 		= mnp * 24.0;
	float 	lr 		= mnp * 32.0;

//	NHs
	float[SMX]	ring_e0_r;
	float[SMX]	ring_e1_r;
	float[SMX]	ring_e0_g;
	float[SMX]	ring_e1_g;
	float[SMX]	ring_e0_b;
	float[SMX]	ring_e1_b;

	vec3 ring_r;
	vec3 ring_g;
	vec3 ring_b;

	for(int i = 0; i < SMX; i++) {
		ring_r			= nhd( vec2( float(i+1), float(i) ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
		ring_e0_r[i] 	= ring_r[0];
		ring_e1_r[i] 	= ring_r[2];
		ring_g			= nhd( vec2( float(i+1), float(i) ), vec2( 0.0, 0.0 ), psn, 0.0, 1, div );
		ring_e0_g[i] 	= ring_g[0];
		ring_e1_g[i] 	= ring_g[2];
		ring_b			= nhd( vec2( float(i+1), float(i) ), vec2( 0.0, 0.0 ), psn, 0.0, 2, div );
		ring_e0_b[i] 	= ring_b[0];
		ring_e1_b[i] 	= ring_b[2]; }

//	----    ----    ----    ----    ----    ----    ----    ----
//	Transition Functions
//	----    ----    ----    ----    ----    ----    ----    ----

	vec2[4] nhdr = vec2[4] (
		vec2( 1.0, 0.0 ),
		vec2( 3.0, 0.0 ),
		vec2( 6.0, 3.0 ),
		vec2( 12.0, 2.0 )
	);
	vec2[4] nhdg = vec2[4] (
		vec2( 2.0, 0.0 ),
		vec2( 4.0, 2.0 ),
		vec2( 5.0, 0.0 ),
		vec2( 9.0, 5.0 )
	);
	vec2[4] nhdb = vec2[4] (
		vec2( 3.0, 2.0 ),
		vec2( 5.0, 0.0 ),
		vec2( 7.0, 3.0 ),
		vec2( 12.0, 8.0 )
	);

	float[9] nhdt;
		nhdt[0]  = split_ring_nh( nhdr[0], ring_e0_r, ring_e1_r );
		nhdt[1]  = split_ring_nh( nhdg[0], ring_e0_g, ring_e1_g );
		nhdt[2]  = split_ring_nh( nhdb[0], ring_e0_b, ring_e1_b );

		nhdt[3]  = split_ring_nh( nhdr[1], ring_e0_r, ring_e1_r );
		nhdt[4]  = split_ring_nh( nhdg[1], ring_e0_g, ring_e1_g );
		nhdt[5]  = split_ring_nh( nhdb[1], ring_e0_b, ring_e1_b );

		nhdt[6]  = split_ring_nh( nhdr[2], ring_e0_r, ring_e1_r );
		nhdt[7]  = split_ring_nh( nhdg[2], ring_e0_g, ring_e1_g );
		nhdt[8]  = split_ring_nh( nhdb[2], ring_e0_b, ring_e1_b );

/*		nhdt[9]	 = split_ring_nh( nhdr[3], ring_e0_r, ring_e1_r );
		nhdt[10] = split_ring_nh( nhdg[3], ring_e0_g, ring_e1_g );
		nhdt[11] = split_ring_nh( nhdb[3], ring_e0_b, ring_e1_b );*/


	if(	nhdt[0]   >= dvmd[0]  && nhdt[0]   <= dvmd[1] 	) { res_r += sr; }
	if(	nhdt[0]   >= dvmd[2]  && nhdt[0]   <= dvmd[3] 	) { res_r -= sr; }
	if(	nhdt[1]   >= dvmd[4]  && nhdt[1]   <= dvmd[5] 	) { res_g += sg; }
	if(	nhdt[1]   >= dvmd[6]  && nhdt[1]   <= dvmd[7] 	) { res_g -= sg; }
	if(	nhdt[2]   >= dvmd[8]  && nhdt[2]   <= dvmd[9] 	) { res_b += sb; }
	if(	nhdt[2]   >= dvmd[10] && nhdt[2]   <= dvmd[11] 	) { res_b -= sb; }

	if(	nhdt[3]   >= dvmd[12]  && nhdt[3]   <= dvmd[13] ) { res_r += sr; }
	if(	nhdt[3]   >= dvmd[14]  && nhdt[3]   <= dvmd[15] ) { res_r -= sr; }
	if(	nhdt[4]   >= dvmd[16]  && nhdt[4]   <= dvmd[17] ) { res_g += sg; }
	if(	nhdt[4]   >= dvmd[18]  && nhdt[4]   <= dvmd[19] ) { res_g -= sg; }
	if(	nhdt[5]   >= dvmd[20]  && nhdt[5]   <= dvmd[21] ) { res_b += sb; }
	if(	nhdt[5]   >= dvmd[22]  && nhdt[5]   <= dvmd[23] ) { res_b -= sb; }

	if(	nhdt[6]   >= dvmd[24]  && nhdt[6]   <= dvmd[25] ) { res_r += sr; }
	if(	nhdt[6]   >= dvmd[26]  && nhdt[6]   <= dvmd[27] ) { res_r -= sr; }
	if(	nhdt[7]   >= dvmd[28]  && nhdt[7]   <= dvmd[29] ) { res_g += sg; }
	if(	nhdt[7]   >= dvmd[30]  && nhdt[7]   <= dvmd[31] ) { res_g -= sg; }
	if(	nhdt[8]   >= dvmd[32]  && nhdt[8]   <= dvmd[33] ) { res_b += sb; }
	if(	nhdt[8]   >= dvmd[34]  && nhdt[8]   <= dvmd[35] ) { res_b -= sb; }

/*	if(	nhdt[9]   >= dvmd[36]  && nhdt[9]   <= dvmd[37] ) { res_r += s; }
	if(	nhdt[9]   >= dvmd[38]  && nhdt[9]   <= dvmd[39] ) { res_r -= s; }
	if(	nhdt[10]   >= dvmd[40]  && nhdt[10]   <= dvmd[41] ) { res_g += s; }
	if(	nhdt[10]   >= dvmd[42]  && nhdt[10]   <= dvmd[43] ) { res_g -= s; }
	if(	nhdt[11]   >= dvmd[44]  && nhdt[11]   <= dvmd[45] ) { res_b += s; }
	if(	nhdt[11]   >= dvmd[46]  && nhdt[11]   <= dvmd[47] ) { res_b -= s; }*/

//	----    ----    ----    ----    ----    ----    ----    ----
//	Blur Application
//	----    ----    ----    ----    ----    ----    ----    ----

	float nhr_blur	=	( 	split_ring_nh( vec2(1.0, 0.0), ring_e0_r, ring_e1_r ) 
						+ 	split_ring_nh( vec2(3.0, 2.0), ring_e0_r, ring_e1_r )
						+ 	split_ring_nh( vec2(6.0, 5.0), ring_e0_r, ring_e1_r ) ) / ( 3.0 );
	float nhg_blur	=	( 	split_ring_nh( vec2(1.0, 0.0), ring_e0_g, ring_e1_g ) 
						+ 	split_ring_nh( vec2(3.0, 2.0), ring_e0_g, ring_e1_g )
						+ 	split_ring_nh( vec2(6.0, 5.0), ring_e0_g, ring_e1_g ) ) / ( 3.0 );
	float nhb_blur	=	( 	split_ring_nh( vec2(1.0, 0.0), ring_e0_b, ring_e1_b ) 
						+ 	split_ring_nh( vec2(3.0, 2.0), ring_e0_b, ring_e1_b )
						+ 	split_ring_nh( vec2(6.0, 5.0), ring_e0_b, ring_e1_b ) ) / ( 3.0 );

	res_r = (res_r + nhr_blur * s) / (1.0 + s);
	res_g = (res_g + nhg_blur * s) / (1.0 + s);
	res_b = (res_b + nhb_blur * s) / (1.0 + s);

//	----    ----    ----    ----    ----    ----    ----    ----
//	Layer Communication
//	----    ----    ----    ----    ----    ----    ----    ----

//	Interpolate
	float	inp_r = (res_r * 1.0 	+ res_g *  li 	+ res_b *  li	) / ( 1.0 + li * 2.0 );
	float	inp_g = (res_r *  li 	+ res_g * 1.0 	+ res_b *  li	) / ( 1.0 + li * 2.0 );
	float	inp_b = (res_r *  li	+ res_g *  li 	+ res_b * 1.0	) / ( 1.0 + li * 2.0 );
	res_r = inp_r;
	res_g = inp_g;
	res_b = inp_b;
/**/

//	Cycle
	float 	lcw = lc * dvmd[51];
	float	cyc_r = (res_r *  1.0 	+ res_g *  lcw 	+ res_b * -lcw	);
	float	cyc_g = (res_r * -lcw	+ res_g *  1.0 	+ res_b *  lcw	);
	float	cyc_b = (res_r *  lcw	+ res_g * -lcw 	+ res_b *  1.0	);
	res_r = cyc_r;
	res_g = cyc_g;
	res_b = cyc_b;
/**/
//	Random
	float[6] 	cyw;
	cyw[0] = dvmd[36] * lr * 1.0;
	cyw[1] = dvmd[37] * lr * 1.0;
	cyw[2] = dvmd[38] * lr * 1.0;
	cyw[3] = dvmd[39] * lr * 1.0;
	cyw[4] = dvmd[40] * lr * 1.0;
	cyw[5] = dvmd[41] * lr * 1.0;
	cyc_r = (res_r * 1.0 	+ res_g * cyw[0] 	+ res_b * cyw[1] ) / (1.0 + (cyw[0]+cyw[1]));
	cyc_g = (res_r * cyw[3]	+ res_g * 1.0 		+ res_b * cyw[2] ) / (1.0 + (cyw[2]+cyw[3]));
	cyc_b = (res_r * cyw[4]	+ res_g * cyw[5] 	+ res_b * 1.0	 ) / (1.0 + (cyw[4]+cyw[5]));
	res_r = cyc_r;
	res_g = cyc_g;
	res_b = cyc_b;

	cyw[0] = dvmd[42] * lr * 1.0;
	cyw[1] = dvmd[43] * lr * 1.0;
	cyw[2] = dvmd[44] * lr * 1.0;
	cyw[3] = dvmd[45] * lr * 1.0;
	cyw[4] = dvmd[46] * lr * 1.0;
	cyw[5] = dvmd[47] * lr * 1.0;
	cyc_r = (res_r * 1.0 	+ res_g * cyw[0] 	+ res_b * cyw[1] ) / (1.0 + (cyw[0]+cyw[1]));
	cyc_g = (res_r * cyw[3]	+ res_g * 1.0 		+ res_b * cyw[2] ) / (1.0 + (cyw[2]+cyw[3]));
	cyc_b = (res_r * cyw[4]	+ res_g * cyw[5] 	+ res_b * 1.0	 ) / (1.0 + (cyw[4]+cyw[5]));
	res_r = cyc_r;
	res_g = cyc_g;
	res_b = cyc_b;
/**/

	res_r -= mnp;
	res_g -= mnp;
	res_b -= mnp;
/**/
//	----    ----    ----    ----    ----    ----    ----    ----
//	Presentation Filtering
//	----    ----    ----    ----    ----    ----    ----    ----

//	----    ----    ----    ----    ----    ----    ----    ----
//	Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

//	Reseed (expensive?)
	if(ub.mrb == 2.0) { res_r = reseed2(0.0); 	res_g = reseed2(7.0); 	res_b = reseed2(13.0); }

//	Clear
	if(ub.mlb == 2.0) { res_r = 0.0; 			res_g = res_r; 			res_b = res_r; }

//	Channel Map
	col[0] 		= res_r;
	col[1] 		= res_g;
	col[2] 		= res_b;

//	Mouse Interaction
	col 		= place(col);

//	Final output
	out_col 	= vec4(col, 1.0);
}
