#version 460
//	Shader developed by Slackermanz:
//		https://twitter.com/slackermanz
//		https://www.reddit.com/user/slackermanz/
//		https://github.com/Slackermanz/VulkanAutomata
//		https://www.youtube.com/channel/UCmoNsNuM0M9VsIXfm2cHPiA/videos
//		https://www.shadertoy.com/user/SlackermanzCA
//		https://discord.gg/BCuYCEn
layout(location 	=  0) out 		vec4 out_col;
layout(binding 		=  0) uniform 	UniBuf {
	uint wsize;
	uint frame; } ub;
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
	out_col = vec4(reseed(0), reseed(1), reseed(2), 1.0);
}


