#version 460
#define PI 3.14159265359

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
const uint PAIR_COUNT = 6u;
const uint MAG_BASE = 3u;   // v3..v38: six 5-bit coefficient magnitudes per word
const uint SIGN_BASE = 39u; // v39..v45: one sign bit per coefficient

// Pair-selective MNCA profile selection.
//
// DENSE_PAIR_SUM is the nonselective baseline. All six pair proposals are
// summed directly with no scoring, selection, or discarding. This is the
// pair-native equivalent of the old depth-2 perceptron baseline.
//
// SINGLE_CHANNEL is a degenerate test mode. Only the red channel is used for
// simulation; green and blue mirror red for display. Each neighborhood has a
// scalar weight instead of a matrix transform. The number of neighborhoods is
// controlled by SC_NEIGHBORHOOD_COUNT. This tests whether the dot-product
// machinery adds anything beyond scalar weighted sums of ring averages.
//
// GENTLE_TOP1 uses stateless replacement and selects the candidate with the
// smallest non-zero deviation from current. Instead of rewarding the loudest
// change, it rewards the gentlest viable transition.
//
// WEIGHTED_TOP1 uses a feature-vector scorer with genotype-owned weights.
// Candidate utility is computed from deviation, proposal value, and
// neighbourhood conformance, then compared against a genotype-owned no-op threshold.
//
// RAW_TOP1_REPLACE uses stateless replacement instead of accumulation. The
// winning pair's raw output becomes the next cell state directly.
//
// RAW_TOP1 uses accumulation (current + impulse). It restores motility but
// creates a ratchet.
//
// RAW_TOP2 keeps raw proposals and deviation scoring, but blends the two best
// retained proposals.
//
// GOVERNED_TOP2 is the first successful civilized profile: bounded headroom
// proposal, relation-band scoring, saturation safety, top-two synthesis, and
// authority-gated output.
#define PSPMNCA_PROFILE_DENSE_PAIR_SUM 0
#define PSPMNCA_PROFILE_SINGLE_CHANNEL 1
#define PSPMNCA_PROFILE_GENTLE_TOP1 0
#define PSPMNCA_PROFILE_WEIGHTED_TOP1 0
#define PSPMNCA_PROFILE_RAW_TOP1_REPLACE 0
#define PSPMNCA_PROFILE_RAW_TOP1 0
#define PSPMNCA_PROFILE_RAW_TOP2 0
#define PSPMNCA_PROFILE_GOVERNED_TOP2 0

#if (PSPMNCA_PROFILE_DENSE_PAIR_SUM + PSPMNCA_PROFILE_SINGLE_CHANNEL + PSPMNCA_PROFILE_GENTLE_TOP1 + PSPMNCA_PROFILE_WEIGHTED_TOP1 + PSPMNCA_PROFILE_RAW_TOP1_REPLACE + PSPMNCA_PROFILE_RAW_TOP1 + PSPMNCA_PROFILE_RAW_TOP2 + PSPMNCA_PROFILE_GOVERNED_TOP2) != 1
#error Select exactly one PSPMNCA profile.
#endif

#if PSPMNCA_PROFILE_WEIGHTED_TOP1
// Genome-owned scoring weights for feature-vector selection.
//
// v46 bits  0..29: weights 0..5
// v47 bits  0..29: weights 6..11
//
// Each weight is one signed 5-bit field:
//   bits 0..3: magnitude, 0..15 mapped to 0.0..1.0
//   bit  4:    sign, 0 = negative, 1 = positive
//
// Weight order:
//   0..2:   wd.rgb, deviation magnitude weight
//   3..5:   wp.rgb, proposal value weight
//   6..8:   wn.rgb, neighbourhood conformance weight
//   9..11:  wb.rgb, no-op threshold weight
float score_weight(uint panel_idx, uint weight_idx) {
    uint word = sb.p[panel_idx].v[46u + (weight_idx / 6u)];
    uint raw = (word >> ((weight_idx % 6u) * 5u)) & 31u;
    float mag = float(raw & 15u) / 15.0;
    float sgn = ((raw >> 4u) == 0u) ? -1.0 : 1.0;
    return mag * sgn;
}

