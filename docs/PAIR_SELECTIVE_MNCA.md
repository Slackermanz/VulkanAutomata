# Pair-Selective Perceptron MNCA

This document explains the active experimental rule architecture in `res/frag/frag_automata0000.frag`.

It is written for people who can read shader code or simple game/graphics code, not for people already fluent in academic cellular automata notation. The goal is to teach the idea well enough that somebody could reimplement it without copying every line of this shader.

This is not a literature survey and not a final paper. It is a working technical description of a mechanism in VulkanAutomata.

## The short version

Traditional or public-facing Multiple Neighborhood Cellular Automata usually look like this:

```text
for each cell:
    measure several neighborhoods
    apply rules to those measurements
    combine the rule effects
    write the next cell value
```

The pair-selective shader does something different:

```text
for each cell:
    create several possible next-state proposals
    each proposal comes from a pair of neighborhoods
    score those proposals
    keep only the top few
    discard the rest
    let the retained proposals update the cell
```

The important word is **discard**.

Most of the work the shader computes is intentionally thrown away. That is not waste in the artistic or dynamical sense. The discard is the selection mechanism. It is what lets different local behaviours compete instead of all being averaged into one mushy result.

## Why the MNCA pair is the atom

A single neighborhood can describe local density, texture, or colour around a cell.

A pair of neighborhoods can describe a relationship between scales:

```text
small neighborhood versus larger neighborhood
near field versus far field
inner structure versus outer structure
one colour relationship at one scale versus another colour relationship at another scale
```

That scale relationship is the minimum useful unit for this family of behaviour.

So in this shader, the basic unit is not a single neighborhood and not a single perceptron pathway. The basic unit is an **MNCA pair**:

```text
MNCA pair =
    neighborhood A
    neighborhood B
    transform for A
    transform for B
    pair transform that combines A and B
    one candidate next-state proposal
```

The active shader has six such pairs.

## What problem this solves

If every rule contributes every frame, the output tends to become a dense average of everything the rule system can say. That can make good patterns, but it can also make the system brittle or overly blended.

The old SMNCA idea was different: compute several speculative futures, then select from them.

This shader restores that idea in a perceptron-style form:

```text
many candidate pair proposals are computed
only the strongest few are allowed to express
everything else is silent for that cell and frame
```

That creates a local mixture-of-experts effect, but the experts are not neural-network blocks in the usual sense. They are two-neighborhood cellular automata pair proposals.

## Data layout

The shader uses 48 pattern words per panel from `sb.p[v_idx].v[0..47]`.

Each bit or bitfield should have one direct meaning. A bit should not be reused as two unrelated controls.

Current active layout:

```text
v0..v2
    12 neighborhood masks
    6 pairs × 2 masks per pair
    8 bits per mask

v3..v38
    coefficient magnitudes
    6 pairs × 36 coefficients per pair
    5 bits per coefficient magnitude
    6 coefficient fields per word

v39..v45
    coefficient signs
    6 pairs × 36 sign bits per pair
    one sign bit per coefficient

v46..v47
    scoring weights for `WEIGHTED_TOP1`
    12 signed 5-bit weights: four features × RGB channels
    reserved by the non-weighted profiles
```

The shader also reads runtime/control values from the uniform buffer, such as scale, zoom, mapping mode, mouse command, and frame seed.

## One pair in detail

For pair `p`, there are two heads:

```text
head A = 2p
head B = 2p + 1
```

Each head has an 8-bit neighborhood mask. Each bit includes or excludes one ring radius from the neighborhood measurement.

An empty mask is allowed. It means the head has no neighborhood evidence. This is a silent genotype, not an error.

The pair then does this:

```text
nA = average selected rings for head A
nB = average selected rings for head B

latentA = dot-product transform of nA
latentB = dot-product transform of nB

predictionA = pair terminal contribution from latentA
predictionB = pair terminal contribution from latentB

proposal = predictionA + predictionB
score = policy-specific judgment over the candidate facts
```

The shader keeps those facts addressable while scoring:

