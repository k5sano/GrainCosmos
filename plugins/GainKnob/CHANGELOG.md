# Changelog

All notable changes to GainKnob will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-11-10

### Added
- Initial release of GainKnob utility plugin
- Single GAIN parameter (-60 to 0 dB range)
- Clean, minimal WebView-based UI with centered rotary knob
- dB value display with -∞ representation at minimum
- Logarithmic gain conversion (dB to linear) using JUCE Decibels class
- Complete silence at -60 dB (gain multiplier = 0.0)
- Unity gain at 0 dB (gain multiplier = 1.0)
- Factory presets:
  - Unity (0 dB - no attenuation)
  - Subtle Cut (-3 dB)
  - Half Volume (-6 dB)
  - Quiet (-12 dB)
  - Silence (-60 dB)
- VST3 and AU plugin format support
- Standalone application mode
- State save/load functionality via APVTS
- Real-time safe parameter modulation
- Denormal protection in audio processing

### Technical Details
- Built with JUCE 8
- WebView UI integration using juce-frontend library
- AudioProcessorValueTreeState for parameter management
- Time-domain gain multiplication
- Zero-latency processing
- Minimal CPU usage

[1.0.0]: https://github.com/user/plugin-freedom-system/releases/tag/GainKnob-v1.0.0
