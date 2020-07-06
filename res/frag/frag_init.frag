#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_col;

void main() {
	const 	vec4	fc 		= gl_FragCoord;
	const 	vec2	dm 		= { 1024.0, 768.0 };
			vec3 	col 	= vec3(0.0, 0.0, 0.0);

	float 	rsq0 = sqrt	( mod( fc[1] * fc[0] + fc[1], 256.0 ) ) / 16.0;
	float 	rsq1 = sqrt	( mod( fc[0] * fc[0] + fc[1], 128.0 ) ) / 12.0;
	float 	rsq2 = sqrt	( mod( fc[1] * fc[1] + fc[0], 64.0 ) ) / 8.0;

	float 	rsqa = sqrt(rsq0 + rsq1 + rsq2);
			rsqa = rsqa * mod( fc[0] * fc[1] + fc[0], 128.0 );
			rsqa = rsqa - floor(rsqa);
			rsqa = rsqa * mod( fc[0] * fc[1] + fc[1], 256.0 );
			rsqa = rsqa - floor(rsqa);

	float 	center_dist_x 	= 1.0 - ( abs( fc[0] - dm[0] * 0.5 ) / (dm[0] * 0.5) );
	float 	center_dist_y 	= 1.0 - ( abs( fc[1] - dm[1] * 0.5 ) / (dm[1] * 0.5) );
	float 	center_blur 	= (center_dist_x*center_dist_y) + (center_dist_x*center_dist_y);
			center_blur 	= clamp(center_blur, 0.0, 1.0);
	float 	rsqa_center		= center_blur - rsqa;

	if(rsqa_center > 0.0) {
		if(rsqa > 0.85) { col[0] = 1.0; }

		if(	mod(fc[0], 61.0) 	<= 12.0	) {
			col[0] = 1.0;
		}

		if(	mod(fc[1], 119.0) 	<= 4.0	) {
			col[0] = 1.0;
		}

		if(	mod(fc[0], 17.0) 	<= 4.0 
		&&	mod(fc[1], 19.0) 	<= 4.0	) {
			col[0] = 1.0;
		}

		if(	mod(fc[0], 117.0) 	<= 40.0 
		&&	mod(fc[1], 119.0) 	<= 40.0	) {
			col[0] = 1.0;
		}

		if(	mod(fc[0], 41.0) >= mod(fc[1], 119.0) ) {
			col[0] = 1.0;
		}

		if(	mod(fc[0], 17.0) == mod(fc[1], 13.0) ) {
			col[0] = 1.0;
		}

		if(	fc[0] < dm[0] * 0.05
		|| 	fc[0] > dm[0] * 0.95
		|| 	fc[1] < dm[1] * 0.05
		|| 	fc[1] > dm[1] * 0.95 ) {
			col[0] = 0.0;
		}

		if(rsqa < 0.05) { col[0] = 0.0; }

		if(	fc[0] > dm[0] * 0.4
		&& 	fc[0] < dm[0] * 0.6
		&& 	fc[1] > dm[1] * 0.4
		&& 	fc[1] < dm[1] * 0.6 ) {
			col[0] = 1.0;
		}

		if(	fc[0] > dm[0] * 0.44
		&& 	fc[0] < dm[0] * 0.56
		&& 	fc[1] > dm[1] * 0.44
		&& 	fc[1] < dm[1] * 0.56 ) {
			col[0] = 0.0;
		}
	}

	out_col	= vec4(col, 1.0);
}
