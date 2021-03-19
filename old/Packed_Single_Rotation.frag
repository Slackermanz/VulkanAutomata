#version 460
#extension GL_ARB_separate_shader_objects : enable
//	----    ----    ----    ----    ----    ----    ----    ----
//	NAME:Packed Single Rotation Rule
//	TYPE:Encoded Margolus Neighbourhood Block Cellular Automata
//	RULE:Single Rotation Rule
//		http://dmishin.blogspot.com/2013/11/the-single-rotation-rule-remarkably.html
//	----    ----    ----    ----    ----    ----    ----    ----
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
float margo() {
	float	states		= 15.0;
	float	block_phase	= mod(ub.frame, 2.0);

//	Get the values of the Red Channel pixels that contain the encoded substates for the Margolus Neighbourhood
	float[4] block_neighbourhood = float[4]
	(	round(gdv(-1.0+block_phase*1.0, -1.0+block_phase*1.0, 0, 1.0) * states),
		round(gdv( 0.0+block_phase*1.0, -1.0+block_phase*1.0, 0, 1.0) * states),
		round(gdv(-1.0+block_phase*1.0,  0.0+block_phase*1.0, 0, 1.0) * states),
		round(gdv( 0.0+block_phase*1.0,  0.0+block_phase*1.0, 0, 1.0) * states)	);

//	Containers for the substates
	float[4] neighbour_0_substates = float[4] ( 0.0, 0.0, 0.0, 0.0 );
	float[4] neighbour_1_substates = float[4] ( 0.0, 0.0, 0.0, 0.0 );
	float[4] neighbour_2_substates = float[4] ( 0.0, 0.0, 0.0, 0.0 );
	float[4] neighbour_3_substates = float[4] ( 0.0, 0.0, 0.0, 0.0 );

//	Decode the substates (binary representation) of the neighbourhood pixels' values
	float m = states + 1.0;
	for(int i = 0; i < 4; i++) {
		neighbour_0_substates[i] = round( mod( (block_neighbourhood[0]), m ) / (m-1.0) );
		neighbour_1_substates[i] = round( mod( (block_neighbourhood[1]), m ) / (m-1.0) );
		neighbour_2_substates[i] = round( mod( (block_neighbourhood[2]), m ) / (m-1.0) );
		neighbour_3_substates[i] = round( mod( (block_neighbourhood[3]), m ) / (m-1.0) );
		m = m / 2.0; }

//	Container for the new/recombined substate
	float[4] new_substates = float[4] ( 0.0, 0.0, 0.0, 0.0 );

//	Get the relevant substates
	new_substates[0] = neighbour_0_substates[3];
	new_substates[1] = neighbour_1_substates[2];
	new_substates[2] = neighbour_2_substates[1];
	new_substates[3] = neighbour_3_substates[0];

//	----    ----    ----    ----    ----    ----    ----    ----
//	Apply the 'Single Rotation Rule' transition
//	----    ----    ----    ----    ----    ----    ----    ----

//	If the current-phased block of substates contains only one value ...
	float sum	= new_substates[0] 
				+ new_substates[1] 
				+ new_substates[2] 
				+ new_substates[3];

//	Container for the transformed substates
    float[4] final_substates;
        final_substates[0] = new_substates[0];
        final_substates[1] = new_substates[1];
        final_substates[2] = new_substates[2];
        final_substates[3] = new_substates[3];

//	If the condition is met, apply the transformation, remapping the substates within the block
/*
//	Single Rotation
	if(sum == 1.0) {
		final_substates[0] = new_substates[2];
		final_substates[1] = new_substates[0];
		final_substates[2] = new_substates[3];
		final_substates[3] = new_substates[1]; }
/**/


//	Diffuse Rotation 
	if(sum == 1.0) {
		final_substates[0] = new_substates[2];
		final_substates[1] = new_substates[0];
		final_substates[2] = new_substates[3];
		final_substates[3] = new_substates[1]; }
	if(sum == 3.0) {
		final_substates[0] = new_substates[2];
		final_substates[1] = new_substates[0];
		final_substates[2] = new_substates[3];
		final_substates[3] = new_substates[1]; }
/**/

//	----    ----    ----    ----    ----    ----    ----    ----

//	Pack the pixel from the final substate data
	float res	= 0.0;
	float n 	= (states + 1.0) / 2.0;
	for(int i = 0; i < 4; i++) {
		res = res + final_substates[i] * n;	n = n / 2.0; }
	res = res / states;

//	Return the encoded pixel value
	return res;
}
void main() {
//	----    ----    ----    ----    ----    ----    ----    ----
//	Initilisation
//	----    ----    ----    ----    ----    ----    ----    ----
	const 	int 	VMX 	= 52;								//	Evolution Variables
			vec4	fc 		= gl_FragCoord;						// 	Frag Coords
			vec2 	dm 		= { ub.w, ub.h };					//	Surface Dimensions
			float 	psn		= 250.0;							//	Precision
			float 	mnp		= 0.004;							//	Minimum Precision Value : (1.0 / psn);
			float	div		= ub.div;							//	Toroidal Surface Divisions
			float 	divi 	= floor((fc[0]*div)/(dm[0]))		
							+ floor((fc[1]*div)/(dm[1]))*div;	//	Division Index
			float 	dspace 	= (divi+1.0)/(div*div);
					dspace 	= (div == 1.0) ? 0.5 : dspace;		//	Division Weight
			float	cs		= (ub.clicks)*0.001;				//	Used for testing/debugging small value variations
			vec3 	col 	= vec3( 0.0, 0.0, 0.0 );			//	Final colour value output container

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
//	RULE:Single Rotation Rule
//		http://dmishin.blogspot.com/2013/11/the-single-rotation-rule-remarkably.html
//	----    ----    ----    ----    ----    ----    ----    ----
//	Rule Initilisation

//	Get the reference frame's origin pixel values
	float	res_r	= gdv( 0.0, 0.0, 0, div );
	float	res_g	= gdv( 0.0, 0.0, 1, div );
	float	res_b	= gdv( 0.0, 0.0, 2, div );

//	Intended rate of change
	float 	s 		= mnp * 12.0;

//	Output value
	res_r = margo();

//	----    ----    ----    ----    ----    ----    ----    ----
//	Presentation Filtering
//	----    ----    ----    ----    ----    ----    ----    ----
	vec3 	n0r 	= nhd( vec2( 4.0, 0.0 ), vec2( 0.0, 0.0 ), psn, 0.0, 0, 1.0 );
	float 	n0rw 	= n0r[0] / n0r[2];
	res_g = (res_r > 0.0) ? res_g + mnp * 2.0 : (mod(ub.frame,32.0) == 0.0) ? res_g - mnp * 2.0 : res_g;
	res_g = (n0rw > 0.06) ? res_g - mnp * 4.0 : res_g;
	res_b = (res_r > 0.0) ? 1.0 : res_b - mnp * 2.0;

//	----    ----    ----    ----    ----    ----    ----    ----
//	Fragment Shader Output
//	----    ----    ----    ----    ----    ----    ----    ----

	if(ub.mrb == 2.0) { res_r = reseed2(); res_g = res_r; res_b = res_r; }
	if(ub.mlb == 2.0) { res_r = 0.0; res_g = res_r; res_b = res_r; }

	col[0] 		= res_r;
	col[1] 		= res_g;
	col[2] 		= res_b;

	col 		= place(col);
	out_col 	= vec4(col, 1.0);
}
