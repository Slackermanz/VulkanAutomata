#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_col;

layout(binding = 0) uniform init_u0 {
    float uw;
	float uh;
	float us;
} u0;

vec3 setCol(int col_idx, float col_val, vec3 col) {
	if(col_val > 1.0 || col_val < 0.0) { col_val = 0.0; }
	col[col_idx] = col_val;
	return col;
}

void main() {
	const vec4	fc 	= gl_FragCoord;
	const float pxo = 1.0 / u0.uw;
	const float pyo = 1.0 / u0.uh;
	const float fcx = fc[0]*pxo;
	const float fcy = fc[1]*pyo;

	vec3 col = { 0.0, 0.0, 0.0 };
	float rx = 0.0;
	float ry = 0.0;

	rx 	= mod( fc[0]*u0.uw, 19.0);
	ry 	= mod( fc[1]*u0.uh, 17.0);
	if((rx == ry || rx + 15.0 <= ry) && fcx > 0.25) {
		col = setCol(0, 1.0, col);
		col = setCol(1, 1.0, col);
		col = setCol(2, 1.0, col);
	} /**/

	rx 	= mod( fc[0]*u0.uw, 61.0);
	ry 	= mod( fc[1]*u0.uh, 47.0);
	if(rx + 30.0 <= ry && fcx > 0.5) {
		col = setCol(0, 1.0, col);
		col = setCol(1, 1.0, col);
		col = setCol(2, 1.0, col);
	} /**/

	rx 	= mod( fc[0]*u0.uw, 13.0);
	ry 	= mod( fc[1]*u0.uh, 197.0);
	if(rx >= ry && fcx > 0.75) {
		col = setCol(0, 1.0, col);
		col = setCol(1, 1.0, col);
		col = setCol(2, 1.0, col);
	} /**/

	rx 	= mod( fc[0]*u0.uw, 97.0);
	ry 	= mod( fc[1]*u0.uh, 181.0);
	if(rx >= ry && fcx > 0.75) {
		col = setCol(0, 1.0, col);
		col = setCol(1, 1.0, col);
		col = setCol(2, 1.0, col);
	} /**/

	rx 	= mod( fc[0]*u0.uw, 101.0);
	ry 	= mod( fc[1]*u0.uh, 201.0);
	if(rx >= ry && fcx <= 0.75) {
		col = setCol(0, 0.0, col);
		col = setCol(1, 0.0, col);
		col = setCol(2, 0.0, col);
	} /**/

	if(fcx > 0.15 && fcx < 0.85 && fcy > 0.45 && fcy < 0.55) {
		col = setCol(0, 1.0, col);
		col = setCol(1, 1.0, col);
		col = setCol(2, 1.0, col);
	} /**/

	if(fcx > 0.45 && fcx < 0.55 && fcy > 0.15 && fcy < 0.85) {
		col = setCol(0, 1.0, col);
		col = setCol(1, 1.0, col);
		col = setCol(2, 1.0, col);
	} /**/

	if(fcx > 0.25 && fcx < 0.75 && fcy > 0.48 && fcy < 0.52) {
		col = setCol(0, 0.0, col);
		col = setCol(1, 0.0, col);
		col = setCol(2, 0.0, col);
	} /**/

	if(fcx > 0.48 && fcx < 0.52 && fcy > 0.25 && fcy < 0.75) {
		col = setCol(0, 0.0, col);
		col = setCol(1, 0.0, col);
		col = setCol(2, 0.0, col);
	} /**/

	rx 	= mod( fc[0]*u0.uw, 29.0);
	ry 	= mod( fc[1]*u0.uh, u0.us);
	if(fcy > 0.9 && rx <= ry) {
		col = setCol(0, 0.0, col);
		col = setCol(1, 1.0, col);
		col = setCol(2, 0.0, col);
	} /**/

	out_col = vec4(col, 1.0);

}
