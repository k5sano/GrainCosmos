# DroneCosmos — Preset System ✅

## Date
2026-02-16

## Summary
Implemented 3-category preset system with Global and Oscillator-specific (A/B/C/D) preset management.

## Preset Folder Structure
```
~/Documents/DroneCosmos/Presets/
├── Global/          # All parameters
├── OscA/            # osc_a_* parameters only
├── OscB/            # osc_b_* parameters only
├── OscC/            # osc_c_* parameters only
└── OscD/            # osc_d_* parameters only
```

## UI Layout (800x150)

### Header Row
```
[DroneCosmos] [Global Preset ▼] [Save]
```

### Oscillator Row
```
[A ▼][Save A] [B ▼][Save B] [C ▼][Save C] [D ▼][Save D]
```

## Features

### Global Presets
- **Saves**: All 25 parameters (mod_range, mod_mode, all osc params, modulation, output)
- **Default**: "Default" factory preset with rich drone settings
- **File Format**: JSON with all parameter values

### Oscillator Presets
- **Saves**: Only 4 parameters per oscillator (waveform, pitch, detune, level)
- **Default**: Each oscillator has its own default settings
- **File Format**: JSON with oscillator-specific parameters

### Preset Management
- **Scan**: Automatically scans all preset folders on startup
- **Save**: Alert dialog for naming presets
- **Load**: ComboBox selection with "Default" option

## Example JSON Files

### Global Preset: `Global/dark_drone.json`
```json
{
  "mod_range": 1.0,
  "mod_mode": 0.3,
  "osc_a_waveform": 0.0,
  "osc_a_pitch": 0.0,
  "osc_a_detune": 0.0,
  "osc_a_level": 75.0,
  ...
  "drone_pitch": 55.0,
  "output_volume": 80.0,
  "limiter_threshold": -1.0,
  "limiter_release": 100.0
}
```

### Oscillator Preset: `OscA/fat_sine.json`
```json
{
  "osc_a_waveform": 0.0,
  "osc_a_pitch": 0.0,
  "osc_a_detune": 0.0,
  "osc_a_level": 75.0
}
```

## Implementation Details

### Files Modified
- `PluginEditor.h`: Added preset management declarations
- `PluginEditor.cpp`: Full preset system implementation

### Key Functions
- `getPresetsFolder()`: Creates/returns base presets folder
- `getOscPresetFolder()`: Returns oscillator-specific folder
- `scanPresets()`: Scans all preset folders and updates ComboBoxes
- `saveGlobalPreset()`: Saves all parameters to JSON
- `saveOscPreset()`: Saves oscillator-specific parameters
- `loadGlobalPreset()`: Loads all parameters from JSON
- `loadOscPreset()`: Loads oscillator-specific parameters

### Default Preset Values
Each oscillator has unique default settings:
- **OSC A**: Sine (0.0), root pitch, 0 detune, 75% level
- **OSC B**: Sine (0.0), root pitch, +7 cents detune, 75% level
- **OSC C**: Saw (1.0), -1 octave, -5 cents detune, 50% level
- **OSC D**: Square (2.0), +7 semitones, +3 cents detune, 50% level

## Usage Examples

### Save Custom Oscillator Setting
1. Adjust OSC A parameters in DAW
2. Click "Save A" button
3. Enter preset name: "fat_low_sine"
4. Preset saved to `~/Documents/DroneCosmos/Presets/OscA/fat_low_sine.json`

### Create Global Drone Preset
1. Adjust all parameters to desired settings
2. Click "Save" button next to Global Preset
3. Enter preset name: "dark_ambient_drone"
4. Preset saved to `~/Documents/DroneCosmos/Presets/Global/dark_ambient_drone.json`

### Load Preset
1. Select preset from dropdown
2. "Default" restores factory defaults
3. User presets appear below "Default"

## Build Status
✅ Build successful
✅ AU installed to ~/Library/Audio/Plug-Ins/Components/
✅ VST3 installed to ~/Library/Audio/Plug-Ins/VST3/
✅ Standalone app available
✅ Preset folders created on first run

## Notes
- Presets are stored in user Documents folder for easy access
- JSON format allows manual editing if needed
- Preset system independent from DAW's preset system
- Each oscillator can have its own library of settings
- Global presets capture complete plugin state

## Future Enhancements (Phase 4)
- Visual preset browser with categorization
- Import/Export preset packs
- Factory preset library included with plugin
- Preset randomization features
