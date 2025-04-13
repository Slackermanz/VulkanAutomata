
# MNCA Engine User Guide

## Introduction
The Multi-Neighborhood Cellular Automata (MNCA) Engine is a powerful simulation system that generates complex visual patterns based on cellular automata rules. This guide covers all input controls and their functions to help you explore and create fascinating emergent patterns.

## Core Controls

### Simulation Playback
| Control | Function | Details |
|---------|----------|---------|
| **SPACE** | Toggle pause/resume | Freezes or continues pattern evolution |
| **S** | Step simulation | Advances one frame when paused |
| **T** | Toggle frame throttling | Limits simulation speed for consistent viewing |

### Pattern Generation & Modification
| Control | Function | Details |
|---------|----------|---------|
| **X** | Clear simulation | Resets all cells to zero/black |
| **Z** | Symmetrical reseed | Creates a symmetrical initial pattern |
| **C** | Blend reseed [12 steps] | Creates a smoother transition over multiple steps |
| **LEFT SHIFT** (release) | Random reseed | Generates random initial conditions |
| **R** | Full randomization | Randomizes all parameters, resets scale/zoom |
| **V** | Mutate target pattern | Applies controlled mutations to current pattern |

### Pattern Navigation & Management
| Control | Function | Details |
|---------|----------|---------|
| **TAB** | Random pattern | Loads random pattern from archive |
| **RIGHT ARROW** | Next pattern | Loads next pattern in archive |
| **LEFT ARROW** | Previous pattern | Loads previous pattern in archive |
| **CTRL+S** | Save to archive | Saves current pattern to archive |
| **Q** | Reload target | Reverts to previous pattern state |

### Visualization Modes
| Control | Function | Details |
|---------|----------|---------|
| **1** | Planar mapping | Uniform parameters across entire field |
| **2** | Linear mapping | Parameters vary horizontally across field |
| **3** | Circular mapping | Parameters vary radially from center |
| **0** | Toggle data display | Shows technical data (may be disabled) |

### Recording & Export
| Control | Function | Details |
|---------|----------|---------|
| **NUMPAD ENTER** | Toggle recording | Starts/stops exporting frames to disk |
| **NUMPAD +** | Increase export frequency | Captures frames more often |
| **NUMPAD -** | Decrease export frequency | Captures frames less often |

### Interface Control
| Control | Function | Details |
|---------|----------|---------|
| **ESC** | Toggle GUI visibility | Shows/hides the interface (always visible when paused) |
| **F1** | Help | Displays help (may be unimplemented) |

## Mouse Controls

### Basic Interactions
| Control | Function | Details |
|---------|----------|---------|
| **Left Mouse Button** | Add values | Draws/adds intensity to pattern at cursor location |
| **Right Mouse Button** | Remove values | Erases/reduces intensity at cursor location |
| **SHIFT+Right Mouse+Drag** | Scale panning | Horizontal motion adjusts scale parameter |

### Pattern Management
| Control | Function | Details |
|---------|----------|---------|
| **Middle Mouse Button** | Update target | Sets current state as target pattern |
| **SHIFT+Middle Mouse** | Update & save | Sets as target pattern and saves to archive |
| **Mouse Back Button** | Previous pattern | Same as LEFT ARROW |
| **Mouse Forward Button** | Next pattern | Same as RIGHT ARROW |

### Zoom Control
| Control | Function | Details |
|---------|----------|---------|
| **Mouse Wheel Up** | Increase zoom | Increases zoom value (affects parameter distribution) |
| **Mouse Wheel Down** | Decrease zoom | Decreases zoom value |

## Understanding Key Concepts

### Scale vs. Zoom
- **Scale**: Fundamental parameter affecting neighborhood size and pattern behavior
- **Zoom**: Modifies how scale is interpreted in non-planar mapping modes
- Together they determine the density and distribution of pattern features

### Parameter Mapping Modes
1. **Planar (1)**: All cells follow identical rules
2. **Linear (2)**: Rules vary horizontally across the field
3. **Circular (3)**: Rules vary based on distance from center

### Target Patterns
- The "target pattern" is the base state for mutations
- Use Middle Mouse Button to capture interesting states as targets
- The mutation system allows controlled exploration of similar patterns

## Recording & Exporting
The MNCA Engine allows you to save interesting patterns and animations:

1. Use **NUMPAD ENTER** to toggle recording
2. Adjust frequency with **NUMPAD +/-**
3. Exported frames can be assembled into videos using external tools
4. Save individual patterns to the archive with **CTRL+S**

## Effective Techniques

### Pattern Discovery
- Use **TAB** to quickly explore diverse patterns
- Apply **R** for completely new starting points
- When you find something interesting, use **V** to explore variations

### Fine-Tuning
- **SHIFT+RMB+Drag** provides precise scale control
- Mouse wheel adjustments alter parameter distribution
- Combine **Z**, **C**, and manual drawing to seed specific patterns

### Creating Videos
- Set appropriate export frequency
- Use throttling for consistent frame timing
- Try different parameter mapping modes for varied effects

### Preserving Discoveries
- Use **SHIFT+MMB** to quickly save interesting patterns
- The archive preserves all parameters for later exploration
- Archive patterns can be navigated with arrow keys

## Advanced Controls

### Mutation Settings
Access via the GUI to control:
- Mutation strength (bit flip probability)
- Target history navigation
- Parameter randomization options

### Scale & Zoom Fine Control
- Hold **SHIFT+RMB** and drag horizontally
- The further from center you drag, the more dramatic the scaling effect
- Release SHIFT to apply the new scale and perform a random reseed

### Multi-Panel View
When available, shows multiple parameter variations:
- Click MMB on a panel to select that variation
- Useful for exploring parameter space systematically

---

This guide covers all inputs of the MNCA Engine. Experiment with different combinations of controls to discover the vast creative possibilities of this cellular automata system!