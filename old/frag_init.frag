#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_col;

layout(binding = 1) uniform init_ub {
    float w;
	float h;
//	float seed;
} ub;

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

void main() {
	const 	vec4	fc 		= gl_FragCoord;
	const 	vec2	dm 		= { ub.w, ub.h };
			vec3 	col 	= vec3(0.0, 0.0, 0.0);

			float 	center_dist_x 	= abs( fc[0] - dm[0] * 0.5 ) / (dm[0] * 0.5);
			float 	center_dist_y 	= abs( fc[1] - dm[1] * 0.5 ) / (dm[1] * 0.5);
			float 	center_dist 	= sqrt((center_dist_x*center_dist_x) + (center_dist_y*center_dist_y));
			float	center_blur 	= clamp(1.0 - center_dist, 0.0, 1.0);

			float 	lump_r			= get_lump(fc[0], fc[1], 18.0, 123.0, 147.0);// * center_blur;
			float 	lump_g			= get_lump(fc[0], fc[1], 26.0, 461.0, 251.0);// * center_blur;
			float 	lump_b			= get_lump(fc[0], fc[1], 48.0, 247.0, 323.0);// * center_blur;

	float cliplump = 1.0;

	float clp = 0.03;

	if(lump_r * lump_g < clp) {cliplump = 0.0;}
	if(lump_g * lump_b < clp) {cliplump = 0.0;}
	if(lump_b * lump_r < clp) {cliplump = 0.0;}
	if(lump_r * lump_g > 1.0-clp) {cliplump = 0.0;}
	if(lump_g * lump_b > 1.0-clp) {cliplump = 0.0;}
	if(lump_b * lump_r > 1.0-clp) {cliplump = 0.0;}

//	if(lump_r * lump_g > 0.16 && lump_r * lump_g < 0.28) {cliplump = 0.0;}
//	if(lump_r * lump_b > 0.46 && lump_r * lump_b < 0.48) {cliplump = 0.0;}

	col[0] = lump_r * cliplump;
	col[1] = lump_g * cliplump;
	col[2] = lump_b * cliplump;
/*
	if(mod(fc[0],10.0) <= 5.0) { col[0] = 0.0; }
	if(mod(fc[1],10.0) <= 5.0) { col[0] = 0.0; }

	float center_px = 1.0;
	float cpx_sz = dm[1]*0.42;
	if(fc[0] >= float(int(dm[0]) / 2)+0.5-cpx_sz) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-cpx_sz) {
			if(fc[0] <= float(int(dm[0]) / 2)+0.5+cpx_sz) {
				if(fc[1] <= float(int(dm[1]) / 2)+0.5+cpx_sz) {
					center_px = 0.0; } } } }

	col[0] = col[0] * center_px;

	center_px = 0.0;

	if(fc[0] >= float(int(dm[0]) / 2)+0.5-1.0) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-(dm[1]*0.22)) {
			if(fc[0] <= float(int(dm[0]) / 2)+0.5+1.0) {
				if(fc[1] <= float(int(dm[1]) / 2)+0.5+(dm[1]*0.22)) {
					center_px = 1.0; } } } }
/*
	if(fc[0] >= float(int(dm[0]) / 2)+0.5-31.0) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-(dm[1]*0.22)) {
			if(fc[0] <= float(int(dm[0]) / 2)+0.5-29.0) {
				if(fc[1] <= float(int(dm[1]) / 2)+0.5+(dm[1]*0.22)) {
					center_px = 1.0; } } } }
	if(fc[0] >= float(int(dm[0]) / 2)+0.5+29.0) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-(dm[1]*0.22)) {
			if(fc[0] <= float(int(dm[0]) / 2)+0.5+31.0) {
				if(fc[1] <= float(int(dm[1]) / 2)+0.5+(dm[1]*0.22)) {
					center_px = 1.0; } } } }
*/
/*	if(fc[0] >= float(int(dm[0]) / 2)+0.5-61.0) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-(dm[1]*0.22)) {
			if(fc[0] <= float(int(dm[0]) / 2)+0.5-59.0) {
				if(fc[1] <= float(int(dm[1]) / 2)+0.5+(dm[1]*0.22)) {
					center_px = 1.0; } } } }
	if(fc[0] >= float(int(dm[0]) / 2)+0.5+59.0) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-(dm[1]*0.22)) {
			if(fc[0] <= float(int(dm[0]) / 2)+0.5+61.0) {
				if(fc[1] <= float(int(dm[1]) / 2)+0.5+(dm[1]*0.22)) {
					center_px = 1.0; } } } }

	if(center_px == 1.0 && mod(fc[1],13.0) <= 2.0)  {col[0] = 1.0;}

/**/
	float center_px = 0.0;
	float cpx_sz = 36.0;
	if(fc[0] >= float(int(dm[0]) / 2)+0.5-cpx_sz) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-cpx_sz) {
			if(fc[0] <= float(int(dm[0]) / 2)-0.5+cpx_sz) {
				if(fc[1] <= float(int(dm[1]) / 2)-0.5+cpx_sz) {
					center_px = 1.0;
				}
			}
		}
	}
/**/
/*	float center_px = 0.0;
	float cpx_sz = 64.0 / 2.0;
	if(fc[0] >= float(int(dm[0]) / 2)+0.5-cpx_sz) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-cpx_sz) {
			if(fc[0] <= float(int(dm[0]) / 2)-0.5+cpx_sz) {
				if(fc[1] <= float(int(dm[1]) / 2)-0.5+cpx_sz) {
					center_px = col[0];
				}
			}
		}
	}
*/
	cpx_sz = cpx_sz * 0.75;
	if(fc[0] >= float(int(dm[0]) / 2)+0.5-cpx_sz) {
		if(fc[1] >= float(int(dm[1]) / 2)+0.5-cpx_sz) {
			if(fc[0] <= float(int(dm[0]) / 2)-0.5+cpx_sz) {
				if(fc[1] <= float(int(dm[1]) / 2)-0.5+cpx_sz) {
					center_px = 0.0;
				}
			}
		}
	}
/**/

	col[0] = center_px;
	col[1] = center_px;
	col[2] = center_px;
/**/

	out_col	= vec4(col, 1.0);
}
