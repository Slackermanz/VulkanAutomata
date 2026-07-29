#version 460
#define PI 3.14159265359
#define LN 2.71828182846

//  ----    ----    ----    ----    ----    ----    ----    ----

layout(location     =  0) out       vec4        out_col;
layout(binding      =  1) uniform   sampler2D   txdata;
layout(binding      =  0) uniform   UniBuf {
    uint v0;  uint v1;  uint v2;  uint v3;  uint v4;  uint v5;  uint v6;  uint v7;
    uint v8;  uint v9;  uint v10; uint v11; uint v12; uint v13; uint v14; uint v15;
    uint v16; uint v17; uint v18; uint v19; uint v20; uint v21; uint v22; uint v23;
    uint v24; uint v25; uint v26; uint v27; uint v28; uint v29; uint v30; uint v31;
    uint v32; uint v33; uint v34; uint v35; uint v36; uint v37; uint v38; uint v39;
    uint v40; uint v41; uint v42; uint v43; uint v44; uint v45; uint v46; uint v47;
    uint v48; uint v49; uint v50; uint v51; uint v52; uint v53; uint v54; uint v55;
    uint v56; uint v57; uint v58; uint v59; uint v60; uint v61; uint v62; uint v63; } ub;

struct UB64 { uint v[64]; };

layout(binding      =  2) readonly buffer SSBO {
    UB64 p[16];
} sb;

//  ----    ----    ----    ----    ----    ----    ----    ----

const uint MAX_RADIUS = 8u;

//  ----    ----    ----    ----    ----    ----    ----    ----

struct ConvData {
    vec4    value;
    float   total;
};

uint u32_upk(uint u32, uint bts, uint off) { return (u32 >> off) & ((1u << bts)-1u); }

float lmap() { return (gl_FragCoord[0] / textureSize(txdata,0)[0]); }
float vmap() { return (gl_FragCoord[1] / textureSize(txdata,0)[1]); }
float cmap() { return sqrt  ( ((gl_FragCoord[0] - textureSize(txdata,0)[0]*0.5) / textureSize(txdata,0)[0]*0.5)
                            * ((gl_FragCoord[0] - textureSize(txdata,0)[0]*0.5) / textureSize(txdata,0)[0]*0.5)
                            + ((gl_FragCoord[1] - textureSize(txdata,0)[1]*0.5) / textureSize(txdata,0)[1]*0.5)
                            * ((gl_FragCoord[1] - textureSize(txdata,0)[1]*0.5) / textureSize(txdata,0)[1]*0.5) ); }

float vwm() {
    float   scale_raw   = uintBitsToFloat(ub.v62);
    float   zoom        = uintBitsToFloat(ub.v61);
    float   scale_new   = scale_raw;
    uint    mode        = u32_upk(ub.v59, 2u, 0u);
    if( mode == 1u ) { //   Linear Parameter Map
        scale_new = ((lmap() + zoom) * (scale_raw / (1.0 + zoom * 2.0))) * 2.0; }
    if( mode == 2u ) { //   Circular Parameter Map
        scale_new = ((sqrt(cmap()) + zoom) * (scale_raw / (1.0 + zoom * 2.0))) * 2.0; }
    return scale_new; }

float  tp(uint n, float s)          { return (float(n+1u)/256.0) * ((s*0.5)/128.0); }
float tp2(uint n, float s)          { return (float(n+1u)/16.0) * ((s*0.5)/8.0); }
float tp3(uint n, float s)          { return (float(n+1u)/64.0) * ((s*0.5)/32.0); }
float bsn(uint v, uint  o)          { return float(u32_upk(v,1u,o)*2u)-1.0; }
float utp(uint v, uint  w, uint o)  { return tp(u32_upk(v,w,w*o), vwm()); }
float ut2(uint v, uint  w, uint o)  { return tp2(u32_upk(v,w,o), vwm()); }
float ut3(uint v, uint  w, uint o)  { return tp2(u32_upk(v,w,w*o), vwm()); }

vec4  sigm(vec4  x, float w) { return 1.0 / ( 1.0 + exp( (-w*2.0 * x * (PI/2.0)) + w * (PI/2.0) ) ); }

vec4  gdv( ivec2 of, sampler2D tx ) {
    of      = ivec2(gl_FragCoord) + of;
    of[0]   = (of[0] + textureSize(tx,0)[0]) & (textureSize(tx,0)[0]-1);
    of[1]   = (of[1] + textureSize(tx,0)[1]) & (textureSize(tx,0)[1]-1);
    return  texelFetch( tx, of, 0); }