```text
current
nA
nB
latentA
latentB
predictionA
predictionB
proposal
```

That matters because scoring should not be forced to judge only the already-combined proposal. Some experiments may ask whether the two sides agree, whether they oppose, whether one side dominates, whether the proposal conforms to either raw neighbourhood, or whether the pair should win or lose as a whole.

The dot products are just weighted mixes. If you have three input colour channels, a dot product says:

```text
output = red * weightR + green * weightG + blue * weightB
```

The shader uses those dot products to let each neighborhood become an internal interpretation, then combines the two interpretations into one pair proposal.

## Why dot products are still the perceptron part

The shader is still perceptron-inspired because it uses direct weighted channel mixing:

```text
RGB input -> weighted transform -> RGB latent value
```

The difference from the earlier three-stage baseline is that the transformed values are now organized around pairs.

The old baseline effectively said:

```text
eight heads produce outputs
sum all outputs
```

The pair-selective shader says:

```text
six pairs produce candidate proposals
select only the strongest few proposals
```

That is the essential architectural shift.

Because pair evaluation is upstream of selection, the same pipeline can also support a nonselective control mode:

```text
six pairs produce candidate proposals
sum all proposals
write the dense pair-sum result
```

That control would not be bit-identical to the older three-layer perceptron baseline, but it would be the correct nonselective baseline for the current pair-native architecture. It lets future experiments ask whether a behaviour belongs to candidate construction or to sparse selection.

## Candidate proposal

A pair terminal transform produces two side predictions:

```text
predictionA = output-space contribution from neighbourhood side A
predictionB = output-space contribution from neighbourhood side B
```

The raw pair proposal is:

```text
proposal = predictionA + predictionB
```

Most experimental profiles currently treat this proposal as a replacement candidate: if the pair wins, the proposal can become the next cell value for the selected channel.

Some profiles deliberately reinterpret the proposal. For example, the governed profile turns the raw proposal into a bounded movement from the current state:

```text
delta = proposal / (1 + abs(proposal))

if delta is positive:
    move toward 1.0 using available headroom

if delta is negative:
    move toward 0.0 using available room above zero
```

That governed version is useful, but it is not the fundamental pair candidate. The fundamental candidate facts are the two side predictions and their raw summed proposal. Scoring, retention, and expression decide how those facts are used.

## Scoring

The score asks whether a pair is expressing a useful relationship.

The active shader considers:

```text
support
    both neighborhood masks are non-empty

relation
    latentA and latentB differ enough to matter

relation band
    the difference is not too tiny and not violently huge

saturation safety
    the proposal is not just slamming straight into 0 or 1
```

In plain language:

```text
a pair scores well when both sides have evidence,
the two sides disagree in an interesting way,
and the resulting proposal still leaves room for graceful dynamics.
```

The score is per colour channel. Red, green, and blue can choose different winning pair proposals.

## Selection

For each colour channel, the shader keeps only the top two scoring proposals.

Everything else is discarded.

The active logic is:

```text
for each pair:
    compute proposal
    compute score

    if score is best for this channel:
        old best becomes second best
        this proposal becomes best

    else if score is second best:
        this proposal becomes second best
```

After all six pairs are evaluated:

```text
retained_value =
    weighted average of best and second-best retained proposals

authority =
    total retained score

next colour =
    mix(current, retained_value, authority)
```

Only the retained proposals can affect the cell. Non-winners do not leak into the output.

This is the main difference from dense aggregation.

## Energy tax

The shader applies a small global tax to the captured current state:

```text
ENERGY_TAX = 16 / 65536
```

This is applied after reading the current cell and before the current value is used by the rule pipeline.

It is intentionally not inside `gdv()`.

`gdv()` is the ancient stable fetch/wrap primitive. It remains untouched.

The tax means:

```text
existing colour has a small cost
if local dynamics cannot regenerate it, it fades toward empty space
```

This creates a stable empty-space basin without adding a convolution blur, neighbour-count support rule, or Game-of-Life-like survival kernel.

## Why not use a blur or convolution fix

