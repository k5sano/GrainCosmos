# DroneCosmos — Modulation Scaling Update ✅

## Date
2026-02-16

## Summary
Added `mod_mode` parameter and implemented proper modulation scaling to control the range from "Drone Mode" (subtle) to "Noise Mode" (extreme).

## New Parameter

| ID | Name | Range | Default | Description |
|----|------|-------|---------|-------------|
| mod_mode | Modulation Mode | 0.0 - 1.0 | 0.0 | 0.0=Drone(穏やか) / 1.0=Noise(過激) |

## Modulation Scaling

### Drone Mode (mod_mode = 0.0)
- **self_mod 最大変調指数**: 0.3
- **cross_mod 最大変調指数**: 0.5
- **ring_mod 最大変調指数**: 0.5
- **chaos_mod LFO振幅**: 0.1
- **特徴**: 温かいうねり、ゆっくりしたビート、倍音の微妙な変化

### Noise Mode (mod_mode = 1.0)
- **self_mod 最大変調指数**: 5.0
- **cross_mod 最大変調指数**: 8.0
- **ring_mod 最大変調指数**: 8.0
- **chaos_mod LFO振幅**: 2.0
- **特徴**: 激しい変調、ノイズに近いテクスチャ

### Intermediate Values
Linear interpolation between Drone and Noise modes.
Recommended sweet spot: **0.3 - 0.5**

## Code Changes

### 1. PluginProcessor.h
- Added `dcFilterZ1` to `OscillatorState` for DC offset removal
- Added `dcFilter()` helper function declaration

### 2. PluginProcessor.cpp
- Added `mod_mode` parameter to layout
- Implemented modulation scaling with mode-based interpolation
- Added phase clamping in self-modulation to prevent runaway/NaN/Inf
- Added 1st order DC highpass filter (~20Hz) to each oscillator output
- Updated parameter-spec.md with mod_mode documentation

### 3. DC Filter Implementation
```cpp
float dcFilter(float sample, float& z1, float rc)
{
    // 1st order highpass: y[n] = rc * (y[n-1] + x[n] - x[n-1])
    float y = rc * (z1 + sample - z1);
    z1 = sample;
    return y;
}
```

## Technical Details

### Self-Modulation Safety
- Phase clamped to [0, 0.5] to prevent runaway oscillation
- Negative phases wrapped using `std::fmod()`
- Ensures stable operation even at extreme modulation settings

### DC Removal
- Each oscillator output passes through ~20Hz highpass
- Removes DC offset introduced by FM modulation
- Prevents subsonic buildup in the signal chain

## Testing Recommendations

1. **Drone Mode (mod_mode = 0.0)**
   - Set self_mod to 100% → should hear gentle warmth
   - Set cross_mod to 100% → subtle beating between osc pairs
   - Set ring_mod to 100% → slow evolution of harmonics

2. **Noise Mode (mod_mode = 1.0)**
   - Same settings should produce chaotic, noise-like textures
   - Self-modulation should create complex inharmonic spectra

3. **Sweet Spot (mod_mode = 0.3 - 0.5)**
   - Musically rich textures
   - Controlled chaos with clear pitch foundation

## Build Status
✅ Build successful
✅ AU installed to ~/Library/Audio/Plug-Ins/Components/
✅ VST3 installed to ~/Library/Audio/Plug-Ins/VST3/
✅ Standalone app available

## Total Parameters
Now 38 automatable parameters (was 37) + 2 settings