ConvData ring( float r ) {

    const float psn = 32768.0;

    float tot = 0.0;
    vec4  val = vec4(0.0,0.0,0.0,0.0);

    float sq2   = sqrt(2.0);

    float o_0 = r + 0.5;
    float o_1 = sq2 * o_0;
    float o_2 = o_1 / 2.0;
    float o_3 = sqrt( o_0*o_0 - r*r );
    float o_4 = o_2 - ( floor(o_2) + 0.5 );
    float o_5 = floor( o_2 ) + floor( o_4 );

    float i_0 = r - 0.5;
    float i_1 = sq2 * i_0;
    float i_2 = i_1 / 2.0;
    float i_3 = sqrt( i_0*i_0 - r*r );
    float i_4 = i_2 - ( floor(i_2) + 1.0 );
    float i_5 = floor( i_2 ) + floor( i_4 );

    float d_0 = ( i_5 ) + 1.0 - ( o_5 );

    for(float i = 1.0; i < floor( i_2 ) + 1.0 - d_0; i++) {

        float j_0 = sqrt( o_0*o_0 - (i+0.0)*(i+0.0) );
        float j_1 = sqrt( i_0*i_0 - (i+0.0)*(i+0.0) );
        float j_2 = ( 1.0 - abs( sign ( (floor( i_2 ) + 1.0) - i ) ) );

        for(float j = floor( j_1 ) + j_2; j < floor( j_0 ); j++) {
            val += floor(gdv(ivec2( i, (j+1)), txdata) * psn);
            val += floor(gdv(ivec2( i,-(j+1)), txdata) * psn);
            val += floor(gdv(ivec2(-i,-(j+1)), txdata) * psn);
            val += floor(gdv(ivec2(-i, (j+1)), txdata) * psn);
            val += floor(gdv(ivec2( (j+1), i), txdata) * psn);
            val += floor(gdv(ivec2( (j+1),-i), txdata) * psn);
            val += floor(gdv(ivec2(-(j+1),-i), txdata) * psn);
            val += floor(gdv(ivec2(-(j+1), i), txdata) * psn);
            tot += 8.0 * psn; } }

//  Orthagonal
    val += floor(gdv(ivec2( r, 0), txdata) * psn);
    val += floor(gdv(ivec2( 0,-r), txdata) * psn);
    val += floor(gdv(ivec2(-r,-0), txdata) * psn);
    val += floor(gdv(ivec2(-0, r), txdata) * psn);
    tot += 4.0 * psn;

//  Diagonal
//  TODO This is not quite perfect
    float k_0 = r;
    float k_1 = sq2 * k_0;
    float k_2 = k_1 / 2.0;
    float k_3 = sqrt( k_0*k_0 - r*r );
    float k_4 = k_2 - ( floor(k_2) + 1.0 );
    float k_5 = floor( k_2 ) + floor( k_4 );

    float dist = round(k_2);

    if( sign( o_4 ) == -1.0 ) {
    //  val += gdv(ivec2( (floor(o_5)+1), floor(o_5)+1), txdata);
        val += floor(gdv(ivec2( (floor(o_5)+1), (floor(o_5)+1)), txdata) * psn);
        val += floor(gdv(ivec2( (floor(o_5)+1),-(floor(o_5)+1)), txdata) * psn);
        val += floor(gdv(ivec2(-(floor(o_5)+1),-(floor(o_5)+1)), txdata) * psn);
        val += floor(gdv(ivec2(-(floor(o_5)+1), (floor(o_5)+1)), txdata) * psn);
        tot += 4.0 * psn; }

    return ConvData( val, tot ); }

vec4 bitmake(ConvData[MAX_RADIUS] rings, uint bits, uint of) {
    vec4  sum = vec4(0.0,0.0,0.0,0.0);
    float tot = 0.0;
    for(uint i = 0u; i < MAX_RADIUS; i++) {
        if(u32_upk(bits, 1u, i+of) == 1u) { sum += rings[i].value; tot += rings[i].total; } }
        return sum / tot; }

//  ----    ----    ----    ----    ----    ----    ----    ----

