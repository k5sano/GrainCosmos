# GrainCosmos - Creative Brief

## Overview

**Type:** Audio Effect (Multi-Effects Processor)
**Core Concept:** Granular multi-effects processor inspired by Hologram Electronics Microcosm that reinterprets incoming audio using granular sampling, micro-looping, pitch-shifting, delay, and glitch techniques
**Status:** 💡 Ideated
**Created:** 2026-02-12

## Vision

GrainCosmos is a sophisticated multi-effects processor that transforms incoming audio through 11 distinct effect algorithms organized into 4 categories. From lush ambient washes to rhythmic glitch patterns and unexpected sonic transformations, GrainCosmos offers a universe of creative sound design possibilities.

The plugin captures the essence of the Hologram Electronics Microcosm hardware, translating its tactile immediacy and sonic palette into a digital format with expanded flexibility. At its heart is a 60-second phrase looper that can be positioned either before or after the effects chain, coupled with MIDI clock sync for rhythmic precision.

The 11 effects represent a comprehensive toolkit:
- **Micro Loop** effects create overlapping loops that play at different speeds (Mosaic), rearrange samples into new rhythmic sequences (Seq), and shift pitch over time (Glide)
- **Granules** generate clusters of micro-loops for ambient washes (Haze), cyclical drones (Tunnel), and pointillistic textures (Strum)
- **Glitch** effects introduce predictable bursts (Blocks), pitch-shifted interruptions (Interrupt), and arpeggiated sequences (Arp)
- **Multi Delay** offers rhythmic tap patterns (Pattern) and manipulated delays with filtering and pitch-shifting (Warp)

The 13-parameter interface provides immediate control over the essential aspects of each effect while maintaining simplicity. Four variations per effect (44 total variations) extend the sonic palette further, each variation subtly shifting the effect's behavior for enhanced creativity.

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| effectSelect | Choice (11 options) | Mosaic | Selects which of the 11 effects is active |
| variation | Choice (4 options) | Var 1 | Selects variation of the current effect (4 per effect = 44 total) |
| activity | 0.0 - 1.0 | 0.5 | Main effect control - intensity/depth of the selected effect |
| shape | 0.0 - 1.0 | 0.5 | Waveform shape and/or modulation frequency |
| filter | 0.0 - 1.0 | 0.75 | Resonant lowpass filter cutoff (20Hz - 20kHz mapped) |
| resonance | 0.0 - 1.0 | 0.25 | Filter resonance/Q (0 - 10 mapped) |
| mix | 0.0 - 1.0 | 0.5 | Dry/wet mix balance |
| time | 0.0 - 1.0 | 0.5 | Delay time or timing-related parameter |
| repeats | 0.0 - 1.0 | 0.3 | Feedback/repeats amount |
| modDepth | 0.0 - 1.0 | 0.0 | Modulation depth for LFO/envelope |
| space | 0.0 - 1.0 | 0.25 | Reverb/wetness amount |
| spaceDecay | 0.0 - 1.0 | 0.5 | Reverb decay time |
| reverse | Bool (toggle) | false | Reverse playback direction |
| hold | Bool (toggle) | false | Freeze/hold current buffer |

## Effect Categories

### Micro Loop
- **Mosaic:** Overlapping loops play back at different speeds, creating layered textures
- **Seq:** Looping samples rearranged into new rhythmic sequences
- **Glide:** Short overlapping loops shift in pitch over time

### Granules
- **Haze:** Clusters of micro-loops create immediate ambient washes
- **Tunnel:** Cyclical micro-loops generate hypnotic, evolving drones
- **Strum:** Rhythmic chains of note onsets create pointillistic, percussive textures

### Glitch
- **Blocks:** Predictable glitches and bursts of notes
- **Interrupt:** Pitch-shifted bursts interrupt the signal
- **Arp:** Sequences of samples create arpeggiated patterns

### Multi Delay
- **Pattern:** 4 delay taps arranged in rhythmic patterns
- **Warp:** Delay taps manipulated with filters and pitch shifting

## UI Concept

**Layout:**
- Vintage hardware aesthetic with aged/relic appearance
- Plugin window: 800x500 pixels (fixed)
- Two-row horizontal layout with header and footer
- 10 controls total (8 knobs + 2 toggles)

**Top Row (y: 80):**
- DELAY (delay_time): 0-2000ms
- GRAIN (grain_size): 5-500ms
- ENVELOPE (envelope_shape): 0.0-1.0 - NEW: controls delay grain envelope shape
- DISTORTION (distortion_amount): 0-100% - NEW: waveshaping/distortion

**Bottom Row (y: 240):**
- FEEDBACK (feedback): 0-95%
- CHAOS (chaos): 0-100%
- CHARACTER (character): 0-100% with purple glow effect (focal point)
- MIX (mix): 0-100%

**Footer (y: 410):**
- FREEZE (freeze): Toggle - NEW: freeze/hold current buffer
- SYNC (tempo_sync): Toggle

**Visual Style:**
- **Chassis:** Olive-gray vintage enclosure (#4a4a3a) with aging effects
- **Aging Effects:** Scratch texture, rust patches, worn paint, chipped edges, grease smudges
- **Knobs:** Retro silver measurement instrument style
  - Tapered conical shape (15-20 degree angle)
  - Knurled/gripping texture (45-degree cross-hatch)
  - Chrome/metallic silver finish (#c0c0c0 to #e8e8e8)
  - Chrome white highlights (#ffffff)
  - Dark gray shadows (#404040)
- **Indicators:** Purple gradient (#9b4dca to #b47eff) on silver surface
- **Text:** Gold color (#ffd89b) with wide letter-spacing
- **Logo/Title:** Japanese brush pen / handwritten marker font (organic, artistic)

**Key Elements:**
- Character knob has soft purple glow for focal differentiation
- Toggle switches use slide-style design
- Skeuomorphic 3D depth with dramatic shadows
- All controls spaced with 70px horizontal gaps
- Generous edge margins (~60px)

## Additional Features

**Phrase Looper:**
- 60-second maximum loop length
- Transport controls: Rec, Overdub, Undo, Play, Stop
- Pre-FX / Post-FX routing toggle
- Visual feedback for recording/playback state

**MIDI Integration:**
- MIDI clock sync for tempo-synced effects
- Parameter automation support
- Preset navigation via MIDI CC

## Use Cases

- **Sound Design:** Creating evolving textures and beds for film/game music
- **Ambient Music:** Generating lush, layered soundscapes from simple inputs
- **Electronic Production:** Adding glitch and rhythmic interest to static loops
- **Live Performance:** Real-time transformation of instruments or vocals
- **Sample Manipulation:** Turning static samples into evolving, organic textures

## Inspirations

- **Hardware:** Hologram Electronics Microcosm
- **Techniques:** Granular synthesis, tape loops, modular glitch workflows
- **Sonic References:** Ambient washes (Brian Eno), glitch textures (Aphex Twin), microsound genre

## Technical Notes

**DSP Considerations:**
- Granular synthesis requires efficient buffer management
- Real-time pitch-shifting without artifacts (phase vocoder or similar)
- Smooth crossfading between loop segments
- Filter implementation should be resonant lowpass with modulatable cutoff
- Multi-tap delay requires individual tap control
- Reverb could be algorithmic (plate/room) for adjustable decay

**Performance:**
- 60-second looper at 48kHz/96kHz requires significant RAM
- Granular effects need careful CPU optimization
- Consider downsampling options for granular buffer
- Smooth parameter transitions to prevent clicks/pops

**Architecture:**
- Effect-based architecture with 11 distinct DSP algorithms
- Shared parameter pool with effect-specific mapping
- Looper can be positioned pre or post effects
- Wet/dry mix for each effect plus global mix

## Next Steps

- [ ] Create UI mockup (`/dream GrainCosmos` → option 3)
- [ ] Create parameter specification for parallel workflow
- [ ] Start implementation (`/implement GrainCosmos`)
- [ ] Research granular DSP algorithms in Stage 0 planning
