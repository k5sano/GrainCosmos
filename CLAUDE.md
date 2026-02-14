# Plugin Freedom System - Development Guide

## Plugin Installation & Testing

After building a plugin, copy to system folders:

```bash
# AU
cp -R build/GrainCosmos_artefacts/Release/AU/GrainCosmos.component ~/Library/Audio/Plug-Ins/Components/

# VST3
cp -R build/GrainCosmos_artefacts/Release/VST3/GrainCosmos.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**IMPORTANT:** Ableton Live caches plugin information. **Always restart Ableton Live** after updating a plugin, otherwise changes won't be detected.

## CMake Automation

Add to CMakeLists.txt for automatic copy after build:
```cmake
COPY_PLUGIN_AFTER_BUILD TRUE
```

## GrainCosmos - WebView Status

**Current Status (2026-02-13):**
- `JUCE_WEB_BROWSER=0` → Plugin works (no crash)
- `JUCE_WEB_BROWSER=1` → Testing...

Root cause appears to be JUCE version incompatibility with WebView implementation.

## JUCE Installation

Always use git clone for JUCE:
```bash
cd /tmp && git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git
mv JUCE /Applications/
```