//  Used to reseed the surface with lumpy noise
//  TODO - Breaks down at 2048+ resolution
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
    float   nhsz_c  = 0.0;
    float   xcn     = 0.0;
    float   nh_val  = 0.0;
    for(float i = -nhsz; i <= nhsz; i += 1.0) {
        for(float j = -nhsz; j <= nhsz; j += 1.0) {
            nh_val = round(sqrt(i*i+j*j));
            if(nh_val <= nhsz) {
                xcn = xcn + get_xcn(x, y, xm0, xm1, i, j);
                nhsz_c = nhsz_c + 1.0; } } }
    float   xcnf    = ( xcn / nhsz_c );
    float   xcaf    = xcnf;
    for(float i = 0.0; i <= nhsz; i += 1.0) {
            xcaf    = clamp((xcnf*xcaf + xcnf*xcaf) * (xcnf+xcnf), 0.0, 1.0); }
    return xcaf; }
float reseed(uint seed, float scl, float amp) {
    float   fx = gl_FragCoord[0];
    float   fy = gl_FragCoord[1];
    float   r0 = get_lump(fx, fy, round( 6.0  * scl), 19.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,17.0), 23.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,43.0));
    float   r1 = get_lump(fx, fy, round( 22.0 * scl), 13.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,29.0), 17.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,31.0));
    float   r2 = get_lump(fx, fy, round( 14.0 * scl), 13.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,11.0), 51.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,37.0));
    float   r3 = get_lump(fx, fy, round( 18.0 * scl), 29.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed, 7.0), 61.0 + mod(u32_upk(ub.v63, 24u, 0u)+seed,28.0));
    return clamp( sqrt((r0+r1)*r3*(amp+1.2))-r2*(amp*1.8+0.2) , 0.0, 1.0); }

vec4 place( vec4 col, float sz, vec2 mxy, uint s, float off ) {
    vec2 dxy = (vec2(gl_FragCoord) - mxy) * (vec2(gl_FragCoord) - mxy);
    float dist = sqrt(dxy[0] + dxy[1]);
    float cy = mod(u32_upk(ub.v63, 24u, 0u)+off, 213.0) / 213.0;
    float c2 = mod(u32_upk(ub.v63, 24u, 0u)+off, 377.0) / 377.0;
    float z2 = ((cos(2.0*PI*c2)/2.0)+0.5);
    float z3 = z2/4.0;
    float z4 = z2-z3;
    float ds = (1.0-dist/sz);
    float vr = (((cos((1.0*PI*4.0*cy)/2.0)+0.5) * z4 + z3) * ds * 0.85 + 0.38 * ds * ds);
    float vg = (((cos((2.0*PI*4.0*cy)/2.0)+0.5) * z4 + z3) * ds * 0.85 + 0.38 * ds * ds);
    float vb = (((cos((3.0*PI*4.0*cy)/2.0)+0.5) * z4 + z3) * ds * 0.85 + 0.38 * ds * ds);
    if(dist <= sz) { col += (s != 1u) ? vec4(-0.38,-0.38,-0.38,-0.38)*ds : vec4(vr,vg,vb,1.0); }
    return col; }

vec4 mouse(vec4 col, float sz) {
    vec2 mxy = vec2( u32_upk(ub.v60, 12u, 0u), u32_upk(ub.v60, 12u, 12u) );
    return place(col, sz, mxy, u32_upk(ub.v60, 2u, 24u), 0.0); }

vec4 symsd(vec4 col, float sz) {
    vec2 posxy = vec2(textureSize(txdata,0)[0]/2.0,textureSize(txdata,0)[1]/2.0);
    for(int i = 0; i < 11; i++) {
        uint sn = ((i&2u)==0u) ? 1u : 0u;
        col = place(col, (sz/11.0)*((11.0-i)),      posxy + vec2(  0.0, 0.0 ), sn, i*u32_upk(ub.v63, 24u, 0u));
        col = place(col, (sz/11.0)*((11.0-i))*0.5,  posxy + vec2(   sz, 0.0 ), sn, i*u32_upk(ub.v63, 24u, 0u));
        col = place(col, (sz/11.0)*((11.0-i))*0.5,  posxy + vec2(  -sz, 0.0 ), sn, i*u32_upk(ub.v63, 24u, 0u));
        col = place(col, (sz/11.0)*((11.0-i))*0.5,  posxy + vec2(  0.0,  sz ), sn, i*u32_upk(ub.v63, 24u, 0u));
        col = place(col, (sz/11.0)*((11.0-i))*0.5,  posxy + vec2(  0.0, -sz ), sn, i*u32_upk(ub.v63, 24u, 0u)); }
    return col; }

