#version 460
#extension GL_ARB_separate_shader_objects : enable
//	Shader developed by Slackermanz:
//		https://www.reddit.com/user/slackermanz/
//		https://github.com/Slackermanz/VulkanAutomata
//	Specification: SlackCA_Specification_2021_02_23:
//		https://mega.nz/file/yRliVDJT#6CUlcGma4DpfXI4S8j0VoUi8Vju0vwRXwVI4klyiNXg
//		Specification version does not include Neighbourhood Offset Test documentation
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

			float 	modfxd 	= mod( fcxo, divx 		 );
			float 	modfyd 	= mod( fcyo, divy 		 );
//	if(x < 0.0) { 	modfxd 	= mod( fcxo + divx, divx );
//					modfyd 	= mod( fcyo + divy, divy ); }

	const 	float 	fcx 	= (modfxd + floor(fc[0]/divx)*divx ) * pxo;
	const 	float 	fcy 	= (modfyd + floor(fc[1]/divy)*divy ) * pyo;

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

	float[VMX] 	ubvn 	= float[VMX]		//	Uniform Buffer V number
	(	ub.v0,	ub.v1,	ub.v2,	ub.v3,	
		ub.v4,	ub.v5,	ub.v6,	ub.v7,	
		ub.v8,	ub.v9,	ub.v10,	ub.v11,	
		ub.v12,	ub.v13,	ub.v14,	ub.v15,	
		ub.v16,	ub.v17,	ub.v18,	ub.v19,	
		ub.v20,	ub.v21,	ub.v22,	ub.v23,	
		ub.v24,	ub.v25,	ub.v26,	ub.v27,	
		ub.v28,	ub.v29,	ub.v30,	ub.v31 	);

	float[VMX] 	dvmd;						//	Division Weighted V number
	for(int i = 0; i < VMX; i++) { dvmd[i] = ubvn[i] * dspace; }

//	----    ----    ----    ----    ----    ----    ----    ----
//	RULE:Scale Variance Agoniser (Slackermanz MSTP variant)
//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation

//	Get the reference frame's origin pixel values
	float	res_r	= gdv( 0.0, 0.0, 0, div );
	float	res_g	= gdv( 0.0, 0.0, 1, div );
	float	res_b	= gdv( 0.0, 0.0, 2, div );

//	Intended rate of change
	float 	s 		= mnp * 20.0;


//	STAGE:Scales
//		DOMAIN: 	Totalistic Individual Neighbourhood Continuous
//		REQUIRES:	Unconditional
//		UPDATE:		Select
//		VALUE:		Origin
//		BLUR:		Relative
//		RESULT:		Multiple

//	Number of Individual Neighbourhoods
	const 	int 		sets	= 7;

//	Container for STAGE:Scales results
			float[sets] rslt;

//	----    ----    ----    ----    ----    ----    ----    ----
//	< Neighbourhood Offset Test >
//	----    ----    ----    ----    ----    ----    ----    ----
//	Define and assess the Individual Neighbourhoods
//	vec3 	n0r 	= nhd( vec2( 1.0, 	0.0  ), vec2( 0.0, 0.0 ), 	psn, 0.0, 0, div );
//	vec3 	n1r 	= nhd( vec2( 3.0, 	0.0  ), vec2( 0.0, 0.0 ), 	psn, 0.0, 0, div );
//	vec3 	n2r 	= nhd( vec2( 5.0, 	1.0  ), vec2( 0.0, 0.0 ), 	psn, 0.0, 0, div );
//	vec3 	n3r 	= nhd( vec2( 10.0, 	3.0  ), vec2( 0.0, 0.0 ), 	psn, 0.0, 0, div );
//	vec3 	n4r 	= nhd( vec2( 18.0, 	5.0  ), vec2( 0.0, 0.0 ), 	psn, 0.0, 0, div );
//	vec3 	n5r 	= nhd( vec2( 28.0, 	10.0 ), vec2( 0.0, 0.0 ), 	psn, 0.0, 0, div );
//	vec3 	n6r 	= nhd( vec2( 42.0, 	18.0 ), vec2( 0.0, 0.0 ), 	psn, 0.0, 0, div );

