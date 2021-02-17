#version 460
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) out vec4 out_col;
layout(binding = 0) uniform sampler2D txdata;
layout(binding = 1) uniform init_ub {
    float w;
	float h;
	float seed;
	float frame;
	float clicks;
	float mx;
	float my;
	float mlb;
	float mrb;
	float div;
	float v0;
	float v1;
	float v2;
	float v3;
	float v4;
	float v5;
	float v6;
	float v7;
	float v8;
	float v9;
	float v10;
	float v11;
	float v12;
	float v13;
	float v14;
	float v15;
	float v16;
	float v17;
	float v18;
	float v19;
	float v20;
	float v21;
	float v22;
	float v23;
	float v24;
	float v25;
	float v26;
	float v27;
	float v28;
	float v29;
	float v30;
	float v31;
} ub;
float plim(float v, float p) {
	v = v * p;
	v = v - fract(v);
	v = v / p;
	return v;
}
float gv(float x, float y, int v) {
			float	div		= 1.0;
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
float gdv(float x, float y, int v, float div) {
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
float cv(float x, float y) {
	if(gv(x, y, 0) != 0.0) { return 1.0; } else { return 0.0; }
}
vec3 place(vec3 col_place) {
	const 	vec4	fc 			= gl_FragCoord;
			float 	place_size 	= 38.0;
			float 	distx 		= (fc[0]-ub.mx) * (fc[0]-ub.mx);
			float 	disty 		= (fc[1]-ub.my) * (fc[1]-ub.my);
			float 	dist  		= sqrt(distx+disty);
	if(ub.mlb == 1.0 || ub.mrb == 1.0) {
		if( fc[0] > ub.mx - place_size && fc[0] < ub.mx + place_size) {
			if( fc[1] > ub.my - place_size && fc[1] < ub.my + place_size) {
				if(dist < place_size) {
					col_place[0] = ub.mlb-ub.mrb;
					col_place[1] = ub.mlb-ub.mrb;
					col_place[2] = ub.mlb-ub.mrb;/**/
				}
		} }
		if( fc[0] > ub.mx - place_size*2.4 && fc[0] < ub.mx + place_size*2.4) {
			if( fc[1] > ub.my - place_size*0.25 && fc[1] < ub.my + place_size*0.25) {
				col_place[0] = ub.mlb-ub.mrb;
				col_place[1] = ub.mlb-ub.mrb;
				col_place[2] = ub.mlb-ub.mrb;/**/
		} } 
		if( fc[0] > ub.mx - place_size*0.25 && fc[0] < ub.mx + place_size*0.25) {
			if( fc[1] > ub.my - place_size*2.4 && fc[1] < ub.my + place_size*2.4) {
				col_place[0] = ub.mlb-ub.mrb;
				col_place[1] = ub.mlb-ub.mrb;
				col_place[2] = ub.mlb-ub.mrb;/**/
		} } 
		if( fc[0] > ub.mx - place_size*0.2 && fc[0] < ub.mx + place_size*0.2) {
			if( fc[1] > ub.my - place_size*0.2 && fc[1] < ub.my + place_size*0.2) {
				if(dist < place_size*0.2) {
					col_place[0] = 1.0-(ub.mlb-ub.mrb);
					col_place[1] = 1.0-(ub.mlb-ub.mrb);
					col_place[2] = 1.0-(ub.mlb-ub.mrb);
				}
		} } 
	}
	return col_place;
}
vec3 nhd( vec2 nbhd, float psn, float thr, int col, float div ) {
	float dist 		= 0.0;
	float cval 		= 0.0;
	float c_total 	= 0.0;
	float c_valid 	= 0.0;
	float c_value 	= 0.0;
	for(float i = -nbhd[0]; i <= nbhd[0]; i += 1.0) {
		for(float j = -nbhd[0]; j <= nbhd[0]; j += 1.0) {
			dist = round(sqrt(i*i+j*j));
			if( dist <= nbhd[0] && dist > nbhd[1] && dist != 0.0 ) {
				cval = gdv(i,j,col,div);
				c_total += psn;
				if( cval > thr ) {
					c_valid += psn;
					cval = psn * cval;
					c_value += cval-fract(cval);
				} } } }

	return vec3( c_value, c_valid, c_total );
}
vec3 export_nh() {
	vec3 c = vec3(0.0,0.0,0.0);
	//	R: Sign;		0.0 	| 	1.0
	//	G: X Coord;		0-255 	: 	0.0-1.0
	//	B: Y Coord;		0-255 	: 	0.0-1.0
	
//	( fc[0] >= float(int(dm[0]) / 2)+0.5-32.0 )

	return vec3(c[0],c[1],c[2]);
}
float cgol_test(float v, int c, float div) {
	vec3 nh = nhd(vec2(1.0,0.0),1.0,0.99,c,div);
	if(nh[0] <= 1.0) { v = 0.0; }
	if(nh[0] == 3.0) { v = 1.0; }
	if(nh[0] >= 4.0) { v = 0.0; }
	return v;
}
float sn(float s) {
	return (s > 0.0) ? 1.0 : (s < 0.0) ? -1.0 : 0.0;
}
float reseed() {
	const 	vec4	fc 		= gl_FragCoord;
	const 	vec2 	dm 		= { ub.w, ub.h };
	float pid = fc[0]+fc[1]*dm[0];
	float r = fract(tan(dot(
			vec2(mod(fc[0]+ub.frame,813.0)+(ub.v0+ub.v1+ub.v2)/41.5712,mod(fc[1]+ub.frame,317.0)+(ub.v3+ub.v4+ub.v5))/51.2156,
			vec2(12.9898,78.233))) * 43758.5453);
	float c = r;
	if(c < 0.2) {c = 0.0;}
	c = fract(c*215.61213);
	if(c < 0.2) {c = 0.0;}
	c = fract(c*151.41512);
	if(mod(fc[0],65.0) <= 24.0) { c = 0.0; }
	if(mod(fc[0],123.0) >= 97.0) { c = 1.0; }
	if(mod(fc[1],45.0) <= 14.0) { c = 0.0; }
	if(mod(fc[1],117.0) >= 102.0) { c = 1.0; }
	if(mod(fc[0]*fc[1],217.0) >= 192.0) { c = 1.0; }
	if(mod(fc[1],72.0) <= 11.0) { c = 0.0; }
	if(mod(fc[0]*fc[1],317.0) >= 292.0) { c = 0.0; }
	if(mod(fc[0]*fc[1],57.0) >= 32.0) { c = r; }
	if(mod(fc[0]+fc[1],74.0) <= 18.0) { c = 1.0; }
	if(mod(fc[0]-fc[1],111.0) <= 23.0) { c = 1.0; }
	if(mod(fc[0]+fc[1],81.0) <= 15.0) { c = 0.0; }
	if(mod(fc[0]-fc[1],121.0) <= 13.0) { c = 0.0; }
	return c;
}
float get_xc(float x, float y, float xmod) {
	float sq = sqrt(mod(x*y+y, xmod)) / sqrt(xmod);
	float xc = mod((x*x)+(y*y), xmod) / xmod;
	return clamp((sq+xc)*0.5, 0.0, 1.0);
}
float shuffle(float x, float y, float xmod, float val) {
	val = val * mod( x*y + x, xmod );
	return (val-floor(val));
}
float get_xcn(float x, float y, float xm0, float xm1, float ox, float oy) {
	float  xc = get_xc(x+ox, y+oy, xm0);
	return shuffle(x+ox, y+oy, xm1, xc);
}
float get_lump(float x, float y, float nhsz, float xm0, float xm1) {
	float 	nhsz_c 	= 0.0;
	float 	xcn 	= 0.0;
	float 	nh_val 	= 0.0;

	if(xm0 < 3.0) { xm0 = 3.0; }
	if(xm1 < 6.0) { xm1 = 6.0; }

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
	const 	vec4	fc 		= gl_FragCoord;
	float 	r0 = get_lump(fc[0], fc[1], 2.0, 19.0 + mod(ub.frame,17.0), 23.0 + mod(ub.frame,43.0));
	float 	r1 = get_lump(fc[0], fc[1], 24.0, 13.0 + mod(ub.frame,29.0), 17.0 + mod(ub.frame,31.0));
	float 	r2 = get_lump(fc[0], fc[1], 8.0, 13.0 + mod(ub.frame,11.0), 51.0 + mod(ub.frame,37.0));
	return clamp((r0+r1)-r2,0.0,1.0);
}
void main() {
	const 	vec4	fc 		= gl_FragCoord;
	const 	vec2 	dm 		= { ub.w, ub.h };
	const 	int 	VMX 	= 32;
	#define PI 3.1415926538
	float 		psn		= 250.0;
	float 		mnp		= 0.004;//1.0 / psn;
	float		div		= ub.div;
	float 		divi 	= floor((fc[0]*div)/(dm[0])) + floor((fc[1]*div)/(dm[1]))*div;
	float		cs		= (ub.clicks)*0.001;
	vec3 		col 	= vec3( 0.0, 0.0, 0.0 );

	float 		dspace = (divi+1.0)/(div*div);
				dspace = (div == 1.0) ? 0.5 : dspace;

	float[VMX] 	ubvn = float[VMX]
	(	ub.v0,	ub.v1,	ub.v2,	ub.v3,	
		ub.v4,	ub.v5,	ub.v6,	ub.v7,	
		ub.v8,	ub.v9,	ub.v10,	ub.v11,	
		ub.v12,	ub.v13,	ub.v14,	ub.v15,	
		ub.v16,	ub.v17,	ub.v18,	ub.v19,	
		ub.v20,	ub.v21,	ub.v22,	ub.v23,	
		ub.v24,	ub.v25,	ub.v26,	ub.v27,	
		ub.v28,	ub.v29,	ub.v30,	ub.v31 	);

	float ezv = 0.5;
	float[VMX] 	ez 	= float[VMX]
	(	ezv,	ezv,	ezv,	ezv,	
		ezv,	ezv,	ezv,	ezv,	
		ezv,	ezv,	ezv,	ezv,	
		ezv,	ezv,	ezv,	ezv,	
		ezv,	ezv,	ezv,	ezv,	
		ezv,	ezv,	ezv,	ezv,	
		ezv,	ezv,	ezv,	ezv,	
		ezv,	ezv,	ezv,	ezv 	);

	float[VMX] 	dvmd;
	for(int i = 0; i < VMX; i++) { 
		dvmd[i] = 	ubvn[i] * dspace
				+ 	ez[i];
	}

	float	res_r	= gdv( 0.0, 0.0, 0, div );// * psn;
	float	res_g	= gdv( 0.0, 0.0, 1, div );// * psn;
	float	res_b	= gdv( 0.0, 0.0, 2, div );// * psn;

	float 	s		= mnp * 10.0;

	vec3 	nmr 	= nhd(vec2(4.0,3.0),psn,0.0,0,div);
	float 	nmrw 	= nmr[0] / nmr[2];

	//	Pair 0
	//	Pair 0, MNCA0
	vec3 	n00r 	= nhd(vec2(1.0,0.0),psn,0.0,0,div);
	vec3 	n01r 	= nhd(vec2(3.0,0.0),psn,0.0,0,div);
	float 	n00rw 	= n00r[0] / n00r[2];
	float 	n01rw 	= n01r[0] / n01r[2];
	float 	r0 		= res_r;
	if(	n00rw >= dvmd[0] 	&& n00rw <= dvmd[1] 	) { r0 += s; }
	if(	n00rw >= dvmd[2] 	&& n00rw <= dvmd[3] 	) { r0 -= s; }
	if( n01rw >= dvmd[4] 	&& n01rw <= dvmd[5] 	) { r0 += s; }
	if( n01rw >= dvmd[6] 	&& n01rw <= dvmd[7] 	) { r0 -= s; }

	//	Pair 0, MNCA1
	vec3 	n10r 	= nhd(vec2(2.0,1.0),psn,0.0,0,div);
	vec3 	n11r 	= nhd(vec2(4.0,2.0),psn,0.0,0,div);
	float 	n10rw 	= n10r[0] / n10r[2];
	float 	n11rw 	= n11r[0] / n11r[2];
	float 	r1 		= res_r;
	if( n10rw >= dvmd[8] 	&& n10rw <= dvmd[9] 	) { r1 += s; }
	if( n10rw >= dvmd[10] 	&& n10rw <= dvmd[11] 	) { r1 -= s; }
	if( n11rw >= dvmd[12] 	&& n11rw <= dvmd[13] 	) { r1 += s; }
	if( n11rw >= dvmd[14] 	&& n11rw <= dvmd[15] 	) { r1 -= s; }

//	r0 = (r0 + (nmrw * 0.08) ) / 1.08;
//	r1 = (r1 + (nmrw * 0.08) ) / 1.08;

	r0 = (r0 + (n00rw * 0.025) + (n01rw * 0.025) + (res_r * 0.05)) / 1.1;
	r1 = (r1 + (n10rw * 0.025) + (n11rw * 0.025) + (res_r * 0.05)) / 1.1;

	//	Choice of P0MNCA0 and P0MNCA1
	float 	d0 		= abs(res_r - r0);
	float 	d1 		= abs(res_r - r1);
	float 	rd0 	= (d0 > d1) ? ((d0 >= mnp) ? r0 : r1) : ((d1 >= mnp) ? r1 : r0);

	//	Pair 1
	//	Pair 1, MNCA0
	vec3 	n20r 	= nhd(vec2(4.0,2.0),psn,0.0,0,div);
	vec3 	n21r 	= nhd(vec2(8.0,4.0),psn,0.0,0,div);
	float 	n20rw 	= n20r[0] / n20r[2];
	float 	n21rw 	= n21r[0] / n21r[2];
	float 	r2 		= res_r;
	if(	n20rw >= dvmd[16] 	&& n20rw <= dvmd[17] 	) { r2 += s; }
	if(	n20rw >= dvmd[18] 	&& n20rw <= dvmd[19] 	) { r2 -= s; }
	if( n21rw >= dvmd[20] 	&& n21rw <= dvmd[21] 	) { r2 += s; }
	if( n21rw >= dvmd[22] 	&& n21rw <= dvmd[23] 	) { r2 -= s; }

	//	Pair 1, MNCA1
	vec3 	n30r 	= nhd(vec2(6.0,0.0),psn,0.0,0,div);
	vec3 	n31r 	= nhd(vec2(12.0,8.0),psn,0.0,0,div);
	float 	n30rw 	= n30r[0] / n30r[2];
	float 	n31rw 	= n31r[0] / n31r[2];
	float 	r3 		= res_r;
	if( n30rw >= dvmd[24] 	&& n30rw <= dvmd[25] 	) { r3 += s; }
	if( n30rw >= dvmd[26] 	&& n30rw <= dvmd[27] 	) { r3 -= s; }
	if( n31rw >= dvmd[28] 	&& n31rw <= dvmd[29] 	) { r3 += s; }
	if( n31rw >= dvmd[30] 	&& n31rw <= dvmd[31] 	) { r3 -= s; }

//	r2 = (r2 + (nmrw * 0.08) ) / 1.08;
//	r3 = (r3 + (nmrw * 0.08) ) / 1.08;

	r2 = (r2 + (n20rw * 0.025) + (n21rw * 0.025) + (res_r * 0.05)) / 1.1;
	r3 = (r3 + (n30rw * 0.025) + (n31rw * 0.025) + (res_r * 0.05)) / 1.1;

	//	Choice of P1MNCA0 and P1MNCA1
	float 	d2 		= abs(res_r - r2);
	float 	d3 		= abs(res_r - r3);
	float 	rd1		= (d2 > d3) ? ((d2 >= mnp) ? r2 : r3) : ((d3 >= mnp) ? r3 : r2);

	//	Choice of P0 and P1
	float 	dr0 	= abs(res_r - rd0);
	float 	dr1 	= abs(res_r - rd1);
	float 	rddr01 	= (dr0 > dr1) ? ((dr0 >= mnp) ? rd0 : rd1) : ((dr1 >= mnp) ? rd1 : rd0);

//	rddr01 = (rddr01 + nmrw * 0.1) / 1.1;
	
	res_r = rddr01;

//	Output
	if(div > 1.0) {
		res_g = (mod(fc[0],dm[0]/div) <= 1.0 || (mod(fc[1],dm[1]/div) <= 1.0)) ? 0.15 : res_r;
		res_b = (mod(fc[0],dm[0]/div) <= 1.0 || (mod(fc[1],dm[1]/div) <= 1.0)) ? 0.15 : res_r;
	} else { res_g = res_r; res_b = res_r; }

	if(ub.mrb == 2.0) { res_r = reseed2(); res_g = res_r; res_b = res_r; }

	col[0] 		= res_r;
	col[1] 		= res_g;
	col[2] 		= res_b;

	col 		= place(col);
	out_col 	= vec4(col, 1.0);
}
