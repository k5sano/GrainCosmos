# DroneCosmos - Session Summary

**Date**: 2026-02-16
**Status**: Phase 1 Complete with Extended Self-Feedback System

## Project Overview
DroneCosmos is a chaotic drone synthesizer with 4 oscillators, extensive FM modulation, and gesture control capabilities via OSC.

## Completed Features

### Core Engine (Phase 1)
- **4-Oscillator Bank** with waveform morphing (sine → saw → square → triangle)
- **FM Modulation Matrix**:
  - Self feedback (phase, amplitude, delay paths)
  - Cross modulation (A↔B, C↔D pairs)
  - Ring modulation (A→B→C→D→A cycle)
  - Chaos LFO for randomness
- **Modulation Range Control**: Low/Mid/High scaling
- **Modulation Mode**: Drone (0.0) to Noise (1.0) interpolation
- **DC Filtering**: 20Hz highpass on each oscillator
- **Safety**: Phase clamping, NaN/Inf checking, limiter

### Extended Self-Feedback System
- **self_fb_phase**: Phase/FM feedback (dynamic harmonics)
- **self_fb_amp**: Amplitude/AM feedback (tremolo/gating)
- **self_fb_delay**: Delay feedback (comb filtering)
- **self_fb_delay_time**: 0.1-50ms delay time
- **self_fb_pitch**: -24 to +24 semitones pitch shift

### Preset System
- **3 Categories**: Global + OscA/B/C/D
- **JSON Format**: Human-editable preset files
- **UI**: 800x150 with ComboBox + Save buttons
- **Location**: `~/Documents/DroneCosmos/Presets/`

## Technical Details

### Parameters
- **Total**: 43 automatable + 2 settings
- **Oscillators**: 16 params (waveform, pitch, detune, level × 4)
- **Self Feedback**: 5 params
- **Modulation**: 4 params (cross, ring, chaos, mode)
- **Output**: 4 params (volume, limiter × 3, drone pitch)
- **Range**: 1 param (mod_range: Low/Mid/High)

### Tech Stack
- JUCE 8.0.4
- C++17, CMake + Ninja
- macOS arm64
- VST3 + AU + Standalone
- Reference: `../GrainCosmos/JUCE`

### Project Structure
```
plugins/DroneCosmos/
├── .ideas/
│   ├── creative-brief.md
│   ├── architecture.md
│   ├── parameter-spec.md
│   ├── plan.md
│   ├── phase1-complete.md
│   ├── modulation-scaling-update.md
│   ├── mod-range-feature.md
│   ├── preset-system.md
│   └── self-feedback-extension.md
├── CMakeLists.txt
└── Source/
    ├── PluginProcessor.h
    ├── PluginProcessor.cpp
    ├── PluginEditor.h
    └── PluginEditor.cpp
```

## Build & Installation

### Build Command
```bash
cd ~/Documents/plugin-freedom-system/plugins/DroneCosmos
rm -rf build && cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Installed Locations
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/DroneCosmos.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/DroneCosmos.component`
- **Standalone**: `build/DroneCosmos_artefacts/Release/Standalone/DroneCosmos.app`

## GitHub Repositories

### Standalone
- **URL**: `https://github.com/k5sano/DroneCosmos`
- **Latest Commit**: `09407a6` - Extended self-feedback system

### Plugin Freedom System
- **URL**: `git@github.com:k5sano/GrainCosmos.git`
- **Location**: `plugins/DroneCosmos/` submodule

## Usage in DAW

### Loading the Plugin
1. Open Ableton Live 12 (or Logic Pro)
2. Create new track
3. Load DroneCosmos from plug-ins list
4. **Important**: Restart DAW after updating plugin

### Basic Operation
- **No MIDI required**: Drone plays automatically at `drone_pitch` frequency
- **Default**: Rich 4-osc drone at 55Hz (A1)
- **Control parameters**: Use DAW's automation or plugin UI

### Preset System
1. **Save**: Click "Save" (Global) or "Save A/B/C/D" (oscillator)
2. **Load**: Select from dropdown
3. **Location**: `~/Documents/DroneCosmos/Presets/`

## Sound Design Starting Points

### Clean Drone
```
mod_range = Low
mod_mode = 0.0
All self_fb_* = 0
cross_mod = 0, ring_mod = 0
```

### Warm Musical Drone
```
mod_range = Mid
mod_mode = 0.3-0.5
self_fb_phase = 20-30
self_fb_amp = 10-20
cross_mod = 30-40
```

### Shimmering Highs
```
self_fb_phase = 30
self_fb_delay = 40
self_fb_delay_time = 8.0 ms
self_fb_pitch = +12
```

### Sub Bass
```
self_fb_phase = 20
self_fb_delay = 50
self_fb_delay_time = 15.0 ms
self_fb_pitch = -12
```

### Chaos Machine
```
mod_range = High
mod_mode = 1.0
All self_fb_* = 70-90
cross_mod = 80, ring_mod = 70
```

## Next Steps (Future Phases)

### Phase 2: Filter Chain
- SVF filter (cutoff, resonance, filter_morph: LP/BP/HP)
- Comb filter (comb_time, comb_feedback)
- Filter feedback loop with fuzz

### Phase 3: Stutter Engine
- Ring buffer stutter
- Pitch shift per repeat
- Gate patterns

### Phase 4: Full UI
- Parameter sliders
- Visual feedback
- XY pads (from GrainCosmos)

### Phase 5: OSC Integration
- GestureBridge connectivity
- Hand gesture mapping
- Real-time control

## Key Files Reference

### Specification Documents
- `.ideas/creative-brief.md` - Concept and references
- `.ideas/architecture.md` - Signal flow and modules
- `.ideas/parameter-spec.md` - All 43 parameters documented
- `.ideas/plan.md` - 5-phase implementation roadmap

### Implementation Notes
- `.ideas/modulation-scaling-update.md` - mod_mode implementation
- `.ideas/mod-range-feature.md` - L/M/H range switching
- `.ideas/self-feedback-extension.md` - Extended feedback details

### Code Reference
- `GrainCosmos/Source/` - Reference for patterns (Fuzz, presets, XY pad)

## Session Commands Reference

### Build
```bash
cd ~/Documents/plugin-freedom-system/plugins/DroneCosmos
rm -rf build && cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Git Push
```bash
git add -A
git commit -m "feat: description"
git push origin main
```

### Test in DAW
```bash
open -a "Ableton Live 12 Suite"
# Or: open -a Logic Pro
```

## Important Notes

### JUCE Installation
Always use git clone (NOT brew, ZIP, or PKG):
```bash
cd /tmp && git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git
mv JUCE /Applications/
```

### DAW Cache
Ableton Live caches plug-in information. **Always restart the DAW** after updating.

### Memory Location
Session memory: `/Users/sanokeigo/.claude/projects/-Users-sanokeigo-Documents-plugin-freedom-system/memory/`

## Version History
- **v1.0** (2026-02-15): Phase 1 - Oscillator bank + basic modulation
- **v1.1** (2026-02-16): Modulation scaling (mod_mode) + Range (L/M/H)
- **v1.2** (2026-02-16): Preset system
- **v1.3** (2026-02-16): Extended self-feedback system (5 paths)

## Resume Commands

### Continue Development
```bash
cd ~/Documents/plugin-freedom-system/plugins/DroneCosmos
# Edit files, build, test
```

### Check Current Status
```bash
git log --oneline -5
git status
```

### Review Specification
```bash
cat .ideas/plan.md
cat .ideas/parameter-spec.md
```

---

**Session End**: DroneCosmos Phase 1 complete with extended self-feedback, preset system, and ready for DAW testing.
