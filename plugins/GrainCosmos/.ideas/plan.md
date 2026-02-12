# GrainCosmos - Implementation Plan

**Date:** 2026-02-12
**Complexity Score:** 4.1 (Complex)
**Strategy:** Phase-based implementation

---

## Complexity Factors

**Calculation breakdown:**

- **Parameters:** 10 parameters (10/5 = 2.0 points, capped at 2.0) = 2.0
  - 8 float parameters (delay_time, grain_size, envelope_shape, distortion_amount, feedback, chaos, character, mix)
  - 2 bool parameters (freeze, tempo_sync)

- **Algorithms:** 7 DSP components = 7
  - Grain Buffer (DelayLine)
  - Grain Voice Engine (32 polyphonic voices)
  - Grain Scheduler
  - Envelope Modulator (ADSR + Tukey)
  - Waveshaping Distortion (polynomial)
  - Freeze State Controller
  - Pitch Quantization + Pan + Position Randomization

- **Features:** 1 point
  - Feedback loops (+1)

- **Total:** 2.0 + 7 + 1 = 10.0 → capped at 5.0

**Final Score:** 4.1 (Phase-based implementation required)

**Rationale:**
- Parameter count (10) is moderate
- Granular synthesis is HIGH complexity (32 voices, dual envelope, real-time scheduling)
- Envelope modulation adds CPU cost and algorithmic complexity (new, not in AngelGrain)
- Waveshaping distortion is MEDIUM (polynomial calculation)
- Freeze functionality is LOW (simple state flag)
- Overall: More complex than AngelGrain due to envelope modulation and distortion, but architecture is proven

---

## Stages

- Stage 0: Research ✓
- Stage 1: Planning ← Current
- Stage 2: Foundation ← Next
- Stage 3: Shell
- Stage 4: DSP [4 phases]
- Stage 5: GUI [3 phases]
- Stage 6: Validation

---

## Complex Implementation (Score ≥ 3.0)

### Stage 4: DSP Phases

#### Phase 4.1: Core Granular Engine

**Goal:** Implement AngelGrain-based granular synthesis with 32 polyphonic voices

**Components:**
- Grain Buffer (juce::dsp::DelayLine with Lagrange3rd interpolation)
- Grain Voice Engine (32 polyphonic voices, copied from AngelGrain)
- Grain Scheduler (sample-based timer, density control via character)
- Tukey Window Function (character-controlled crossfade, from AngelGrain)
- Pitch Quantization (octaves and fifths, from AngelGrain)
- Random Pan Generator (chaos-controlled, from AngelGrain)
- Position Randomization (chaos-controlled, from AngelGrain)
- Feedback Loop (soft saturation, from AngelGrain)
- Dry/Wet Mixer (juce::dsp::DryWetMixer)

**Basic signal flow:**
```
Input → Dry/Wet Mixer (capture) → Feedback Mix → Grain Buffer
→ Grain Scheduler → Voice Engine (32 voices) → Tukey Window
→ Pitch Shift + Pan → Sum → Feedback Gain → Dry/Wet Mixer (blend) → Output
```

**Test Criteria:**
- [ ] Plugin loads in DAW without crashes
- [ ] Audio passes through (wet and dry signals audible)
- [ ] Delay_time parameter controls grain spawn interval correctly
- [ ] Grain_size parameter changes grain duration correctly
- [ ] Character parameter adjusts density and crossfade (glitchy ↔ smooth)
- [ ] Chaos parameter randomizes position, pitch, pan, timing
- [ ] Feedback parameter controls recirculation amount
- [ ] Mix parameter blends dry/wet correctly
- [ ] No artifacts or discontinuities at nominal settings
- [ ] CPU usage acceptable at 32 voices (test with 100% character)

**Duration Estimate:** 3-4 hours (copying proven AngelGrain architecture)

---

#### Phase 4.2: Envelope Modulation System

**Goal:** Implement ADSR envelope modulation with dual envelope multiplication (ADSR × Tukey)

**Components:**
- Envelope Modulator (per-grain ADSR generation)
  - Percussive envelope (fast attack, fast decay, no sustain)
  - Balanced envelope (medium attack/decay/sustain/release)
  - Smooth envelope (slow attack, long sustain, slow release)
  - Linear crossfade based on envelope_shape (0.0-1.0)
- Dual Envelope Multiplication: `finalEnvelope = adsrEnvelope * tukeyWindow`
- Integration with Grain Voice Engine (apply combined envelope per sample)

