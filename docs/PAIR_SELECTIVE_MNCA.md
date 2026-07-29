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
    reserved
    currently not assigned a phenotype in this shader
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

impulse = pair terminal transform of latentA and latentB
proposal = bounded movement from the current cell state
score = how credible/useful this proposal is
```

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

## Candidate proposal

A pair terminal transform produces an `impulse`.

The impulse is not written directly as the next colour. It is first bounded:

```text
delta = impulse / (1 + abs(impulse))
```

That keeps the movement finite without simply clipping it.

The proposal then moves from the current cell state by a limited amount:

```text
if delta is positive:
    move toward 1.0 using available headroom

if delta is negative:
    move toward 0.0 using available room above zero
```

This is why the proposal is a state transition rather than a raw colour output.

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

The selector currently retains the top two proposals per channel. Top one, top two, and other sparse policies may produce different regimes.

The energy tax is currently fixed. Future versions could expose it as a controlled parameter, but doing so should respect the one-bit-one-meaning genotype principle.
