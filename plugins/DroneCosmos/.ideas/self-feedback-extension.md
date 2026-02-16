# DroneCosmos — Extended Self-Feedback System ✅

## Date
2026-02-16

## Summary
Extended self-feedback system from single FM to 3 parallel feedback paths: Phase (FM), Amplitude (AM), and Delay (Comb/Pitch shift).

## New Parameters (5 total)

| ID | Name | Range | Default | Description |
|----|------|-------|---------|-------------|
| self_fb_phase | Self FB Phase | 0 - 100 | 0 | Phase feedback (FM-style) |
| self_fb_amp | Self FB Amplitude | 0 - 100 | 0 | Amplitude feedback (AM-style) |
| self_fb_delay | Self FB Delay | 0 - 100 | 0 | Delay feedback (Comb-style) |
| self_fb_delay_time | Self FB Delay Time | 0.1 - 50.0 | 5.0 | ms |
| self_fb_pitch | Self FB Pitch Shift | -24 - +24 | 0 | semitones |

**Replaced**: `self_mod` → `self_fb_phase`

## Feedback Paths

### 1. Phase Feedback (self_fb_phase)
**Type**: Frequency Modulation (FM)
**Implementation**: Output modulates own phase increment
**Effect**: Dynamic frequency changes, harmonic generation
**Formula**: `phase += output * phaseScale * 0.1`
**Characteristics**:
- Subtle: gentle vibrato, beating
- Extreme: complex inharmonic spectra, noise-like textures

### 2. Amplitude Feedback (self_fb_amp)
**Type**: Amplitude Modulation (AM)
**Implementation**: Output modulates own amplitude
**Effect**: Volume varies with output level
**Formula**: `output *= (1.0 + ampScale * prevOutput)`
**Characteristics**:
- Subtle: gentle tremolo
- Extreme: pumping, gating, ring modulation effects
- Clamped to prevent negative gain: `juce::jlimit(0.0f, 2.0f, amMod)`

### 3. Delay Feedback (self_fb_delay + self_fb_pitch)
**Type**: Comb Filtering with Pitch Shift
**Implementation**: Ring buffer with variable read rate
**Effect**: Resonant peaks, shimming, sub-oscillator
**Buffer Size**: 2400 samples (50ms @ 48kHz)
**Characteristics**:
- **Delay Time** (0.1-50ms): Controls resonance frequency
  - Short: High-frequency comb filtering
  - Long: Low-frequency resonance, echo effects
- **Pitch Shift** (-24 to +24 semitones): Controls feedback timbre
  - 0: Standard comb filtering
  - +12: Shimmering high-octave feedback
  - -12: Sub-octave reinforcement
  - Non-integer values: Chorus/detune effects

## Technical Implementation

### OscillatorState Structure
```cpp
struct OscillatorState
{
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float dcFilterZ1 = 0.0f;

    // Self-feedback states
    float prevOutput = 0.0f;
    std::array<float, 2400> delayBuffer{};  // 50ms @ 48kHz
    int delayWritePos = 0;
    float delayReadPos = 0.0f;
};
```

### Processing Order (per oscillator)
1. **Generate waveform**: `output = generateWaveform(phase, waveParam) * level`
2. **Apply AM feedback**: `output *= (1.0 + selfFbAmpAmt * prevOutput)`
3. **Apply delay feedback**: Read from delay buffer with pitch shift, add to phase
4. **Apply phase feedback**: `phase += output * selfFbPhaseAmt`
5. **Store for next sample**: `prevOutput = output`
6. **Safety checks**: NaN/Inf check, DC filtering

### Delay Feedback with Pitch Shift
```cpp
float pitchShiftRatio = std::pow(2.0f, pitchShiftSemitones / 12.0f);
float delaySamples = delayTimeMs * sampleRate / 1000.0f;
float readPos = writePos - delaySamples * pitchShiftRatio;

// Linear interpolation for smooth pitch shift
int readPos0 = (int)readPos;
int readPos1 = (readPos0 + 1) % bufferSize;
float frac = readPos - readPos0;
float delayed = buffer[readPos0] * (1.0f - frac) + buffer[readPos1] * frac;
```

## Sound Design Examples

### Subtle Warmth
```
self_fb_phase = 15
self_fb_amp = 10
self_fb_delay = 0
```
→ Gentle vibrato with slight amplitude variation

### Shimmering Highs
```
self_fb_phase = 30
self_fb_delay = 40
self_fb_delay_time = 8.0
self_fb_pitch = +12
```
→ High-octave feedback creates bright shimmering harmonics

### Deep Sub Bass
```
self_fb_phase = 20
self_fb_delay = 50
self_fb_delay_time = 15.0
self_fb_pitch = -12
```
→ Low-octave reinforcement adds sub-bass foundation

### Chaotic Ringing
```
self_fb_phase = 80
self_fb_amp = 60
self_fb_delay = 70
self_fb_delay_time = 3.0
self_fb_pitch = +7
```
→ Metallic ringing, inharmonic spectra, pumping

## Preset System Updates

### Updated Parameter Lists
All preset save/load functions updated to include new 5 parameters:
- `saveGlobalPreset()`: Saves all 43 parameters
- `saveOscPreset()`: Oscillator-specific (unchanged)
- `loadGlobalPreset()`: Default values updated

### Factory Preset Defaults
All self-feedback parameters default to 0 (off) for clean starting point.

## Parameter Count
**Previous**: 39 automatable parameters
**Current**: 43 automatable parameters (+4)
**Change**: `self_mod` → 5 new parameters

## Safety Features
- **Gain clamping**: AM feedback limited to prevent negative gain
- **Phase clamping**: Phase kept in [0, 0.5] range to prevent runaway
- **NaN/Inf checking**: All outputs checked before mixing
- **DC filtering**: 20Hz highpass on each oscillator output

## Build Status
✅ Build successful
✅ AU installed to ~/Library/Audio/Plug-Ins/Components/
✅ VST3 installed to ~/Library/Audio/Plug-Ins/VST3/
✅ Standalone app available

## Notes
- Delay feedback uses linear interpolation for smooth pitch shifting
- All three feedback paths can be used simultaneously for complex textures
- Range scaling (L/M/H) applies to all self-feedback parameters
- Mode scaling (Drone/Noise) applies to phase feedback more aggressively
- AM feedback scaled by 0.5x to prevent excessive volume pumping

## Future Enhancements
- Stereo delay offset per oscillator
- Filter in feedback path
- Feedback polarity inversion
- LFO modulation of delay time