//	Get the Totalistic value of each Individual Neighbourhood
//	rslt[0] = n0r[0] / n0r[2];
//	rslt[1] = n1r[0] / n1r[2];
//	rslt[2] = n2r[0] / n2r[2];
//	rslt[3] = n3r[0] / n3r[2];
//	rslt[4] = n4r[0] / n4r[2];
//	rslt[5] = n5r[0] / n5r[2];
//	rslt[6] = n6r[0] / n6r[2];
	vec2	tg;
//			tg				= vec2(ub.mx, ub.my);
			tg				= vec2(dm[0]/2.0, dm[1]/2.0);

	float 	center_dist_x 	= abs	( fc[0] - tg[0] * 1.0 ) / (dm[0] * 1.0);
	float 	center_dist_y 	= abs	( fc[1] - tg[1] * 1.0 ) / (dm[1] * 1.0);
	float 	cdx 			= 	 	( fc[0] - tg[0] * 1.0 ) / (dm[0] * 1.0);
	float 	cdy 			= 	 	( fc[1] - tg[1] * 1.0 ) / (dm[1] * 1.0);
	float 	cdist			= sqrt	( center_dist_x*center_dist_x + center_dist_y*center_dist_y );
	float[sets] nhdto;
	float 	ofx 	= round(sign(cdx) * -1.0 * sqrt((center_dist_x*center_dist_x)) * cdist*cdist * 128.0);
	float 	ofy 	= round(sign(cdy) * -1.0 * sqrt((center_dist_y*center_dist_y)) * cdist*cdist * 128.0);
	float	oft 	= 0.0;
//	if(cdist > 0.48) { oft = ofx; ofx = ofy; ofy = -oft; }
//	if(sqrt(center_dist_x*center_dist_x + center_dist_y*center_dist_y*3.0) < 0.12) { ofy = ofy * 3.0; }
	vec3 	n0ro 	= nhd( vec2( 1.0, 	0.0  ), vec2( ofx, ofy ), 	psn, 0.0, 0, div );
	vec3 	n1ro 	= nhd( vec2( 3.0, 	0.0  ), vec2( ofx, ofy ), 	psn, 0.0, 0, div );
	vec3 	n2ro 	= nhd( vec2( 5.0, 	1.0  ), vec2( ofx, ofy ), 	psn, 0.0, 0, div );
	vec3 	n3ro 	= nhd( vec2( 10.0, 	3.0  ), vec2( ofx, ofy ), 	psn, 0.0, 0, div );
	vec3 	n4ro 	= nhd( vec2( 18.0, 	5.0  ), vec2( ofx, ofy ), 	psn, 0.0, 0, div );
	vec3 	n5ro 	= nhd( vec2( 28.0, 	10.0 ), vec2( ofx, ofy ), 	psn, 0.0, 0, div );
	vec3 	n6ro 	= nhd( vec2( 42.0, 	18.0 ), vec2( ofx, ofy ), 	psn, 0.0, 0, div );
	nhdto[0] = n0ro[0] / n0ro[2];
	nhdto[1] = n1ro[0] / n1ro[2];
	nhdto[2] = n2ro[0] / n2ro[2];
	nhdto[3] = n3ro[0] / n3ro[2];
	nhdto[4] = n4ro[0] / n4ro[2];
	nhdto[5] = n5ro[0] / n5ro[2];
	nhdto[6] = n6ro[0] / n6ro[2];
	float ow = 2.0;
//	for(int i = 0; i < sets; i++) { rslt[i] = ( rslt[i] + nhdto[i] * ow ) / ( 1.0 + ow ); }
	for(int i = 0; i < sets; i++) { rslt[i] = nhdto[i]; }
//	----    ----    ----    ----    ----    ----    ----    ----

//	Apply a BLUR:Relative to the VALUE:Origin
	rslt[0] = (res_r + rslt[0] * s) / (1.0 + s );
	rslt[1] = (res_r + rslt[1] * s) / (1.0 + s );
	rslt[2] = (res_r + rslt[2] * s) / (1.0 + s );
	rslt[3] = (res_r + rslt[3] * s) / (1.0 + s );
	rslt[4] = (res_r + rslt[4] * s) / (1.0 + s );
	rslt[5] = (res_r + rslt[5] * s) / (1.0 + s );
	rslt[6] = (res_r + rslt[6] * s) / (1.0 + s );

