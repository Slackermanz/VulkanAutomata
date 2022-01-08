#version 460

const float FILL_SCALE 	= 1.0;
const float TRQD_SCALE 	= FILL_SCALE + 2.0;

vec2 positions[3] = vec2[](
	vec2(-FILL_SCALE, -FILL_SCALE),
	vec2( TRQD_SCALE, -FILL_SCALE),
	vec2(-FILL_SCALE,  TRQD_SCALE)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