vec3 score_weight3(uint panel_idx, uint base_idx) {
    return vec3(
        score_weight(panel_idx, base_idx + 0u),
        score_weight(panel_idx, base_idx + 1u),
        score_weight(panel_idx, base_idx + 2u));
}
#endif

#define PSPMNCA_USE_ENERGY_TAX 1

// Experimental scorer override for inspecting neighbourhood conformance as the
// universal selection law. When enabled, WEIGHTED_TOP1 ignores v46/v47 scorer
// weights and uses score = 1 - min(distance to neighbourhood A, distance to neighbourhood B).
#define PSPMNCA_FORCE_CONFORMANCE_SCORER 1

// Single-channel degenerate test configuration.
// When PSPMNCA_PROFILE_SINGLE_CHANNEL is enabled, this controls the number
// of neighborhoods. Each neighborhood uses: 8-bit mask, 5-bit weight, 1-bit sign.
// Layout: masks at v0, magnitudes after masks, signs after magnitudes.
// Maximum: approximately 104 neighborhoods with 48-word genotype budget.
#define SC_NEIGHBORHOOD_COUNT 8
#define SC_ACTIVATIONS_PER_NEIGHBORHOOD 0
#define SC_USE_SIGN 1
#define SC_USE_WEIGHT 0
#define SC_USE_STEP 0
#define SC_STEP_MULTIPLY_BY_NEIGHBORHOOD 1
#define SC_BASELINE_PASSTHROUGH 1
#define SC_FIXED_STEP_CONTRIB 1
#define SC_USE_SIZE_PRIORITY 1
#define SC_SIZE_PRIORITY_REF_PIXELS 8.0
#define SC_USE_SCALE_LADDER 1
#define SC_SCALE_LADDER_COUNT 7
#define SC_SCALE_LADDER_USE_MUTATION_MASKS 1
#define SC_SCALE_LADDER_PREBLEND_CURRENT 1
#define SC_SCALE_SELECT_MIN_ABS_INTERVAL 1
#define SC_SCALE_SELECT_MAX_ABS_INTERVAL 0
#define SC_SCALE_APPLY_SIGNED_STEP 1
#define SC_SCALE_APPLY_SELECTED_BIAS 1
#define SC_SCALE_VARIANCE_STEP (96.0 / 4032.0)
#define SC_SCALE_VARIANCE_BIAS (12.0 / 4032.0)
#define SC_MICROSTEP (512.0 / 65536.0)

#define SC_MASK_WORDS ((SC_NEIGHBORHOOD_COUNT + 3) / 4)

#if SC_USE_STEP
#define SC_CONTRIB_COUNT (SC_NEIGHBORHOOD_COUNT * SC_ACTIVATIONS_PER_NEIGHBORHOOD)
#define SC_RANGE_WORDS ((SC_CONTRIB_COUNT + 2) / 3)
#else
#define SC_CONTRIB_COUNT SC_NEIGHBORHOOD_COUNT
#define SC_RANGE_WORDS 0
#endif

#if SC_USE_WEIGHT
#define SC_MAG_WORDS ((SC_CONTRIB_COUNT + 5) / 6)
#else
#define SC_MAG_WORDS 0
#endif

#if SC_USE_SIGN
#define SC_SIGN_WORDS ((SC_CONTRIB_COUNT + 31) / 32)
#else
#define SC_SIGN_WORDS 0
#endif

#define SC_TOTAL_WORDS (SC_MASK_WORDS + SC_RANGE_WORDS + SC_MAG_WORDS + SC_SIGN_WORDS)

#if SC_TOTAL_WORDS > 48
#error SINGLE_CHANNEL scalar configuration exceeds the 48-word genotype budget.
#endif

#if SC_USE_SCALE_LADDER && SC_SCALE_LADDER_COUNT < 1
#error SC_USE_SCALE_LADDER requires at least one scale sample.
#endif