//	----    ----    ----    ----    ----    ----    ----    ----
//	STAGE:Variance
//		DOMAIN: 	Scales
//		REQUIRES:	Previous
//		UPDATE:		Subtract
//		VALUE:		Domain
//		BLUR:		Specific
//		RESULT:		Multiple

//	Container for STAGE:Variance results
	float[sets] variance;

//	UPDATE:Subtract the REQUIRES:Previous value
	for(int i = 0; i < sets; i++) { 
		if(i == 0) 	{ variance[i] = res_r		- rslt[i]; }
		else 		{ variance[i] = rslt[i-1] 	- rslt[i]; } }

//	----    ----    ----    ----    ----    ----    ----    ----
//	STAGE:MinimumSelection
//		DOMAIN: 	Variance
//		REQUIRES:	MinimumABS
//		UPDATE:		Select
//		VALUE:		Domain
//		BLUR:		Specific
//		RESULT:		Single

//	Index of an element in DOMAIN:Variance
	int vsn = 0;

//	Get the index of the element in DOMAIN:Variance that meets REQUIRES:MinimumABS
	for( int i = 0; i < sets; i++ ) { if( abs(variance[vsn]) > abs(variance[i]) ) { vsn = i; } }

//	UPDATE:Select the DOMAIN:Variance value with the REQUIRES:MinimumABS index
	float minvar = variance[vsn];

//	----    ----    ----    ----    ----    ----    ----    ----
//	STAGE:Output
//		DOMAIN: 	MinimumSelection
//		REQUIRES:	Unconditional
//		UPDATE:		AdditiveSign
//		VALUE:		Origin
//		BLUR:		Diffuse
//		RESULT:		Single

//	UPDATE:AdditiveSign the VALUE:Origin
	res_r =	 res_r + sign(minvar) * s;

//	BLUR:Diffuse the result
//		BLUR Section altered for Neighbourhood Offset Test
	float 	blr_sz	= round((clamp(cdist,0.0,1.0))*32.0);
	vec3 	blr 	= nhd( vec2( blr_sz+1.0, 0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, div );
	float	blrt	= blr[0] / blr[2];
	res_r = (res_r + blrt * s * blr_sz * 0.3) / (1.0 + s * blr_sz * 0.3);

//	----    ----    ----    ----    ----    ----    ----    ----
//	Presentation Filtering
//	----    ----    ----    ----    ----    ----    ----    ----
//		BLUR Section altered for Neighbourhood Offset Test - Sauron variant
	vec3 	n0g 	= nhd( vec2( 4.0, 0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 1, div );
	vec3 	n0b 	= nhd( vec2( 2.0, 0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 2, div );
	float 	n0gw 	= n0g[0] / n0g[2];
	float 	n0bw 	= n0b[0] / n0b[2];
	res_g = ( res_g + n0gw * s * 1.0 + res_r * s * 1.2 ) / (1.0 + s * 3.2);
	res_g = res_g * res_r * (1.0-cdist) * 1.4;
//	res_b = ( res_b + n0bw * s * 0.5 + res_r * s * 0.5 ) / (1.0 + s * 1.0) - res_g * 0.25 - mnp;
	res_b = (pow((1.0-cdist), 24.0) * 32.0) * res_r * res_g;

	float eye = sqrt(center_dist_x*center_dist_x*64.0 + center_dist_y*center_dist_y);
	res_r = res_r - clamp(pow((1.0-eye), 24.0) * 8.0, 0.0, 0.75);
	float eye2 = sqrt(center_dist_x*center_dist_x + center_dist_y*center_dist_y*1.5);
	res_r = res_r - clamp(pow((eye2), 6.0) * 1.0, 0.0, 1.0);
	res_g = res_g + res_g * clamp(pow((1.0-eye2),2.0),0.0,1.0) * 0.2;

	

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
