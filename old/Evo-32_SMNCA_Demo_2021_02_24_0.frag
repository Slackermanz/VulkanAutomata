#version 460
#extension GL_ARB_separate_shader_objects : enable
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
} ub;
float gdv(float x, float y, int v, float div) {
//	Get Div Value: Return the value of a specified pixel
//		x, y : 	Relative integer-spaced coordinates to origin [ 0.0, 0.0 ]
//		v	 :	Colour channel [ 0, 1, 2 ]
//		div	 :	Integer-spaced number of toroidal divisions of the surface/medium
	const 	vec4	fc 		= gl_FragCoord;
	const 	vec2 	dm 		= { ub.w, ub.h };
			float 	divx	= dm[0] / div;
			float 	divy	= dm[1] / div;
	const 	float 	pxo 	= 1.0 / dm[0];
	const 	float 	pyo 	= 1.0 / dm[1];
	const 	float 	fcxo 	= fc[0] + x;
	const 	float 	fcyo 	= fc[1] + y;
	const 	float 	fcx 	= (mod(fcxo,divx) + floor(fc[0]/divx)*divx ) * pxo;
	const 	float 	fcy 	= (mod(fcyo,divy) + floor(fc[1]/divy)*divy ) * pyo;
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
					col_place[0] = ub.mlb-ub.mrb; } } }

		if( fc[0] > ub.mx - place_size*2.4 && fc[0] < ub.mx + place_size*2.4) {
			if( fc[1] > ub.my - place_size*0.25 && fc[1] < ub.my + place_size*0.25) {
				col_place[0] = ub.mlb-ub.mrb; } }

		if( fc[0] > ub.mx - place_size*0.25 && fc[0] < ub.mx + place_size*0.25) {
			if( fc[1] > ub.my - place_size*2.4 && fc[1] < ub.my + place_size*2.4) {
				col_place[0] = ub.mlb-ub.mrb; } }

		if( fc[0] > ub.mx - place_size*0.2 && fc[0] < ub.mx + place_size*0.2) {
			if( fc[1] > ub.my - place_size*0.2 && fc[1] < ub.my + place_size*0.2) {
				if(dist < place_size*0.2) {
					col_place[0] = 1.0-(ub.mlb-ub.mrb); } } } }

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
float reseed2() {
//	Used to reseed the surface with noise
	const 	vec4	fc 		= gl_FragCoord;
	float 	r0 = get_lump(fc[0], fc[1], 2.0, 19.0 + mod(ub.frame,17.0), 23.0 + mod(ub.frame,43.0));
	float 	r1 = get_lump(fc[0], fc[1], 24.0, 13.0 + mod(ub.frame,29.0), 17.0 + mod(ub.frame,31.0));
	float 	r2 = get_lump(fc[0], fc[1], 8.0, 13.0 + mod(ub.frame,11.0), 51.0 + mod(ub.frame,37.0));
	return clamp((r0+r1)-r2,0.0,1.0);
}
void main() {
//	----    ----    ----    ----    ----    ----    ----    ----
//	Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----
	const 	vec4	fc 		= gl_FragCoord;					// 	Frag Coords
	const 	vec2 	dm 		= { ub.w, ub.h };				//	Surface Dimensions
	const 	int 	VMX 	= 32;							//	Evolution Variables
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
		ub.v28,	ub.v29,	ub.v30,	ub.v31 	);

//	Hardcoded coordinates for demonstration purposes
	float[VMX] 	demo 	= float[VMX]
	(	0.287000,	1.041000,	-0.079000,	-0.264000,	
		-0.052000,	-0.002000,	0.970000,	0.368000,	
		1.070000,	0.510000,	0.135000,	0.955000,	
		-0.064000,	0.556000,	0.750000,	0.234000,	
		0.486000,	-0.250000,	-0.243000,	0.632000,	
		0.136000,	0.603000,	0.123000,	0.634000,	
		0.536000,	0.640000,	0.861000,	-0.150000,	
		0.094000,	0.662000,	0.210000,	0.143000	);