#if SC_USE_SCALE_LADDER && SC_SCALE_LADDER_USE_MUTATION_MASKS && SC_SCALE_LADDER_COUNT > SC_NEIGHBORHOOD_COUNT
#error SC_SCALE_LADDER_USE_MUTATION_MASKS requires SC_SCALE_LADDER_COUNT <= SC_NEIGHBORHOOD_COUNT.
#endif

#if (SC_SCALE_SELECT_MIN_ABS_INTERVAL + SC_SCALE_SELECT_MAX_ABS_INTERVAL) > 1
#error Select at most one scalar scale interval selection policy.
#endif

const float PAIR_STEP = 0.125;
const float ENERGY_UNIT = 1.0 / 65536.0;
const float ENERGY_TAX = 16.0 * ENERGY_UNIT;

//  ----    ----    ----    ----    ----    ----    ----    ----

struct ConvData {
    vec4    value;
    float   total;
};

struct PairEval {
    uint    mask_a;
    uint    mask_b;
    float   support;
    vec3    nh_a;
    vec3    nh_b;
    vec3    latent_a;
    vec3    latent_b;
    vec3    pred_a;
    vec3    pred_b;
    vec3    proposal;
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

float tp2(uint n, float s)          { return (float(n+1u)/16.0) * ((s*0.5)/8.0); }
float bsn(uint v, uint  o)          { return float(u32_upk(v,1u,o)*2u)-1.0; }
float ut3(uint v, uint  w, uint o)  { return tp2(u32_upk(v,w,w*o), vwm()); }


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

ConvData bitmake_data(ConvData[MAX_RADIUS] rings, uint bits, uint of) {
    vec4  sum = vec4(0.0,0.0,0.0,0.0);
    float tot = 0.0;
    for(uint i = 0u; i < MAX_RADIUS; i++) {
        if(u32_upk(bits, 1u,i+of) == 1u) { sum += rings[i].value; tot += rings[i].total; } }
    return ConvData(sum, tot); }

vec4 bitmake(ConvData[MAX_RADIUS] rings, uint bits, uint of) {
    ConvData data = bitmake_data(rings, bits, of);
    return (data.total > 0.0) ? data.value / data.total : vec4(0.0); }

float sc_size_priority(ConvData data) {
#if SC_USE_SIZE_PRIORITY
    return min(1.0, (SC_SIZE_PRIORITY_REF_PIXELS * 32768.0) / max(data.total, 1.0));
#else
    return 1.0;
#endif
}

uint sc_scale_ladder_radius(uint idx) {
    uint denom = (uint(SC_SCALE_LADDER_COUNT) > 1u) ? (uint(SC_SCALE_LADDER_COUNT) - 1u) : 1u;
    return min(1u + (idx * (MAX_RADIUS - 1u)) / denom, MAX_RADIUS);
}

uint sc_scalar_mask(uint panel_idx, uint idx) {
    return u32_upk(sb.p[panel_idx].v[idx / 4u], 8u, (idx % 4u) * 8u);
}

uint sc_scale_ladder_mask(uint panel_idx, uint idx) {
#if SC_SCALE_LADDER_USE_MUTATION_MASKS
    return sc_scalar_mask(panel_idx, idx);
#else
    uint radius = sc_scale_ladder_radius(idx);
    return (radius >= MAX_RADIUS) ? ((1u << MAX_RADIUS) - 1u) : ((1u << radius) - 1u);
#endif
}

uint pair_mask(uint panel_idx, uint head_idx) {
    return u32_upk(sb.p[panel_idx].v[head_idx / 4u], 8u, (head_idx & 3u) * 8u);
}

float pair_coeff_mag(uint panel_idx, uint pair_idx, uint coeff_idx) {
    uint word_idx = MAG_BASE + pair_idx * 6u + coeff_idx / 6u;
    uint field_idx = coeff_idx % 6u;
    return float(ut3(sb.p[panel_idx].v[word_idx], 5u, field_idx)) / 31.0;
}

float pair_coeff_sign(uint panel_idx, uint pair_idx, uint coeff_idx) {
    uint sign_idx = pair_idx * 36u + coeff_idx;
    return bsn(sb.p[panel_idx].v[SIGN_BASE + sign_idx / 32u], sign_idx & 31u);
}

float pair_coeff(uint panel_idx, uint pair_idx, uint coeff_idx) {
    return pair_coeff_mag(panel_idx, pair_idx, coeff_idx) * pair_coeff_sign(panel_idx, pair_idx, coeff_idx);
}

vec3 pair_side_encode(uint panel_idx, uint pair_idx, uint coeff_base, vec3 nh) {
    return vec3(
        dot(nh, vec3(
            pair_coeff(panel_idx, pair_idx, coeff_base + 0u),
            pair_coeff(panel_idx, pair_idx, coeff_base + 1u),
            pair_coeff(panel_idx, pair_idx, coeff_base + 2u))),
        dot(nh, vec3(
            pair_coeff(panel_idx, pair_idx, coeff_base + 3u),
            pair_coeff(panel_idx, pair_idx, coeff_base + 4u),
            pair_coeff(panel_idx, pair_idx, coeff_base + 5u))),
        dot(nh, vec3(
            pair_coeff(panel_idx, pair_idx, coeff_base + 6u),
            pair_coeff(panel_idx, pair_idx, coeff_base + 7u),
            pair_coeff(panel_idx, pair_idx, coeff_base + 8u))));
}

vec3 pair_terminal_a(uint panel_idx, uint pair_idx, vec3 a) {
    return vec3(
        dot(a, vec3(
            pair_coeff(panel_idx, pair_idx, 18u),
            pair_coeff(panel_idx, pair_idx, 19u),
            pair_coeff(panel_idx, pair_idx, 20u))),
        dot(a, vec3(
            pair_coeff(panel_idx, pair_idx, 24u),
            pair_coeff(panel_idx, pair_idx, 25u),
            pair_coeff(panel_idx, pair_idx, 26u))),
        dot(a, vec3(
            pair_coeff(panel_idx, pair_idx, 30u),
            pair_coeff(panel_idx, pair_idx, 31u),
            pair_coeff(panel_idx, pair_idx, 32u))));
}

vec3 pair_terminal_b(uint panel_idx, uint pair_idx, vec3 b) {
    return vec3(
        dot(b, vec3(
            pair_coeff(panel_idx, pair_idx, 21u),
            pair_coeff(panel_idx, pair_idx, 22u),
            pair_coeff(panel_idx, pair_idx, 23u))),
        dot(b, vec3(
            pair_coeff(panel_idx, pair_idx, 27u),
            pair_coeff(panel_idx, pair_idx, 28u),
            pair_coeff(panel_idx, pair_idx, 29u))),
        dot(b, vec3(
            pair_coeff(panel_idx, pair_idx, 33u),
            pair_coeff(panel_idx, pair_idx, 34u),
            pair_coeff(panel_idx, pair_idx, 35u))));
}


PairEval eval_pair(uint panel_idx, uint pair_idx, ConvData[MAX_RADIUS] rings) {
    PairEval candidate;
    uint head_a = pair_idx * 2u;
    uint head_b = head_a + 1u;

    candidate.mask_a = pair_mask(panel_idx, head_a);
    candidate.mask_b = pair_mask(panel_idx, head_b);
    candidate.support = ((candidate.mask_a != 0u) && (candidate.mask_b != 0u)) ? 1.0 : 0.0;

    candidate.nh_a = bitmake(rings, candidate.mask_a, 0u).rgb;
    candidate.nh_b = bitmake(rings, candidate.mask_b, 0u).rgb;

    candidate.latent_a = pair_side_encode(panel_idx, pair_idx, 0u, candidate.nh_a);
    candidate.latent_b = pair_side_encode(panel_idx, pair_idx, 9u, candidate.nh_b);

    candidate.pred_a = pair_terminal_a(panel_idx, pair_idx, candidate.latent_a);
    candidate.pred_b = pair_terminal_b(panel_idx, pair_idx, candidate.latent_b);
    candidate.proposal = candidate.pred_a + candidate.pred_b;

    return candidate;
}

vec3 pair_neighbourhood_error(PairEval candidate) {
    return min(abs(candidate.proposal - candidate.nh_a), abs(candidate.proposal - candidate.nh_b));
}

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
#if PSPMNCA_USE_ENERGY_TAX
    res_c.rgb = max(res_c.rgb - vec3(ENERGY_TAX), vec3(0.0));
#endif

//  Panel Index ID
    uint v_idx = uint(vmap()*4.0) * 4u + uint(lmap()*4.0);