Older experiments sometimes used convolution or blending after selection to smooth rough edges.

That can work visually, but it hides a problem: if selection produces malformed output and a blur fixes it later, the rule is depending on a post-processing repair.

This shader tries to avoid that.

The selector itself should produce graceful state proposals. The tax is not a blur. It does not ask whether nearby cells support this cell. It only says the cell state pays a small cost before it participates in the next update.

## How this differs from common MNCA implementations

A baseline/public MNCA implementation usually has:

```text
multiple neighborhoods
rules based on neighborhood values
all applicable rules contribute to the output
```

This shader has:

```text
multiple pairs of neighborhoods
each pair creates one candidate transition
only the top candidate transitions are retained
most candidates are discarded
```

The difference is not just implementation style. It changes the kind of behaviour the system can express.

Dense aggregation asks:

```text
what is the combined effect of all rules?
```

Sparse pair selection asks:

```text
which local scale-pair explanations deserve to become real here?
```

That competition is what produces the selection-driven, metastable behaviour.

## Important constraints

These constraints are part of the design and should be preserved unless a future experiment intentionally breaks them.

### One bit, one direct meaning

A raw genotype bit should not control two unrelated behaviours.

Good:

```text
this bit includes ring 5 in pair 2 head A
```

Bad:

```text
this same bit includes ring 5 and also changes a selector mode
```

A bit can have downstream effects through the value it helps compute. That is normal. What should be avoided is unrelated aliasing of raw bit meaning.

### Silent genotypes are valid

An empty neighborhood mask, a cancelling matrix, or a pair with no score is not broken.

Silence is useful. Mutation can later turn silent structure into expressed structure.

The only thing that must be prevented is non-finite arithmetic such as division by zero producing NaN.

### Selection must be sparse

If every candidate gets to influence the output, the hallmark behaviour is lost.

The selector should retain only a small number of proposals and discard the rest.

### Magnitude alone is not authority

A candidate should not win just because it is huge.

Huge raw output tends to create saturation, flicker, and brittle extremes. The score should reward useful relation and safe expression, not raw force.

### Do not hide bad selection with post-processing

Avoid fixing the selector by adding a blur or convolution cleanup after the fact.

If selection causes flicker or saturation, the candidate construction or score is wrong.

## What to copy if reimplementing

A minimal reimplementation needs these parts:

1. A toroidal texture/grid.
2. A ring or neighborhood bank.
3. A way to choose two neighborhoods per pair.
4. A pair transform that turns those two neighborhoods into one proposal.
5. A score for each proposal.
6. A top-k selector that keeps only a small number of proposals.
7. A fallback toward the current state when authority is weak.
8. A small energy tax if empty space should remain stable.

Pseudocode:

```text
current = read_current_cell()
current = max(current - energy_tax, 0)

rings = measure_neighborhood_rings()

best = current
second = current
best_score = 0
second_score = 0

for each pair:
    nA = selected_rings(maskA)
    nB = selected_rings(maskB)

    latentA = weighted_mix_A(nA)
    latentB = weighted_mix_B(nB)

    impulse = weighted_mix_pair(latentA, latentB)
    proposal = bounded_move_from_current(current, impulse)

    score = proposal_fitness(latentA, latentB, proposal)

    keep proposal if it is in the top two scores

retained = score_weighted_mix(best, second)
authority = best_score + second_score

next = mix(current, retained, authority)
```

That is the core idea.

## Current active constants

As of this draft, the active shader uses:

```text
MAX_RADIUS  = 8
PAIR_COUNT  = 6
PAIR_STEP   = 0.125
ENERGY_TAX  = 16 / 65536
```

These are empirical working values, not laws of nature.

## Open questions

The current shader forces alpha to `1.0`.

Alpha may be usable later as hidden state: confidence, prior authority, refractory charge, memory, or energy. Before doing that, the render/export/presentation pipeline needs to be checked carefully so alpha does not unexpectedly affect display or file output.

The shader now exposes compile-time profiles near the top of `frag_automata0000.frag`:

```glsl
#define PSPMNCA_PROFILE_DENSE_PAIR_SUM 1
#define PSPMNCA_PROFILE_SINGLE_CHANNEL 0
#define PSPMNCA_PROFILE_GENTLE_TOP1 0
#define PSPMNCA_PROFILE_WEIGHTED_TOP1 0
#define PSPMNCA_PROFILE_RAW_TOP1_REPLACE 0
#define PSPMNCA_PROFILE_RAW_TOP1 0
#define PSPMNCA_PROFILE_RAW_TOP2 0
#define PSPMNCA_PROFILE_GOVERNED_TOP2 0
```

Exactly one profile should be enabled before compiling.

`DENSE_PAIR_SUM` is the nonselective baseline. All six pair proposals are summed directly with no scoring, selection, or discarding. This is the pair-native equivalent of the old depth-2 perceptron baseline. It shares the same upstream pair evaluation pipeline; the selector is simply bypassed. Use this to confirm whether a behaviour belongs to candidate construction or to sparse selection.

`SINGLE_CHANNEL` is a degenerate test mode. Only the red channel is used for simulation; green and blue mirror red for display. It is controlled by `SC_NEIGHBORHOOD_COUNT` and `SC_ACTIVATIONS_PER_NEIGHBORHOOD`.

The scalar profile has seven independent compile-time feature switches. Any combination is valid as long as the total genotype budget fits within 48 words:

```text
SC_USE_SIGN         apply sign bits (±1) per contribution, or always positive
SC_USE_WEIGHT       apply 5-bit magnitude weights, or use 1.0
SC_USE_STEP         apply range-gated activations, or always fire
SC_STEP_MULTIPLY_BY_NEIGHBORHOOD   fired update is nh-scaled or constant
SC_FIXED_STEP_CONTRIB              cap nh-scaled updates to a binary-friendly microstep
SC_USE_SIZE_PRIORITY               scale each contribution by inverse selected-neighbourhood pixel count
SC_USE_SCALE_LADDER                bypass contribution summing and use a scale-ladder interval rule
SC_SCALE_LADDER_USE_MUTATION_MASKS use mutation-owned scalar masks as ladder observations instead of fixed cumulative masks
SC_SCALE_LADDER_PREBLEND_CURRENT   blend each ladder observation with current before interval comparison
SC_SCALE_SELECT_MIN_ABS_INTERVAL   select the adjacent scale interval with smallest absolute difference
SC_SCALE_SELECT_MAX_ABS_INTERVAL   select the adjacent scale interval with largest absolute difference
SC_SCALE_APPLY_SIGNED_STEP         apply sign(selected interval difference) as a fixed step
SC_SCALE_APPLY_SELECTED_BIAS       add a small bias from the selected ladder observation
SC_BASELINE_PASSTHROUGH             start from current state or start from zero
```

When `SC_USE_STEP` is `0`, each neighbourhood contributes once. Each contribution is the ring average, optionally multiplied by a weight and/or sign. When `SC_USE_STEP` is `1`, each neighbourhood contributes `SC_ACTIVATIONS_PER_NEIGHBORHOOD` times, each gated by a 5-bit lower / 5-bit upper threshold range:

```text
if lower <= ring_average <= upper:
    contribution fires
```

This is implemented with branchless hard `step()` gates, not a smooth sigmoid. If `lower > upper`, the activation cannot fire; this is a valid silent genotype.

The genotype layout chains only the enabled fields:

```text
masks always first
then range windows (if SC_USE_STEP)
then magnitudes (if SC_USE_WEIGHT)
then signs (if SC_USE_SIGN)
```

The budget only counts enabled fields. Disabled features consume zero words.

When `SC_USE_SIZE_PRIORITY` is `1`, each contribution is multiplied by an inverse selected-neighbourhood size factor. The factor is derived from the same accumulated sample count used by `bitmake`, so a mask selecting only a tiny ring keeps full priority while a mask selecting many sampled pixels is weakened. This is explicit scalar-test governance for checking whether small kernels need stronger local authority than broad kernels; it does not change how rings are measured.