//	Division Weighted V number
	float[VMX] 	dvmd;
	for(int i = 0; i < VMX; i++) { dvmd[i] = demo[i] + ubvn[i] * dspace; }

//	----    ----    ----    ----    ----    ----    ----    ----
//	RULE:Evo32 Selective MNCA
//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation

//	Get the reference frame's origin pixel values
	float	res_r	= gdv( 0.0, 0.0, 0, div );
	float	res_g	= gdv( 0.0, 0.0, 1, div );
	float	res_b	= gdv( 0.0, 0.0, 2, div );

//	Intended rate of change
	float 	s 		= mnp * 12.0;

//	----    ----    ----    ----    ----    ----    ----    ----
//	STAGE:MNCA
//		DOMAIN: 	Totalistic Multiple Neighbourhood Continuous
//		REQUIRES:	Conditional Range
//		UPDATE:		Additive
//		VALUE:		Origin
//		BLUR:		Relative
//		RESULT:		Multiple

//	Number of Individual Neighbourhoods
	const 	int 		nhds	= 8;
//	Container for Neighbourhood Totalistic values
			float[nhds] nhdt;

//	Number of MNCA Groups
	const 	int 		sets	= 4;
//	Container for STAGE:MNCA results
			float[sets] rslt;
						rslt[0] = res_r;
						rslt[1] = res_r;
						rslt[2] = res_r;
						rslt[3] = res_r;