    vec3 current = res_c.rgb;

#if PSPMNCA_PROFILE_DENSE_PAIR_SUM
    vec3 dense_sum = vec3(0.0);
    for(uint p = 0u; p < PAIR_COUNT; p++) {
        PairEval candidate = eval_pair(v_idx, p, nh_rings_m);
        dense_sum += candidate.proposal;
    }
    res_c.rgb = dense_sum;
#elif PSPMNCA_PROFILE_SINGLE_CHANNEL
    const uint SC_MASK_BASE = 0u;
    const uint SC_RANGE_OFF = uint(SC_MASK_WORDS);
    const uint SC_MAG_OFF = SC_RANGE_OFF + uint(SC_RANGE_WORDS);
    const uint SC_SIGN_OFF = SC_MAG_OFF + uint(SC_MAG_WORDS);

#if SC_BASELINE_PASSTHROUGH
    float sc_sum = current.r;
#else
    float sc_sum = 0.0;
#endif

#if SC_USE_SCALE_LADDER
    float sc_scale_values[SC_SCALE_LADDER_COUNT];

    for (uint i = 0u; i < uint(SC_SCALE_LADDER_COUNT); i++) {
        ConvData scale_data = bitmake_data(nh_rings_m, sc_scale_ladder_mask(v_idx, i), 0u);
        float scale_nh = (scale_data.total > 0.0) ? scale_data.value.r / scale_data.total : 0.0;
#if SC_SCALE_LADDER_PREBLEND_CURRENT
        sc_scale_values[i] = (current.r + scale_nh * SC_SCALE_VARIANCE_STEP) / (1.0 + SC_SCALE_VARIANCE_STEP);
#else
        sc_scale_values[i] = scale_nh;
#endif
    }