When `SC_USE_SCALE_LADDER` is `1`, the scalar profile bypasses the contribution-summing branch and evaluates a sequence of ladder observations. With `SC_SCALE_LADDER_USE_MUTATION_MASKS` set to `1`, those observations are the first `SC_SCALE_LADDER_COUNT` mutation-owned scalar masks in declaration order. This hands neighbourhood geometry back to the genotype, but the sequence is not guaranteed to be a true ordered scale ladder. With `SC_SCALE_LADDER_USE_MUTATION_MASKS` set to `0`, the observations are fixed cumulative masks over the current ring bank; that is closer to the old `ScaleVariance.frag` shape but is mutation-invariant.

The scale-ladder controls are deliberately separate. `SC_SCALE_LADDER_PREBLEND_CURRENT` decides whether each observation is smoothed against the current cell before interval comparison. `SC_SCALE_SELECT_MIN_ABS_INTERVAL` and `SC_SCALE_SELECT_MAX_ABS_INTERVAL` choose which adjacent ladder interval is selected. `SC_SCALE_APPLY_SIGNED_STEP` applies a fixed step in the sign direction of the selected interval. `SC_SCALE_APPLY_SELECTED_BIAS` adds a small contribution from the selected observation.

The old `ScaleVariance.frag` behaviour is approximated by this preset:

```text
SC_USE_SCALE_LADDER 1
SC_SCALE_LADDER_USE_MUTATION_MASKS 0
SC_SCALE_LADDER_PREBLEND_CURRENT 1
SC_SCALE_SELECT_MIN_ABS_INTERVAL 1
SC_SCALE_SELECT_MAX_ABS_INTERVAL 0
SC_SCALE_APPLY_SIGNED_STEP 1
SC_SCALE_APPLY_SELECTED_BIAS 1
```

Turning individual flags off lets the ladder itself, preblend, interval selection, signed step, and selected-scale bias be tested independently instead of treating ScaleVariance as a forced-function branch.

Examples:

```text
SC_NEIGHBORHOOD_COUNT 8, sign only, no weight, no step, zero baseline
    8 neighbourhoods, ±1 raw ring average, start from zero
    simplest nontrivial scalar MNCA

SC_NEIGHBORHOOD_COUNT 8, sign+weight, no step, zero baseline
    8 neighbourhoods, signed weighted ring averages

SC_NEIGHBORHOOD_COUNT 8, sign+weight+step, 5 activations, zero baseline
    8 neighbourhoods, 5 range-gated update functions per neighbourhood
    closest to historical SMNCA scalar range rules

SC_NEIGHBORHOOD_COUNT 100, no sign, no weight, no step, passthrough baseline
    100 raw unsigned ring averages added to current state

SC_USE_SCALE_LADDER 1, SC_SCALE_LADDER_USE_MUTATION_MASKS 0, SC_SCALE_LADDER_COUNT 7, min-absolute interval, signed step, selected bias
    fixed ordered cumulative scale ladder, choose smallest adjacent scale interval, apply fixed signed step and bias

SC_USE_SCALE_LADDER 1, SC_SCALE_LADDER_USE_MUTATION_MASKS 1, SC_SCALE_LADDER_COUNT 7
    mutation-owned scalar masks used as the ladder observations in declaration order
```

This tests how each feature independently contributes to the expressiveness of scalar MNCA on the current VulkanAutomata ring/substrate.

`WEIGHTED_TOP1` uses a feature-vector scorer with weights read from the pattern genome. Candidate utility is compared against a genotype-owned no-op threshold:

```text
proposal = impulse
neighbourhood_error = min(abs(proposal - neighbourhoodA), abs(proposal - neighbourhoodB))

utility = wd * abs(proposal - current)
        + wp * proposal
        + wn * neighbourhood_error

score = utility - wb

retain only the best (maximum score) proposal per channel
winner expresses directly when score is above zero
fallback to current when no candidate beats the threshold
```

The four weight families are:

```text
wd  weight on deviation magnitude — positive rewards loud change, negative rewards gentle change
wp  weight on proposal value — positive prefers brighter candidates, negative prefers darker
wn  weight on neighbourhood conformance error — negative rewards matching either neighbourhood, positive rewards divergence
wb  no-op threshold — higher values make expression harder, lower values make expression easier
```

The weights live in `v46` and `v47`:

```text
v46 bits 0..29  weights 0..5
v47 bits 0..29  weights 6..11

weights 0..2    wd.rgb
weights 3..5    wp.rgb
weights 6..8    wn.rgb
weights 9..11   wb.rgb
```

Each weight is one signed 5-bit field:

```text
bits 0..3  magnitude, 0..15 mapped to 0.0..1.0
bit  4     sign, 0 = negative, 1 = positive
```

This uses 60 of the 64 clean spare bits in `v46/v47`. The high two bits of each word remain unused.

Changing `wp` away from zero can break the exchange symmetry that causes two-cycle thrashing: A preferring B no longer forces B to prefer A, because the proposal-value term scores differently for the forward and return transitions.

Changing `wn` changes whether the scorer prefers candidates that resemble one of their two neighbourhood observations. A negative `wn` rewards local conformance. A positive `wn` rewards anti-conformance or novelty. This term does not smooth or clamp the selected proposal; it only changes which proposal wins.

Changing `wb` changes the no-op competitor. A high threshold makes candidate expression rarer. A low or negative threshold makes expression easier. This replaces the earlier hardcoded zero threshold with a mutable scoring boundary.

Existing archive records already contain values in `v46/v47`, because the mutation machinery has always copied and mutated all 48 pattern words. Under `WEIGHTED_TOP1`, those previously latent bits now become active scoring policy. That is intentional for this experiment, but it means old patterns inherit whatever scorer genome they already had.

`GENTLE_TOP1` uses stateless replacement and minimum-deviation selection. Instead of selecting the candidate that changes the state the most, it selects the candidate that changes the state the least while still being non-zero.

`DENSE_PAIR_SUM` is the nonselective baseline. All six pair proposals are summed directly with no scoring, selection, or discarding. This is the pair-native equivalent of the old depth-2 perceptron baseline. It shares the same upstream pair evaluation pipeline; the selector is simply bypassed.

`RAW_TOP1_REPLACE` uses stateless replacement instead of accumulation. The winning pair's raw output becomes the next cell state directly. Empty neighbourhoods produce zero, zero is an exact fixed point, and no ratchet or integration can fill space over time.

`RAW_TOP1` uses accumulation (`proposal = current + impulse`). It restores motility but creates a ratchet: any persistent positive bias fills space over time because state is integrated rather than replaced.

`RAW_TOP2` keeps the raw proposal and raw deviation score, but retains and score-blends the two best proposals per channel. It tests whether top-two synthesis is useful without the baseline-sticky authority mix.

`GOVERNED_TOP2` is the first successful civilized profile. It uses the bounded headroom proposal, relation-band scoring, saturation safety, top-two synthesis, and authority-gated output described in the main sections above. It produces smoother and more stable behaviour, but it also narrows the expressive range.

The energy tax is controlled independently:

```glsl
#define PSPMNCA_USE_ENERGY_TAX 1
```

The weighted scorer currently has an experimental override:

```glsl
#define PSPMNCA_FORCE_CONFORMANCE_SCORER 1
```

When this is enabled, `WEIGHTED_TOP1` ignores the `v46/v47` scorer weights and uses one universal conformance scorer:

```text
neighbourhood_error = min(abs(proposal - neighbourhoodA), abs(proposal - neighbourhoodB))
score = 1 - neighbourhood_error
```

This makes the best candidate the one whose proposal is closest to either of its two neighbourhood observations. It is meant for inspection of this scoring law without genotype-owned scorer variation. Set it to `0` to return to the evolvable weighted scorer stored in `v46/v47`.

The tax is currently fixed. Future versions could expose it as a controlled parameter, but doing so should respect the one-bit-one-meaning genotype principle.