//	Define and assess the Individual Neighbourhoods
	vec3 	n00r 	= nhd( vec2( 1.0, 	0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	vec3 	n01r 	= nhd( vec2( 3.0, 	0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	vec3 	n10r 	= nhd( vec2( 2.0, 	0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	vec3 	n11r 	= nhd( vec2( 5.0, 	3.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	vec3 	n20r 	= nhd( vec2( 5.0, 	2.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	vec3 	n21r 	= nhd( vec2( 9.0, 	7.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	vec3 	n30r 	= nhd( vec2( 4.0, 	2.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	vec3 	n31r 	= nhd( vec2( 12.0, 	9.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );

//	Get the Totalistic value of each Individual Neighbourhood
	nhdt[0] = n00r[0] / n00r[2];
	nhdt[1] = n01r[0] / n01r[2];
	nhdt[2] = n10r[0] / n10r[2];
	nhdt[3] = n11r[0] / n11r[2];
	nhdt[4] = n20r[0] / n20r[2];
	nhdt[5] = n21r[0] / n21r[2];
	nhdt[6] = n30r[0] / n30r[2];
	nhdt[7] = n31r[0] / n31r[2];

//	UPDATE:Additive the VALUE:Origin according to REQUIRES:Conditional Range
	if(	nhdt[0] >= dvmd[0] 	&& nhdt[0] <= dvmd[1] 	) { rslt[0] += s; }
	if(	nhdt[0] >= dvmd[2] 	&& nhdt[0] <= dvmd[3] 	) { rslt[0] -= s; }
	if( nhdt[1] >= dvmd[4] 	&& nhdt[1] <= dvmd[5] 	) { rslt[0] += s; }
	if( nhdt[1] >= dvmd[6] 	&& nhdt[1] <= dvmd[7] 	) { rslt[0] -= s; }

	if( nhdt[2] >= dvmd[8] 	&& nhdt[2] <= dvmd[9] 	) { rslt[1] += s; }
	if( nhdt[2] >= dvmd[10] && nhdt[2] <= dvmd[11] 	) { rslt[1] -= s; }
	if( nhdt[3] >= dvmd[12] && nhdt[3] <= dvmd[13] 	) { rslt[1] += s; }
	if( nhdt[3] >= dvmd[14] && nhdt[3] <= dvmd[15] 	) { rslt[1] -= s; }

	if(	nhdt[4] >= dvmd[16] && nhdt[4] <= dvmd[17] 	) { rslt[2] += s; }
	if(	nhdt[4] >= dvmd[18] && nhdt[4] <= dvmd[19] 	) { rslt[2] -= s; }
	if( nhdt[5] >= dvmd[20] && nhdt[5] <= dvmd[21] 	) { rslt[2] += s; }
	if( nhdt[5] >= dvmd[22] && nhdt[5] <= dvmd[23] 	) { rslt[2] -= s; }

	if( nhdt[6] >= dvmd[24] && nhdt[6] <= dvmd[25] 	) { rslt[3] += s; }
	if( nhdt[6] >= dvmd[26] && nhdt[6] <= dvmd[27] 	) { rslt[3] -= s; }
	if( nhdt[7] >= dvmd[28] && nhdt[7] <= dvmd[29] 	) { rslt[3] += s; }
	if( nhdt[7] >= dvmd[30] && nhdt[7] <= dvmd[31] 	) { rslt[3] -= s; }

//	Apply a BLUR:Relative to the result
	rslt[0] = (rslt[0] + nhdt[0] * s * 0.5 + nhdt[1] * s * 0.5) / (1.0 + s * 1.0 );
	rslt[1] = (rslt[1] + nhdt[2] * s * 0.5 + nhdt[3] * s * 0.5) / (1.0 + s * 1.0 );
	rslt[2] = (rslt[2] + nhdt[4] * s * 0.5 + nhdt[5] * s * 0.5) / (1.0 + s * 1.0 );
	rslt[3] = (rslt[3] + nhdt[6] * s * 0.5 + nhdt[7] * s * 0.5) / (1.0 + s * 1.0 );

//	----    ----    ----    ----    ----    ----    ----    ----
//	STAGE:Variance
//		DOMAIN: 	MNCA
//		REQUIRES:	Unconditional
//		UPDATE:		Subtract
//		VALUE:		Origin
//		BLUR:		Specific
//		RESULT:		Multiple

//	Container for STAGE:Variance results
	float[sets] variance;

//	UPDATE:Subtract the REQUIRES:Previous value
	for(int i = 0; i < sets; i++) { 
		{ variance[i] = res_r - rslt[i]; } }

//	----    ----    ----    ----    ----    ----    ----    ----
//	STAGE:Output
//		DOMAIN: 	MNCA, Variance
//		REQUIRES:	MaximumABS_Match
//		UPDATE:		Select
//		VALUE:		Domain[0]
//		BLUR:		Specific
//		RESULT:		Single

//	Index of an element in DOMAIN:Variance
	int von = 0;

//	Get the index of the element in DOMAIN:Variance that meets REQUIRES:MaximumABS_Match
	for( int i = 0; i < sets; i++ ) { if( abs(variance[von]) < abs(variance[i]) ) { von = i; } }

//	UPDATE:Select the DOMAIN:MNCA value with the REQUIRES:MaximumABS_Match index
	float maxvar = rslt[von];

//	Output that value
	res_r = maxvar;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Presentation Filtering
//	----    ----    ----    ----    ----    ----    ----    ----
	vec3 	n0g 	= nhd( vec2( 1.0, 0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 1, div );
	vec3 	n0b 	= nhd( vec2( 2.0, 0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 2, div );
	float 	n0gw 	= n0g[0] / n0g[2];
	float 	n0bw 	= n0b[0] / n0b[2];
	res_g = ( res_g + n0gw * s * 2.0 + res_r * s * 2.0 ) / (1.0 + s * 4.0);
	res_b = ( res_b + n0bw * s * 1.0 + res_r * s * 1.0 ) / (1.0 + s * 2.0);

//	----    ----    ----    ----    ----    ----    ----    ----
//	Fragment Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	if(ub.mrb == 2.0) { res_r = reseed2(); res_g = res_r; res_b = res_r; }

	col[0] 		= res_r;
	col[1] 		= res_g;
	col[2] 		= res_b;

	col 		= place(col);
	out_col 	= vec4(col, 1.0);
}
