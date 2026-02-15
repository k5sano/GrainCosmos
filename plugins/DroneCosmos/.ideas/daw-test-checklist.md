# DroneCosmos — DAW Test Checklist

## Date
2026-02-15

## DAW
Ableton Live 12 Suite

## Test Procedure

### 1. Plugin Scan
- [ ] Open Ableton Live Preferences → Plug-Ins
- [ ] Verify DroneCosmos appears in the plugin list
- [ ] Check that both VST3 and AU versions are available

### 2. Basic Loading
- [ ] Create new MIDI or Audio track
- [ ] Load DroneCosmos on the track
- [ ] Verify plugin UI opens without crash

### 3. Sound Output (Phase 1)
- [ ] Confirm drone sound is audible immediately (no MIDI needed)
- [ ] Default sound should be rich drone (4 oscillators combined)
- [ ] Verify no clicks, pops, or artifacts

### 4. Parameter Testing

#### Oscillator Bank
- [ ] **Waveform morphing**: Test each osc_a/b/c/d_waveform
  - 0.0 = Sine (smooth)
  - 1.0 = Saw (brighter)
  - 2.0 = Square (hollow)
  - 3.0 = Triangle (mellow)
  - Test intermediate values (0.5, 1.5, etc.)

- [ ] **Pitch**: Test osc_a/b/c/d_pitch
  - Verify pitch changes in semitones
  - Check extreme values (-24, +24)

- [ ] **Detune**: Test osc_a/b/c/d_detune
  - Verify subtle pitch shifting
  - Test extreme detuning for beating effects

- [ ] **Level**: Test osc_a/b/c/d_level
  - Verify each oscillator can be soloed
  - Check level at 0% (silence)

#### Modulation Matrix
- [ ] **Self Modulation (self_mod)**
  - Increase to hear waveform complexity
  - Extreme values should produce noise-like textures

- [ ] **Cross Modulation (cross_mod)**
  - Test A↔B interaction
  - Test C↔D interaction
  - Should create metallic beating

- [ ] **Ring Modulation (ring_mod)**
  - A→B→C→D→A cycle
  - Should create feedback-like chaos

- [ ] **Chaos Modulation (chaos_mod)**
  - Add randomness to other modulations
  - Slow evolution of textures

#### Drone Control
- [ ] **Drone Pitch (drone_pitch)**
  - Test range 20-500 Hz
  - Verify smooth pitch transitions

#### Output
- [ ] **Output Volume (output_volume)**
  - Test 0-100% range
  - Check at 0% (silence)

- [ ] **Limiter Threshold (limiter_threshold)**
  - Test limiting kicks in at high modulation
  - No distortion should occur with limiter active

- [ ] **Limiter Release (limiter_release)**
  - Test fast vs slow release

### 5. Automation
- [ ] Automate drone_pitch (pitch sweep)
- [ ] Automate self_mod (morph from clean to chaotic)
- [ ] Automate cross_mod (beating effects)
- [ ] Verify automation playback is smooth

### 6. Performance
- [ ] Check CPU usage (should be low for Phase 1)
- [ ] Verify no audio dropouts
- [ ] Test at different buffer sizes (64, 128, 256, 512)

### 7. Save/Load
- [ ] Save Ableton Set with DroneCosmos
- [ ] Close and reopen Ableton Live
- [ ] Load set and verify parameters are recalled
- [ ] Verify drone sound matches saved state

### 8. UI (Placeholder)
- [ ] Verify placeholder UI displays correctly
- [ ] Check window is resizable
- [ ] No graphical glitches

## Bug Report
Record any issues found:

```
Date:
Issue:
Steps to reproduce:
Expected behavior:
Actual behavior:
```

## Sign-off
Phase 1 testing: [ ] PASS / [ ] FAIL

Tester notes:
