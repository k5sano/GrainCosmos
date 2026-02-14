# GrainCosmos Research Findings

**Research Date:** 2026-02-12
**Research Level:** Level 2 (Moderate Investigation - 30 min)
**Source:** Local codebase analysis + web research

---

## Executive Summary

Research completed on granular multi-effects plugins, focusing on:
1. Hologram Electronics Microcosm architecture
2. Existing granular plugins (Scatter, AngelGrain)
3. JUCE multi-effects architecture patterns
4. UI design patterns for complex effects processors

**Key Finding:** The Microcosm achieves 11 effects through algorithm variations within a unified granular/delay framework, not 11 separate DSP engines.

**Recommendation:** Start with simplified version (AngelGrain + envelope/distortion/freeze) rather than full 11-effect implementation.

---

## Local Codebase Analysis

### Scatter (Granular Delay with Scale Quantization)
- **Voices:** 64 polyphonic grain voices
- **Interpolation:** Lagrange3rd for smooth pitch shifting
- **Features:** Scale quantization (5 scales), pitch randomization, pan randomization, reverse playback
- **Parameters:** 9 total (delay_time, grain_size, density, pitch_random, scale, root_note, pan_random, feedback, mix)
- **CPU:** ~40-60% single core at 48kHz
- **UI:** Cream skeuomorphic, 90mm knobs, central particle field visualization

### AngelGrain (Simpler Granular Delay)
- **Voices:** 32 polyphonic grain voices (mono input → stereo output)
- **Features:** Unified "chaos" parameter, character morphing (glitchy ↔ smooth), pitch quantization to octaves/fifths only
- **Parameters:** 7 total (delay_time, grain_size, feedback, chaos, character, tempo_sync, mix)
- **CPU:** ~30-45% single core at 48kHz
- **UI:** Neomorphic design, celestial blue accent, 6 controls in 2-row layout
- **Key Innovation:** Single chaos parameter controls 4 dimensions (position, pitch, pan, timing)

---

## Microcosm Architecture Insights

The Hologram Electronics Microcosm uses **algorithm variations** within shared DSP framework:

**11 Effects in 4 Categories:**
1. **Micro Loop:** Mosaic (overlapping loops), Seq (rhythmic sequences), Glide (pitch-shifting)
2. **Granules:** Haze (ambient washes), Tunnel (cyclical drones), Strum (pointillistic)
3. **Glitch:** Blocks (predictable bursts), Interrupt (pitch-shifted), Arp (arpeggios)
4. **Multi Delay:** Pattern (4-tap rhythmic), Warp (filtered/pitch-shifted)

**Technical Approach:**
- Single delay buffer (phrase looper)
- Effect switching changes grain spawn/processing algorithms
- Shared components: window functions, pitch shifters, filters
- Variation system: 4 variations per effect (44 total)

---

## JUCE Architecture Best Practices

From web research ([JUCE tutorials](https://juce.com/tutorials/tutorial_audio_processor_graph/), [forum discussions](https://forum.juce.com/t/creating-a-chain-of-effects/10456)):

**Recommended: Single AudioProcessor with Effect Switching**
- Use one `AudioProcessor` class
- Switch DSP algorithms in `processBlock()` based on effect parameter
- Shared components: delay buffer, grain voices, window functions
- Different effects read from/process shared buffers differently

**Architecture Pattern:**
```cpp
class MultiEffectProcessor : public AudioProcessor {
    // Shared components
    juce::dsp::DelayLine<float, Lagrange3rd> delayBuffer;
    std::array<GrainVoice, 64> grainVoices;

    enum EffectType { EFFECT_A, EFFECT_B, EFFECT_C };
    EffectType currentEffect;

    void processBlock(AudioBuffer& buffer) override {
        switch (currentEffect) {
            case EFFECT_A: processEffectA(buffer); break;
            case EFFECT_B: processEffectB(buffer); break;
        }
    }
};
```

**NOT Recommended for This Use Case:**
- `juce::AudioProcessorGraph` (too complex for single plugin)
- Separate processor per effect (overhead, complexity)

---

## Granular DSP Core Components

From [JUCE forum discussions](https://forum.juce.com/t/advice-for-learning-about-granular-microsound/60359) and [academic papers](http://www.rossbencina.com/static/code/granular-synthesis/BencinaAudioAnecdotes310801.pdf):

**Essential Components:**
1. **Circular delay buffer** with Lagrange3rd interpolation
2. **Grain voice pool** (32-64 voices, pre-allocated)
3. **Grain scheduler** (sample-based timer)
4. **Window function** (Hann or Tukey) for anti-click
5. **Pitch shifting**: `rate = 2^(semitones/12)`

**Performance Optimization:**
- Pre-calculate window lookup tables
- Use `juce::FloatVectorOperations` for SIMD
- Profile CPU target: 40-60% single core

---

## UI Design Patterns

**Pattern 1: Effect Selector Strip + Shared Controls**
- Tab/button strip to select effect
- Controls update based on selected effect
- 9 main knobs with different mappings per effect

**Pattern 2: Variation Buttons**
- 4 variation buttons per effect
- Each variation presets parameter relationships

**Pattern 3: Centered Visualization**
- Central display shows effect activity
- Controls clustered around visualization

**Color Aesthetics:**
- Dark cosmic: Navy/purple (#0a0e1a to #1a1e3a)
- High contrast controls
- Glow effects for active elements

---

## Complexity Assessment

| Aspect | Full GrainCosmos | AngelGrain + Features |
|--------|------------------|----------------------|
| Effects | 11 distinct algorithms | 1 granular + 3 modules |
| Parameters | 13 + 44 variations | ~10 total |
| DSP Complexity | VERY HIGH | MEDIUM |
| UI Complexity | 9 knobs + 11 effect buttons + 4 variation buttons + looper | ~8-10 knobs |
| Implementation Time | 30-40 hrs | 8-12 hrs |

---

## Sources

### Local References
- `plugins/Scatter/.ideas/architecture.md` - Complete granular delay architecture
- `plugins/AngelGrain/.ideas/architecture.md` - Simplified granular delay with chaos/character
- `plugins/Scatter/.ideas/mockups/v4-ui.yaml` - UI design with particle field
- `plugins/AngelGrain/.ideas/mockups/v1-ui.yaml` - Neomorphic UI design

### Web References
- [JUCE Tutorial: AudioProcessorGraph](https://juce.com/tutorials/tutorial_audio_processor_graph/)
- [JUCE Forum: Creating Chain of Effects](https://forum.juce.com/t/creating-a-chain-of-effects/10456)
- [JUCE Forum: Granular Synthesis Advice](https://forum.juce.com/t/advice-for-learning-about-granular-microsound/60359)
- [Ross Bencina: Real-Time Granular Synthesis](http://www.rossbencina.com/static/code/granular-synthesis/BencinaAudioAnecdotes310801.pdf)
- [Awesome JUCE](https://github.com/sudara/awesome-juce)

---

## Recommendations

**Pivot to Simplified Approach:**
- Clone AngelGrain as starting point
- Add three enhancement modules:
  1. **Envelope modifier**: Change delay sound envelope
  2. **Distortion**: Waveshaping on grain output
  3. **Freeze**: Hold current buffer state

**Next Steps:**
1. Create UI mockup showing AngelGrain base + new controls
2. Define parameter ranges for envelope, distortion, freeze
3. Plan DSP integration points

**Complexity:** MEDIUM (achievable in 8-12 hours of implementation)
