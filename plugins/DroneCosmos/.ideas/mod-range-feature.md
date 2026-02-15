# DroneCosmos — Modulation Range Feature ✅

## Date
2026-02-16

## Summary
Added `mod_range` parameter with L/M/H range switching for parameter exploration during development.

## New Parameter

| ID | Name | Type | Choices | Default | Description |
|----|------|------|---------|---------|-------------|
| mod_range | Modulation Range | Choice | Low, Mid, High | Mid | L=繊細/M=バランス/H=過激 |

## Range Definitions

### Low (繊細なドローン向け)
- **self_mod**: 0-100 → 変調指数 0-0.1
- **cross_mod**: 0-100 → 変調指数 0-0.2
- **ring_mod**: 0-100 → 変調指数 0-0.2
- **chaos_mod**: 0-100 → LFO振幅 0-0.05
- **用途**: 繊細なテクスチャ、環境音楽的なドローン

### Mid (バランス) - Default
- **self_mod**: 0-100 → 変調指数 0-1.0
- **cross_mod**: 0-100 → 変調指数 0-2.0
- **ring_mod**: 0-100 → 変調指数 0-2.0
- **chaos_mod**: 0-100 → LFO振幅 0-0.3
- **用途**: 汎用的なドローンシンセサイザー

### High (ノイズマシン)
- **self_mod**: 0-100 → 変調指数 0-5.0
- **cross_mod**: 0-100 → 変調指数 0-10.0
- **ring_mod**: 0-100 → 変調指数 0-10.0
- **chaos_mod**: 0-100 → LFO振幅 0-2.0
- **用途**: 激しいテクスチャ、ノイズ、サウンドデザイン

## Combined Scaling Logic

The modulation amount is calculated as:
1. **Range scaling** (mod_range): Maps 0-100 parameter to the appropriate range
2. **Mode scaling** (mod_mode): Provides additional Drone/Noise interpolation

```
selfAmt = selfParam * rangeScale * modeScale
```

## Safety Improvements

### NaN/Inf Checking
- Each oscillator output checked: `if (!std::isfinite(sample)) sample = 0.0f;`
- Final mix checked before output

### Phase Clamping (from previous update)
- Phase clamped to [0, 0.5] to prevent runaway oscillation
- Negative phases wrapped using `std::fmod()`

### DC Filter (from previous update)
- 1st order highpass at ~20Hz on each oscillator output
- Removes DC offset from FM modulation

## Code Implementation

### Parameter Definition
```cpp
layout.add(std::make_unique<juce::AudioParameterChoice>(
    juce::ParameterID{"mod_range", 1}, "Mod Range",
    juce::StringArray{"Low", "Mid", "High"}, 1));
```

### Range Selection
```cpp
int rangeIndex = static_cast<int>(std::round(*modRange));
float rangeSelfScale, rangeCrossScale, rangeRingScale, rangeChaosScale;

switch (rangeIndex)
{
    case 0: // Low
        rangeSelfScale = 0.001f;
        rangeCrossScale = 0.002f;
        rangeRingScale = 0.002f;
        rangeChaosScale = 0.0005f;
        break;
    case 1: // Mid
        rangeSelfScale = 0.01f;
        rangeCrossScale = 0.02f;
        rangeRingScale = 0.02f;
        rangeChaosScale = 0.003f;
        break;
    case 2: // High
        rangeSelfScale = 0.05f;
        rangeCrossScale = 0.1f;
        rangeRingScale = 0.1f;
        rangeChaosScale = 0.02f;
        break;
}
```

## Usage Examples

### Example 1: Delicate Ambient Drone
```
mod_range = Low
mod_mode = 0.0
self_mod = 30
cross_mod = 20
ring_mod = 10
chaos_mod = 5
```

### Example 2: Rich Musical Drone
```
mod_range = Mid
mod_mode = 0.3
self_mod = 50
cross_mod = 40
ring_mod = 30
chaos_mod = 20
```

### Example 3: Chaos Machine
```
mod_range = High
mod_mode = 1.0
self_mod = 100
cross_mod = 80
ring_mod = 70
chaos_mod = 50
```

## Build Status
✅ Build successful
✅ AU installed to ~/Library/Audio/Plug-Ins/Components/
✅ VST3 installed to ~/Library/Audio/Plug-Ins/VST3/
✅ Standalone app available

## Total Parameters
Now 39 automatable parameters + 2 settings

## Notes
- mod_range is primarily a development/exploration tool
- Allows quick switching between subtle and extreme behaviors
- Combined with mod_mode for fine-grained control
- Recommended workflow: Start with Low/Mid range, increase to High for sound design