    uint selected_scale = 0u;
    float selected_variance = current.r - sc_scale_values[0];

    for (uint i = 1u; i < uint(SC_SCALE_LADDER_COUNT); i++) {
        float variance = sc_scale_values[i - 1u] - sc_scale_values[i];
        bool select_interval = false;
#if SC_SCALE_SELECT_MIN_ABS_INTERVAL
        select_interval = abs(selected_variance) > abs(variance);
#endif
#if SC_SCALE_SELECT_MAX_ABS_INTERVAL
        select_interval = abs(selected_variance) < abs(variance);
#endif
        if (select_interval) {
            selected_scale = i;
            selected_variance = variance;
        }
    }

#if SC_SCALE_APPLY_SIGNED_STEP
    sc_sum += sign(selected_variance) * SC_SCALE_VARIANCE_STEP;
#endif
#if SC_SCALE_APPLY_SELECTED_BIAS
    sc_sum += sc_scale_values[selected_scale] * SC_SCALE_VARIANCE_BIAS;
#endif
#else
    for (uint n = 0u; n < uint(SC_NEIGHBORHOOD_COUNT); n++) {
        uint mask = sc_scalar_mask(v_idx, n);
        ConvData sc_nh_data = bitmake_data(nh_rings_m, mask, 0u);
        float nh = (sc_nh_data.total > 0.0) ? sc_nh_data.value.r / sc_nh_data.total : 0.0;
        float size_priority = sc_size_priority(sc_nh_data);

#if SC_USE_STEP
        for (uint a = 0u; a < uint(SC_ACTIVATIONS_PER_NEIGHBORHOOD); a++) {
            uint ci = n * uint(SC_ACTIVATIONS_PER_NEIGHBORHOOD) + a;
            uint range_word = sb.p[v_idx].v[SC_RANGE_OFF + ci / 3u];
            uint range_off = (ci % 3u) * 10u;
            float lo = float(u32_upk(range_word, 5u, range_off + 0u)) / 31.0;
            float hi = float(u32_upk(range_word, 5u, range_off + 5u)) / 31.0;
            float active = step(lo, nh) * step(nh, hi);

            float contrib = active;
#if SC_STEP_MULTIPLY_BY_NEIGHBORHOOD
#if SC_FIXED_STEP_CONTRIB
            contrib *= min(nh, SC_MICROSTEP);
#else
            contrib *= nh;
#endif
#endif
            contrib *= size_priority;
#if SC_USE_WEIGHT
            contrib *= ut3(sb.p[v_idx].v[SC_MAG_OFF + ci / 6u], 5u, ci % 6u);
#endif
#if SC_USE_SIGN
            contrib *= bsn(sb.p[v_idx].v[SC_SIGN_OFF + ci / 32u], ci % 32u);
#endif
            sc_sum += contrib;
        }
#else
#if SC_FIXED_STEP_CONTRIB
        float contrib = min(nh, SC_MICROSTEP);
#else
        float contrib = nh;
#endif
        contrib *= size_priority;
#if SC_USE_WEIGHT
        contrib *= ut3(sb.p[v_idx].v[SC_MAG_OFF + n / 6u], 5u, n % 6u);
#endif
#if SC_USE_SIGN
        contrib *= bsn(sb.p[v_idx].v[SC_SIGN_OFF + n / 32u], n % 32u);
#endif
        sc_sum += contrib;
#endif
    }
#endif

