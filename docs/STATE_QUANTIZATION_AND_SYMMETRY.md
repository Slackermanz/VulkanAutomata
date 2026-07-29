# State Quantization and Symmetry Preservation

This document explains a design principle used throughout VulkanAutomata shaders:

```text
floating-point arithmetic may be used inside a frame,
but floating-point error must not be allowed to accumulate as hidden state.
```

The simulation should preserve exact spatial symmetries for as long as the rule and input state preserve them. If two symmetric cells see the same symmetric neighbourhood data, they should continue to produce the same result. Tiny floating-point drift should not become a hidden source of spontaneous symmetry breaking.

This is not about making the shader academically pure. It is about keeping the cellular automata honest.

## The basic problem

A cellular automaton runs the same update rule over and over.

If the state is stored as unrestricted floating-point values, tiny rounding differences can become part of the next frame. After enough frames, those tiny differences may grow into visible differences. That means the simulation can start producing behaviour that came from arithmetic accident rather than from the rule.

That is especially dangerous in systems that are supposed to preserve symmetry.

Example:

```text
left side and right side begin perfectly mirrored
both sides should evolve the same way

but a tiny rounding difference appears
that difference gets stored
next frame sees that difference as real state
many frames later the mirror symmetry is gone
```

VulkanAutomata tries to avoid that.

## The state format is part of the rule

The work images are created as:

```text
VK_FORMAT_R16G16B16A16_UNORM
```

That means each channel is stored as a normalized 16-bit value, not as arbitrary 32-bit float state.

Inside the shader, samples are received as floats because shaders operate on floats. But when the frame is written back to the work image, the value is stored again in the 16-bit normalized state format.

This gives the simulation a bounded, finite state substrate:

```text
frame N:
    read finite stored state as floats
    compute next state
    write finite stored state again
```

That write-back step prevents hidden sub-quantum floating-point detail from surviving indefinitely.

## Quantize before accumulating neighbourhoods

The shader does not simply sum raw sampled floats in the ring calculation. It does this:

```glsl
const float psn = 32768.0;

val += floor(gdv(offset, txdata) * psn);
tot += psn;
```

That is deliberate.

Each sampled channel is converted into an integer-like bin before it is accumulated. Instead of adding many slightly different floating approximations of normalized texture values, the shader adds quantized values.

Conceptually:

```text
raw sample:
    maybe 0.42857362...

quantized sample:
    floor(raw sample * 32768)
    an integer-like value
```

The ring sum is then:

```text
sum of quantized sample values
divided by
sum of quantized sample weights
```

This makes neighbourhood measurement stable. The ring average is based on explicit quantized bins, not on a long chain of raw floating-point additions.

## Why `32768`

`32768` is `2^15`.

That matters because powers of two are exactly representable in binary floating point. This makes the quantization grid friendly to the GPU's float arithmetic.

The state image is 16-bit normalized, but the ring accumulation uses a 15-bit precision scale. That is a safety margin. It gives up a little precision so that sums stay comfortably in the exact integer range of 32-bit floats during local neighbourhood accumulation.

The goal is not maximum numeric precision. The goal is stable, symmetric, reproducible behaviour.

## Exact-enough accumulation

32-bit floats can exactly represent all integers up to `16,777,216`.

The ring sums in this shader stay far below that limit. Because each contribution has been floored into an integer-like value, adding those contributions is stable and exact for the intended neighbourhood sizes.

That means this kind of expression is safe:

```glsl
val += floor(sample * 32768.0);
```

for the ring sizes used here.

After all samples are accumulated, the shader divides:

```glsl
average = val / total;
```

That division produces a float again, but it is a float derived from a stable quantized sum. It is not a float carrying arbitrary microscopic history from previous frames.

## Toroidal wrapping and symmetry

The texture read helper wraps coordinates like this:

```glsl
of[0] = (of[0] + textureSize(tx,0)[0]) & (textureSize(tx,0)[0]-1);
of[1] = (of[1] + textureSize(tx,0)[1]) & (textureSize(tx,0)[1]-1);
```

This assumes power-of-two dimensions. Under that assumption, the bitwise `&` is a fast toroidal wrap.

The topology is therefore:

```text
left edge connects to right edge
top edge connects to bottom edge
```

This matters for symmetry because every cell sees the same wrapped world structure. There are no special boundary cells with different rules.

## Symmetric cells must do the same work

Symmetry preservation depends on more than quantization. It also depends on symmetric cells doing the same sequence of operations.

The shader is written so that each cell runs the same rule program:

```text
same ring construction
same pair count
same pair order
same scoring logic
same top-two retention logic
```

If two cells see the same neighbourhood values, they execute the same operations on the same values and should produce the same output.