void main() {

//  ----    ----    ----    ----    ----    ----    ----    ----
//  Rule Initilisation
//  ----    ----    ----    ----    ----    ----    ----    ----

//  NH Rings
    ConvData[MAX_RADIUS] nh_rings_m;
    for(uint i = 0u; i < MAX_RADIUS; i++) { nh_rings_m[i] = ring(i+1.0); }

//  Output Values
    vec4 res_c = gdv( ivec2(0, 0), txdata );

//  Panel Index ID
    uint v_idx = uint(vmap()*4.0) * 4u + uint(lmap()*4.0);

    vec4 res_v = vec4(0.0,0.0,0.0,1.0);
    uint bt = 5u;
    float t = 31.0;

    for(uint i = 0u; i < 8u; i++) {

        // Get the average values of the color channels for the unique neighborhood kernel
        vec4 nhv = bitmake( nh_rings_m, sb.p[v_idx].v[i/(32u/MAX_RADIUS)], (i*MAX_RADIUS) & 31u );

        // Weights, from -1.0 to +1.0, one set for each color channel being read
        vec4 nnvr0 = vec4(
            (float(ut3( sb.p[v_idx].v[i*3u+2u+0u], bt,  0u )) / t)*bsn(sb.p[v_idx].v[i+26u],0u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+0u], bt,  1u )) / t)*bsn(sb.p[v_idx].v[i+26u],1u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+0u], bt,  2u )) / t)*bsn(sb.p[v_idx].v[i+26u],2u),
            1.0
        );

        vec4 nnvg0 = vec4(
            (float(ut3( sb.p[v_idx].v[i*3u+2u+0u], bt,  3u )) / t)*bsn(sb.p[v_idx].v[i+26u],3u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+0u], bt,  4u )) / t)*bsn(sb.p[v_idx].v[i+26u],4u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+0u], bt,  5u )) / t)*bsn(sb.p[v_idx].v[i+26u],5u),
            1.0
        );

        vec4 nnvb0 = vec4(
            (float(ut3( sb.p[v_idx].v[i*3u+2u+1u], bt,  0u )) / t)*bsn(sb.p[v_idx].v[i+26u],6u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+1u], bt,  1u )) / t)*bsn(sb.p[v_idx].v[i+26u],7u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+1u], bt,  2u )) / t)*bsn(sb.p[v_idx].v[i+26u],8u),
            1.0
        );

        // sub-result
        vec4 subres_0 = vec4(0.0,0.0,0.0,1.0);

        subres_0[0] = dot(nhv.rgb, nnvr0.rgb);
        subres_0[1] = dot(nhv.rgb, nnvg0.rgb);
        subres_0[2] = dot(nhv.rgb, nnvb0.rgb);

        vec4 nnvr1 = vec4(
            (float(ut3( sb.p[v_idx].v[i*3u+2u+1u], bt,  3u )) / t)*bsn(sb.p[v_idx].v[i+26u],9u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+1u], bt,  4u )) / t)*bsn(sb.p[v_idx].v[i+26u],10u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+1u], bt,  5u )) / t)*bsn(sb.p[v_idx].v[i+26u],11u),
            1.0
        );

        vec4 nnvg1 = vec4(
            (float(ut3( sb.p[v_idx].v[i*3u+2u+2u], bt,  0u )) / t)*bsn(sb.p[v_idx].v[i+26u],12u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+2u], bt,  1u )) / t)*bsn(sb.p[v_idx].v[i+26u],13u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+2u], bt,  2u )) / t)*bsn(sb.p[v_idx].v[i+26u],14u),
            1.0
        );

        vec4 nnvb1 = vec4(
            (float(ut3( sb.p[v_idx].v[i*3u+2u+2u], bt,  3u )) / t)*bsn(sb.p[v_idx].v[i+26u],15u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+2u], bt,  4u )) / t)*bsn(sb.p[v_idx].v[i+26u],16u),
            (float(ut3( sb.p[v_idx].v[i*3u+2u+2u], bt,  5u )) / t)*bsn(sb.p[v_idx].v[i+26u],17u),
            1.0
        );

        // sub-result
        vec4 subres_1 = vec4(0.0,0.0,0.0,1.0);

        subres_1[0] = dot(subres_0.rgb, nnvr1.rgb);
        subres_1[1] = dot(subres_0.rgb, nnvg1.rgb);
        subres_1[2] = dot(subres_0.rgb, nnvb1.rgb);

        vec4 nnvr2 = vec4(
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)], bt,  (i&1u)*3u+0u )) / t)*bsn(sb.p[v_idx].v[i+26u],18u),
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)], bt,  (i&1u)*3u+1u )) / t)*bsn(sb.p[v_idx].v[i+26u],19u),
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)], bt,  (i&1u)*3u+2u )) / t)*bsn(sb.p[v_idx].v[i+26u],20u),
            1.0
        );

        vec4 nnvg2 = vec4(
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)*2u], bt,  (1u-(i&1u))*3u+0u )) / t)*bsn(sb.p[v_idx].v[i+26u],21u),
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)*2u], bt,  (1u-(i&1u))*3u+1u )) / t)*bsn(sb.p[v_idx].v[i+26u],22u),
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)*2u], bt,  (1u-(i&1u))*3u+2u )) / t)*bsn(sb.p[v_idx].v[i+26u],23u),
            1.0
        );

        vec4 nnvb2 = vec4(
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)+1u], bt,  (i&1u)*3u+0u )) / t)*bsn(sb.p[v_idx].v[i+26u],24u),
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)+1u], bt,  (i&1u)*3u+1u )) / t)*bsn(sb.p[v_idx].v[i+26u],25u),
            (float(ut3( sb.p[v_idx].v[34u+(i/2u)*3u+(i&1u)+1u], bt,  (i&1u)*3u+2u )) / t)*bsn(sb.p[v_idx].v[i+26u],26u),
            1.0
        );

        // sub-result
        vec4 subres_2 = vec4(0.0,0.0,0.0,1.0);

        subres_2[0] = dot(subres_1.rgb, nnvr2.rgb);
        subres_2[1] = dot(subres_1.rgb, nnvg2.rgb);
        subres_2[2] = dot(subres_1.rgb, nnvb2.rgb);

        res_v += subres_2;
    }

    res_c = res_v;

//  ----    ----    ----    ----    ----    ----    ----    ----
//  Shader Output
//  ----    ----    ----    ----    ----    ----    ----    ----


    if( u32_upk(ub.v63, 24u, 0u) <= 0u
    ||  u32_upk(ub.v60, 6u, 26u) == 1u ) {
        res_c[0] = reseed( u32_upk(ub.v63, 8u, 24u) + 0u, 1.0, 0.4 ); 
        res_c[1] = reseed( u32_upk(ub.v63, 8u, 24u) + 1u, 1.0, 0.4 ); 
        res_c[2] = reseed( u32_upk(ub.v63, 8u, 24u) + 2u, 1.0, 0.4 ); 
        res_c[3] = reseed( u32_upk(ub.v63, 8u, 24u) + 3u, 1.0, 0.4 ); }

    if( u32_upk(ub.v60, 6u, 26u) == 2u ) {
        res_c[0] = 0.0; 
        res_c[1] = 0.0; 
        res_c[2] = 0.0; 
        res_c[3] = 1.0; }

    if( u32_upk(ub.v60, 6u, 26u) == 3u ) {
        res_c = symsd(res_c, 128.0); }

    if( u32_upk(ub.v60, 6u, 26u) == 4u ) {
        
        res_c[0] = reseed( u32_upk(ub.v63, 8u, 24u) + 0u, clamp(2.0-(cmap())*8.0,0.01,2.0), 0.4 ); 
        res_c[1] = reseed( u32_upk(ub.v63, 8u, 24u) + 1u, clamp(2.0-(cmap())*8.0,0.01,2.0), 0.4 ); 
        res_c[2] = reseed( u32_upk(ub.v63, 8u, 24u) + 2u, clamp(2.0-(cmap())*8.0,0.01,2.0), 0.4 ); 
        res_c[3] = reseed( u32_upk(ub.v63, 8u, 24u) + 3u, clamp(2.0-(cmap())*8.0,0.01,2.0), 0.4 );

        float res_c_mask = reseed( u32_upk(ub.v63, 8u, 24u) + 4u, 2.0, 0.05 );

        res_c[0] -= res_c_mask;
        res_c[1] -= res_c_mask;
        res_c[2] -= res_c_mask;

     }

    if(u32_upk(ub.v60, 2u, 24u) != 0u) {
        //res_c = mouse_px(res_c, 1.0);
        res_c = mouse(res_c, 64.0);
    }

//  Force alpha to 1.0
    res_c[3]    = 1.0;

    out_col     = res_c;

}