**Parameter connections:**
- envelope_shape: Crossfades between percussive/balanced/smooth ADSR shapes
- grain_size: Controls total envelope duration
- character: Controls Tukey window alpha (crossfade amount)

**Test Criteria:**
- [ ] envelope_shape parameter morphs between percussive/balanced/smooth correctly
- [ ] Percussive setting (0.0) creates fast, transient grains
- [ ] Smooth setting (1.0) creates slow attack, sustained grains
- [ ] Envelope crossfade is smooth (no clicks or sudden changes)
- [ ] Dual envelope multiplication produces expected results (ADSR × Tukey)
- [ ] CPU usage with dual envelope is acceptable (measure vs Phase 4.1)
- [ ] No artifacts at extreme settings (envelope_shape 1.0 + character 100%)

**Duration Estimate:** 2-3 hours (new feature, moderate complexity)

---

#### Phase 4.3: Distortion and Freeze

**Goal:** Add waveshaping distortion unit and freeze state controller

**Components:**
- Waveshaping Distortion Unit
  - Polynomial transfer function: `f(x) = x + k*x^3`
  - Soft clipping: `tanh(output)`
  - Drive mapping: `k = (distortion_amount / 100.0) * 5.0`
  - Placement: Pre-feedback (distorted grains feed back)
- Freeze State Controller
  - Atomic boolean flag: `std::atomic<bool> freezeEnabled`
  - Buffer write logic: Skip writes when freeze enabled
  - Input gain ramp: 0.0→1.0 over 50ms on freeze disable
  - Feedback loop continues in freeze mode (infinite sustain)

**Parameter connections:**
- distortion_amount: Controls polynomial gain factor (0.0-5.0)
- freeze: Enables/disables buffer writes (state flag)
- feedback: Continues applying in freeze mode (accumulates frozen texture)

**Test Criteria:**
- [ ] distortion_amount parameter adds harmonic coloration correctly
- [ ] At 0% distortion, signal is clean (bypassed)
- [ ] At 100% distortion, rich harmonics without digital clipping
- [ ] Soft clipping prevents overflow at all settings
- [ ] Distorted signal feeds back correctly (timbre accumulates)
- [ ] freeze parameter holds buffer content when enabled
- [ ] In freeze mode, grains continue reading from frozen buffer
- [ ] Feedback continues in freeze mode (creates infinite sustain)
- [ ] No click when disabling freeze (input gain ramp works)
- [ ] High feedback + freeze doesn't cause runaway (feedback capped at 0.95)
- [ ] CPU usage acceptable with distortion + freeze active

**Duration Estimate:** 2-3 hours (both features LOW-MEDIUM complexity)

---

#### Phase 4.4: Tempo Sync Integration

**Goal:** Implement BPM-synchronized delay times using host tempo

**Components:**
- Tempo Sync System
  - Host tempo query via juce::AudioPlayHead
  - Note division mapping: 1/16, 1/8, 1/4, 1/2, 1/1
  - delay_time quantization to nearest division when tempo_sync enabled
  - Fallback to 120 BPM if playhead unavailable

**Parameter connections:**
- tempo_sync (bool): Enables/disables note division quantization
- delay_time: Quantized to divisions when tempo_sync true, free-running when false

**Note division mapping (at 120 BPM):**
- 1/16 note: 125ms
- 1/8 note: 250ms
- 1/4 note: 500ms
- 1/2 note: 1000ms
- 1 (whole): 2000ms

**Test Criteria:**
- [ ] tempo_sync parameter switches between ms/divisions correctly
- [ ] When tempo_sync enabled, delay_time snaps to nearest note division
- [ ] Note divisions calculate correctly at various tempos (60, 120, 174 BPM)
- [ ] Parameter automation updates delay time in real-time
- [ ] When tempo_sync disabled, delay_time uses free-running ms
- [ ] Plugin works when host playhead unavailable (offline bounce, default 120 BPM)
- [ ] No artifacts when switching tempo_sync on/off
- [ ] Display shows note division (e.g., "1/4") when tempo_sync enabled

**Duration Estimate:** 1-2 hours (straightforward JUCE pattern)

---

### Stage 5: GUI Phases

#### Phase 5.1: Layout and Basic Controls

**Goal:** Integrate v3-ui.yaml mockup with vintage hardware aesthetic

**Components:**
- Copy v3-ui.html to Source/ui/public/index.html (from mockup)
- Update PluginEditor.h/cpp with WebView setup
- Configure CMakeLists.txt for WebView resources
  - Add juce_gui_extra module
  - Set NEEDS_WEB_BROWSER TRUE
  - Add check_native_interop.js to binary data
- Implement vintage chassis styling
  - Olive-gray background (#4a4a3a to #2a2a1a)
  - Aging effects (scratches, rust patches, worn paint, grease smudges)
  - Retro silver instrument knobs (tapered, knurled, chrome finish)
  - Purple indicators (#9b4dca to #b47eff)
  - Gold text (#ffd89b)
  - Japanese brush pen logo font

**Control layout (from v3-ui.yaml):**
- Top Row (y: 80):
  - DELAY (delay_time) - x: 60
  - GRAIN (grain_size) - x: 220
  - ENVELOPE (envelope_shape) - x: 380
  - DISTORTION (distortion_amount) - x: 540
- Bottom Row (y: 240):
  - FEEDBACK (feedback) - x: 60
  - CHAOS (chaos) - x: 220
  - CHARACTER (character) - x: 380 (with purple glow)
  - MIX (mix) - x: 540
- Footer:
  - FREEZE (freeze) - x: 640, y: 275 (slide toggle)
  - SYNC (tempo_sync) - x: 60, y: 425 (slide toggle)

**Test Criteria:**
- [ ] WebView window opens with correct size (800×500px, fixed)
- [ ] All 10 controls visible and styled correctly
- [ ] Layout matches mockup design (two rows + footer)
- [ ] Background and aging effects render properly
- [ ] Knobs have retro silver instrument appearance
- [ ] Character knob has soft purple glow effect
- [ ] Toggle switches use slide-style design
- [ ] Text uses gold color with wide letter-spacing
- [ ] Logo displays with Japanese brush pen font
- [ ] No layout breaks or overlapping elements

**Duration Estimate:** 3-4 hours (vintage aesthetic requires detailed CSS)

---

#### Phase 5.2: Parameter Binding and Interaction

**Goal:** Two-way parameter communication (UI ↔ DSP) for all 10 parameters

**Components:**
- JavaScript → C++ relay calls (control changes)
  - 8 rotary knobs: delay_time, grain_size, envelope_shape, distortion_amount, feedback, chaos, character, mix
  - 2 toggle switches: freeze, tempo_sync
  - Use getSliderState() for float parameters
  - Use getToggleState() for bool parameters
- C++ → JavaScript parameter updates (host automation)
  - Value formatting and display (ms, %, etc.)
  - Real-time parameter updates during playback
  - Preset changes update all UI elements

**Knob interaction (relative drag):**
- Frame-delta pattern: `rotation += (lastY - currentY) * sensitivity`
- NOT absolute positioning: `rotation = startY - currentY` (WRONG)
- Update parameter via `sliderState.setValue(normalizedValue)`
- Visual update via `getNormalisedValue()` in callback

**Toggle interaction (slide switches):**
- Click toggles boolean state
- Visual feedback: Slide animation between on/off positions

**Test Criteria:**
- [ ] All 8 rotary knobs respond to drag correctly
- [ ] Dragging updates DSP parameters in real-time
- [ ] Knobs use relative drag (not absolute positioning)
- [ ] Both toggle switches work correctly
- [ ] Host automation updates UI controls
- [ ] Preset changes update all 10 UI elements
- [ ] Parameter values display correctly (with units: s, ms, %)
- [ ] No lag or visual glitches during interaction
- [ ] Parameter smoothing prevents zipper noise (if applicable)
- [ ] WebSliderParameterAttachment uses 3-parameter constructor (JUCE 8 requirement)

**Duration Estimate:** 2-3 hours (10 parameters, straightforward bindings)

---

#### Phase 5.3: Visual Polish and Special Effects

**Goal:** Implement vintage hardware visual details (aging, glow effects, special indicators)

**Components:**
- Aging layers (texture overlays)
  - Scratch texture: CSS noise with 0.08 opacity
  - Rust patches: Gradient patches in corners (0.15 opacity)
  - Worn areas: Faded sections with base color #3d3d2d
  - Chipped edges: Border roughness (2-4px chips)
  - Grease smudges: Dark patches around controls (0.2 opacity)
- Character knob glow effect
  - Purple glow (#9b4dca)
  - Blur: 20px, Spread: 5px, Opacity: 0.6
  - Applied to CHARACTER knob only
- Knob indicator gradients
  - Purple gradient (#9b4dca to #b47eff) on silver surface
  - Fixed lighting layer (doesn't rotate with knob)
  - Inset highlights at top-left, inset shadows at bottom-right

**Advanced visual elements:**
- Tempo sync indicator (when tempo_sync enabled, show note division instead of ms)
- Freeze indicator (visual feedback when freeze active)
- Parameter value displays (show current values for all knobs)

**Test Criteria:**
- [ ] Aging effects look realistic (not overdone or cartoonish)
- [ ] Character knob has visible purple glow
- [ ] Knob indicators rotate smoothly without visual artifacts
- [ ] Lighting highlights are fixed (don't rotate with knob)
- [ ] Chrome texture looks metallic (silver gradients)
- [ ] Knurled texture is visible (45-degree cross-hatch)
- [ ] Tapered conical shape is apparent (15-20 degree angle)
- [ ] Tempo sync shows note division (1/16, 1/8, etc.) when enabled
- [ ] Freeze toggle shows active state clearly
- [ ] Performance acceptable (no frame drops at 60fps)
- [ ] Visual polish matches vintage hardware reference images

**Duration Estimate:** 2-3 hours (CSS-heavy, fine-tuning required)

---

### Implementation Flow

- **Stage 2: Foundation** - Project structure, CMakeLists.txt, build system
- **Stage 3: Shell** - APVTS parameters (10 parameters), Processor boilerplate
- **Stage 4: DSP** - 4 phases
  - Phase 4.1: Core Granular Engine (3-4 hours)
  - Phase 4.2: Envelope Modulation (2-3 hours)
  - Phase 4.3: Distortion + Freeze (2-3 hours)
  - Phase 4.4: Tempo Sync (1-2 hours)
- **Stage 5: GUI** - 3 phases
  - Phase 5.1: Layout + Basic Controls (3-4 hours)
  - Phase 5.2: Parameter Binding (2-3 hours)
  - Phase 5.3: Visual Polish (2-3 hours)
- **Stage 6: Validation** - Presets, pluginval, changelog (2-3 hours)

**Total Estimated Duration:** 19-25 hours

---

## Implementation Notes

### Thread Safety

- All parameter reads use atomic `getRawParameterValue()->load()` (APVTS)
- Freeze state uses `std::atomic<bool>` for thread-safe reads/writes
- Voice array pre-allocated in `prepareToPlay()` (no allocations in `processBlock()`)
- Envelope calculations optimized (consider pre-calculating tables if per-sample cost is too high)
- Grain scheduler uses sample counter (no time-based operations in audio thread)

**Example:**
```cpp
// Audio thread: Atomic parameter reads
float delayTime = *apvts.getRawParameterValue("delay_time");
float chaos = *apvts.getRawParameterValue("chaos");
bool frozen = freezeEnabled.load(std::memory_order_relaxed);

// Message thread: Atomic parameter writes (via APVTS)
apvts.getParameter("delay_time")->setValueNotifyingHost(newValue);
freezeEnabled.store(true, std::memory_order_relaxed);
```

### Performance

- **Estimated CPU usage per component:**
  - Granular engine (32 voices): ~30-45% single core (from AngelGrain)
  - Dual envelope (ADSR + Tukey): ~10-15% (additional over base)
  - Waveshaping distortion: ~5-10%
  - Freeze logic: <1%
  - Total: ~45-70% single core at 48kHz, 512 sample buffer

- **Optimization opportunities:**
  - Pre-calculate envelope tables for common (character, envelope_shape) combinations
    - 11 character steps × 11 envelope_shape steps = 121 tables
    - Reduces per-sample pow/cos/sin calls to linear interpolation
  - Use `x*x*x` instead of `pow(x, 3)` for cubic distortion
  - Use SIMD for grain summing (`juce::FloatVectorOperations::add`)
  - Early-exit inactive voices (check active flag before processing)

- **Buffer size sensitivity:**
  - Smaller buffers (64 samples) -> more overhead
  - Larger buffers (2048 samples) -> better efficiency

- **Performance triggers:**
  - If CPU > 70% sustained: Reduce to 16 voices or implement envelope tables
  - If CPU > 85% sustained: Show warning in UI, recommend reducing character/chaos

### Latency

- **Processing latency sources:**
  - Grain buffer read: 10-2000ms (user-controlled via delay_time)
  - No FFT latency (time-domain pitch shifting)
  - No lookahead
  - Freeze: No additional latency (buffer hold is instant)

- **Host compensation:**
  - Granular delay is creative effect, not precision time-alignment
  - Variable delay_time makes latency reporting impractical
  - Recommendation: Don't report latency via `getLatencySamples()`
  - Matches industry standard (Output Portal, Arturia Fragments don't report latency)

### Denormal Protection

- Use `juce::ScopedNoDenormals` in `processBlock()`
- Grain voices may produce denormals near envelope end (fade to zero)
- Feedback loop may accumulate denormals at high feedback
- Distortion polynomial may produce denormals near zero
- DelayLine and DryWetMixer handle denormals internally
- Input gain ramp on freeze disable prevents denormal buildup

### Known Challenges

- **Envelope modulation CPU cost:** Dual envelope (ADSR × Tukey) adds significant per-sample processing
  - Solution: Pre-calculate 121 envelope tables if needed (fallback 4.2.1)
  - Monitor CPU usage early, implement tables if >70%

- **Distortion feedback harshness:** High distortion + high feedback can become unmusical
  - Solution: Cap feedback at 0.95, users can lower distortion/feedback
  - Consider post-feedback placement if pre-feedback proves too harsh (fallback 4.3.1)

- **Parameter interaction confusion:** envelope_shape and character both affect envelope
  - Solution: Clear parameter naming (ENVELOPE vs CHARACTER), UI tooltips
  - Document interaction in manual: "ENVELOPE controls temporal shape, CHARACTER controls crossfade"
  - If UX testing shows confusion, simplify to single envelope (fallback 4.2.2)

- **Freeze mental model:** Freeze doesn't "pause" everything (feedback continues)
  - Solution: Clear UI indicator (FREEZE active), manual documentation
  - If feedback causes confusion, disable feedback in freeze mode (fallback 4.3.2)

- **Vintage CSS complexity:** Aging effects, knurled textures, chrome gradients require detailed CSS
  - Solution: Use CSS gradients, noise textures, box-shadow extensively
  - Reference TapeAge or other vintage plugins for CSS patterns
  - Iterative approach: Base styling → aging → polish

**References to similar plugins:**
- AngelGrain: Proven granular engine architecture (copy voice management, scheduler)
- TapeAge: Vintage hardware CSS implementation (aging, chrome, textures)
- FlutterVerb: Tempo sync implementation pattern
- Output Portal: Granular delay parameter reference (density, grain size)

---

## References

### Contract Files

- Creative brief: `plugins/GrainCosmos/.ideas/creative-brief.md`
- Parameter spec: `plugins/GrainCosmos/.ideas/parameter-spec.md`
- DSP architecture: `plugins/GrainCosmos/.ideas/architecture.md` (this file)
- UI mockup: `plugins/GrainCosmos/.ideas/mockups/v3-ui.yaml`

### Related Plugins for Reference

- **AngelGrain** - Granular engine architecture (voice management, scheduler, Tukey window)
  - Copy: Grain buffer, voice engine, scheduler, pitch quantization, chaos system
  - Modify: Add ADSR envelope multiplication, distortion unit, freeze state

- **TapeAge** - Vintage hardware UI implementation
  - Reference: Aging effects (scratches, rust, worn paint), chrome textures, CSS patterns

- **FlutterVerb** - Tempo sync implementation
  - Reference: AudioPlayHead usage, note division mapping, BPM query

- **Output Portal** - Granular delay parameter ranges
  - Reference: Density parameter, grain size ranges, mix behavior

- **Arturia Efx Fragments** - Granular texture modes
  - Reference: Density control, randomization systems

### Professional Plugins Researched

1. **Red Panda Particle** - Freeze functionality, granular delay pedal
2. **Output Portal** - Granular delay with pitch shifting, scale quantization
3. **Mutable Instruments Clouds/Beads** - Per-grain envelope control, density/texture parameters
4. **Arturia Efx Fragments** - Granular texture modes, randomization
5. **Kaivo** - ADSR envelope per grain

### JUCE Documentation References

- `juce::dsp::DelayLine` - Variable delay with Lagrange3rd interpolation
- `juce::dsp::WindowingFunction` - Window table generation (Tukey custom, Hann built-in)
- `juce::dsp::DryWetMixer` - Latency-compensated dry/wet mixing
- `juce::AudioPlayHead` - Host transport state for tempo sync
- `juce::Random` - Thread-safe random number generation

### Technical Resources

- Granular Synthesis Basics (PsychoSynth.com)
- Towards Per-Grain Parameterisation in Granular Synthesiser Design (NIME 2025)
- Sound In A Nutshell: Granular Synthesis (envelope windowing)
- Waveshaping Fundamentals (Columbia University DSP)
- Red Panda Particle Manual (freeze behavior)