Tie-breaking is deterministic. For example, the selector uses a fixed pair order. If two candidates have equal score, the earlier retained candidate remains preferred because the comparison uses strict greater-than checks. This may bias candidate order, but it does not create spatial symmetry breaking because every cell uses the same order.

## Empty masks are silent, not NaN

A neighbourhood mask may select no rings.

That is a valid genotype. It means the neighbourhood head is silent.

However, division by zero would create `NaN`, and `NaN` is poison for symmetry. Once `NaN` enters state, comparisons and arithmetic can become unpredictable.

So `bitmake` must treat empty neighbourhoods as neutral:

```glsl
return (tot > 0.0) ? sum / tot : vec4(0.0);
```

This preserves silent genotypes without allowing non-finite arithmetic.

Silence is useful. `NaN` is not.

## The energy tax uses a binary-friendly unit

The active pair-selective shader defines:

```glsl
const float ENERGY_UNIT = 1.0 / 65536.0;
const float ENERGY_TAX = 16.0 * ENERGY_UNIT;
```

This is not an arbitrary decimal constant.

`1 / 65536` is `1 / 2^16`, which is exactly representable in binary floating point. The tax is therefore defined in a clean binary unit:

```text
ENERGY_TAX = 16 / 65536 = 1 / 4096
```

The tax is applied to the captured current cell state:

```glsl
vec4 res_c = gdv(ivec2(0, 0), txdata);
res_c.rgb = max(res_c.rgb - vec3(ENERGY_TAX), vec3(0.0));
```

It is intentionally not applied inside `gdv()`.

`gdv()` remains a pure texture fetch and wrap helper. The tax is part of the rule pipeline, not part of the ancient sampling primitive.

## Why the tax is before assessment

The tax is applied after the current cell is read and before that cell value becomes the baseline for candidate proposals.

That means:

```text
read current state
pay energy tax
then evaluate rule candidates
```

This gives empty space a stable basin. If a cell cannot regenerate its colour through selected pair proposals, it fades toward zero.

This is different from subtracting at the very end as a visual cleanup. The tax changes the inherited state before the rule evaluates it. It is part of the cellular rule, not a post-processing blur or repair.

## What this does not guarantee

This discipline preserves symmetry under the intended VulkanAutomata contract:

```text
same shader
same state format
same dimensions
same rule parameters
same driver behaviour for the same operations
same symmetric input
```

It does not claim that every GPU driver, compiler, or future rewrite will produce bit-identical output across all machines forever.

It also does not claim that random reseeding commands will create symmetric state unless the reseed operation itself is symmetric. If the input state or command intentionally breaks symmetry, the simulation will evolve that asymmetry.

The goal is narrower and more important:

```text
do not let hidden floating-point drift become a new source of state.
```

## Things that would violate the contract

The following changes would weaken or break the quantization/symmetry discipline:

### Accumulating raw texture samples

Bad:

```glsl
sum += gdv(offset, txdata);
```

Better:

```glsl
sum += floor(gdv(offset, txdata) * psn);
```

Raw sample accumulation can preserve tiny representation differences through large sums.

### Using arbitrary decimal constants for persistent state changes

Risky:

```glsl
value -= 0.00025;
```

Better:

```glsl
value -= 16.0 / 65536.0;
```

Binary-friendly constants are easier to reason about.

### Letting empty neighbourhoods divide by zero

Bad:

```glsl
return sum / tot;
```

when `tot` may be zero.

Better:

```glsl
return (tot > 0.0) ? sum / tot : vec4(0.0);
```

### Moving the tax into `gdv()`

Do not silently change `gdv()` to return taxed samples. That would alter all neighbourhood perception and tamper with the low-level sampling primitive.

If a future shader wants taxed neighbourhood perception, it should use a separate helper with an explicit name.

### Adding a blur to fix selector artifacts

If a selector produces flicker or saturation, the selector or candidate proposal is wrong.

A convolution blur can hide bad output, but it also smuggles in another neighbourhood rule and dilutes the authority of the primary MNCA pairs.

### Storing work state as unrestricted float without a new discipline

Switching the work image to a floating-point format may be useful for some future experiment, but it removes the automatic finite-state reset provided by R16 UNORM storage.

If that happens, the shader needs a new explicit quantization boundary.

## Design rule of thumb

For every value that can become persistent state, ask:

```text
Will tiny floating-point detail survive into the next frame?
```

If yes, add an intentional quantization boundary.

For every symmetry-preserving operation, ask:

```text
Will symmetric cells execute the same operations on the same quantized values?
```

If yes, symmetry should persist.

For every exception, ask:

```text
Is this an intentional rule feature, or accidental arithmetic leakage?
```

Only intentional rule features belong in the automaton.