    res_c.rgb = vec3(sc_sum);
#else
    vec3 best_score = vec3(0.0);
    vec3 best_value = current;
    vec3 second_score = vec3(0.0);
    vec3 second_value = current;

    for(uint p = 0u; p < PAIR_COUNT; p++) {
        PairEval candidate = eval_pair(v_idx, p, nh_rings_m);
        vec3 impulse = candidate.proposal;

#if PSPMNCA_PROFILE_WEIGHTED_TOP1
        vec3 proposal = candidate.proposal;
        float support = candidate.support;
        vec3 neighbourhood_error = pair_neighbourhood_error(candidate);
#if PSPMNCA_FORCE_CONFORMANCE_SCORER
        vec3 score = support * (vec3(1.0) - neighbourhood_error);
#else
        vec3 score_wd = score_weight3(v_idx, 0u);
        vec3 score_wp = score_weight3(v_idx, 3u);
        vec3 score_wn = score_weight3(v_idx, 6u);
        vec3 score_threshold = score_weight3(v_idx, 9u);
        vec3 score = support * (
            score_wd * abs(proposal - current)
        +   score_wp * proposal
        +   score_wn * neighbourhood_error
        -   score_threshold );
#endif
#endif

#if PSPMNCA_PROFILE_RAW_TOP1_REPLACE || PSPMNCA_PROFILE_GENTLE_TOP1
        vec3 proposal = candidate.proposal;
        float support = candidate.support;
        vec3 score = support * abs(proposal - current);
#endif

#if PSPMNCA_PROFILE_RAW_TOP1 || PSPMNCA_PROFILE_RAW_TOP2
        vec3 proposal = current + candidate.proposal;
        float support = candidate.support;
        vec3 score = support * abs(proposal - current);
#endif

#if PSPMNCA_PROFILE_GOVERNED_TOP2
        vec3 delta = candidate.proposal / (vec3(1.0) + abs(candidate.proposal));
        vec3 proposal = current
            + PAIR_STEP * (
                max(delta, vec3(0.0)) * (vec3(1.0) - current)
            +   min(delta, vec3(0.0)) * current );

        vec3 relation = abs(candidate.latent_a - candidate.latent_b);
        vec3 relation_norm = relation / (vec3(1.0) + relation);
        vec3 relation_band = vec3(4.0) * relation_norm * (vec3(1.0) - relation_norm);
        vec3 saturation_safety = vec3(4.0) * proposal * (vec3(1.0) - proposal);
        float support = candidate.support;
        vec3 score = support * relation_band * saturation_safety;
#endif

#if PSPMNCA_PROFILE_GENTLE_TOP1
        if(score.r > 0.0) {
            if(best_score.r == 0.0 || score.r < best_score.r) {
                second_score.r = best_score.r;
                second_value.r = best_value.r;
                best_score.r = score.r;
                best_value.r = proposal.r;
            } else if(second_score.r == 0.0 || score.r < second_score.r) {
                second_score.r = score.r;
                second_value.r = proposal.r;
            }
        }

        if(score.g > 0.0) {
            if(best_score.g == 0.0 || score.g < best_score.g) {
                second_score.g = best_score.g;
                second_value.g = best_value.g;
                best_score.g = score.g;
                best_value.g = proposal.g;
            } else if(second_score.g == 0.0 || score.g < second_score.g) {
                second_score.g = score.g;
                second_value.g = proposal.g;
            }
        }

        if(score.b > 0.0) {
            if(best_score.b == 0.0 || score.b < best_score.b) {
                second_score.b = best_score.b;
                second_value.b = best_value.b;
                best_score.b = score.b;
                best_value.b = proposal.b;
            } else if(second_score.b == 0.0 || score.b < second_score.b) {
                second_score.b = score.b;
                second_value.b = proposal.b;
            }
        }
#else
        if(score.r > best_score.r) {
            second_score.r = best_score.r;
            second_value.r = best_value.r;
            best_score.r = score.r;
            best_value.r = proposal.r;
        } else if(score.r > second_score.r) {
            second_score.r = score.r;
            second_value.r = proposal.r;
        }

        if(score.g > best_score.g) {
            second_score.g = best_score.g;
            second_value.g = best_value.g;
            best_score.g = score.g;
            best_value.g = proposal.g;
        } else if(score.g > second_score.g) {
            second_score.g = score.g;
            second_value.g = proposal.g;
        }

        if(score.b > best_score.b) {
            second_score.b = best_score.b;
            second_value.b = best_value.b;
            best_score.b = score.b;
            best_value.b = proposal.b;
        } else if(score.b > second_score.b) {
            second_score.b = score.b;
            second_value.b = proposal.b;
        }
#endif
    }

#if PSPMNCA_PROFILE_RAW_TOP1 || PSPMNCA_PROFILE_RAW_TOP1_REPLACE || PSPMNCA_PROFILE_GENTLE_TOP1 || PSPMNCA_PROFILE_WEIGHTED_TOP1
    res_c.rgb = vec3(
        (best_score.r > 0.0) ? best_value.r : current.r,
        (best_score.g > 0.0) ? best_value.g : current.g,
        (best_score.b > 0.0) ? best_value.b : current.b);
#endif

#if PSPMNCA_PROFILE_RAW_TOP2
    vec3 retained_score = best_score + second_score;
    vec3 retained_value =
        (best_value * best_score + second_value * second_score)
        / max(retained_score, vec3(0.000001));

    res_c.rgb = vec3(
        (retained_score.r > 0.0) ? retained_value.r : current.r,
        (retained_score.g > 0.0) ? retained_value.g : current.g,
        (retained_score.b > 0.0) ? retained_value.b : current.b);
#endif

#if PSPMNCA_PROFILE_GOVERNED_TOP2
    vec3 retained_score = best_score + second_score;
    vec3 retained_value =
        (best_value * best_score + second_value * second_score)
        / max(retained_score, vec3(0.000001));
    vec3 authority = clamp(retained_score, vec3(0.0), vec3(1.0));

    res_c.rgb = mix(current, retained_value, authority);
#endif

#endif // !PSPMNCA_PROFILE_DENSE_PAIR_SUM && !PSPMNCA_PROFILE_SINGLE_CHANNEL

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
