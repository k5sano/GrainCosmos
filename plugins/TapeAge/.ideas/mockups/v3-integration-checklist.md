# TapeAge WebView UI Integration Checklist

**Version:** v3 (Vintage Hardware Aesthetic)
**Generated:** 2025-11-10
**Purpose:** Step-by-step guide to integrate WebView UI into TapeAge plugin during Stage 5 (GUI)

---

## Prerequisites

- [ ] Stage 4 (DSP) completed - plugin processes audio correctly
- [ ] Plugin builds and loads in DAW
- [ ] Parameters defined in PluginProcessor (drive, age, mix)
- [ ] JUCE project configured with CMake

---

## Step 1: Organize Files

### 1.1 Create UI Directory

```bash
cd plugins/TapeAge
mkdir -p Source/ui
```

### 1.2 Copy HTML File

```bash
cp .ideas/mockups/v3-ui.html Source/ui/v3-ui.html
```

**Verify:** `Source/ui/v3-ui.html` exists and contains production HTML

---

## Step 2: Update CMakeLists.txt

### 2.1 Add Binary Data Section

Open `plugins/TapeAge/CMakeLists.txt` and add:

```cmake
juce_add_binary_data(TapeAgeAssets
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/ui/v3-ui.html
)
```

**Location:** Before `juce_add_plugin()` call

### 2.2 Enable WebView Support

In `juce_add_plugin()`, add:

```cmake
NEEDS_WEB_BROWSER TRUE
```

### 2.3 Link Binary Data

In `target_link_libraries()`, add:

```cmake
target_link_libraries(TapeAge
    PRIVATE
        TapeAgeAssets  # Add this line
        juce::juce_audio_utils
        juce::juce_dsp
    # ... rest of links
)
```

### 2.4 Platform-Specific Links (macOS)

```cmake
if(APPLE)
    target_link_libraries(TapeAge PRIVATE "-framework WebKit")
endif()
```

**Verify:** CMake configuration runs without errors

---

## Step 3: Update PluginEditor.h

### 3.1 Copy Boilerplate

```bash
cp .ideas/mockups/v3-PluginEditor.h Source/PluginEditor.h
```

**Or manually add:**

```cpp
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TapeAgeAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    TapeAgeAudioProcessorEditor(TapeAgeAudioProcessor&);
    ~TapeAgeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void handleWebViewMessage(const juce::var& message);
    void updateWebViewParameter(const juce::String& paramID, float value);
    void updateVUMeter(float dbLevel);

    TapeAgeAudioProcessor& audioProcessor;
    std::unique_ptr<juce::WebBrowserComponent> webView;

    float currentVULevel = -60.0f;
    float peakVULevel = -60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapeAgeAudioProcessorEditor)
};
```

**Verify:** File compiles without syntax errors

---

## Step 4: Update PluginEditor.cpp

### 4.1 Copy Implementation

```bash
cp .ideas/mockups/v3-PluginEditor.cpp Source/PluginEditor.cpp
```

**Or manually implement:**

Key sections to add:
- Constructor: Load HTML from BinaryData, create WebView
- `timerCallback()`: Update VU meter at 60 Hz
- `handleWebViewMessage()`: Handle parameter changes from JavaScript
- `updateWebViewParameter()`: Send parameter updates to JavaScript
- `updateVUMeter()`: Send VU meter levels to JavaScript

**Verify:** File compiles without errors

---

## Step 5: Update PluginProcessor

### 5.1 Add Output Level Tracking

In `PluginProcessor.h`, add:

```cpp
private:
    std::atomic<float> outputLevel { -60.0f };

public:
    float getOutputLevel() const { return outputLevel.load(); }
```

### 5.2 Calculate Output Level in processBlock()

In `PluginProcessor.cpp`, add after DSP processing:

```cpp
void TapeAgeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // ... existing DSP code ...

    // Calculate output level for VU meter
    float maxLevel = 0.0f;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float channelMax = buffer.getMagnitude(channel, 0, buffer.getNumSamples());
        maxLevel = std::max(maxLevel, channelMax);
    }

    float dbLevel = juce::Decibels::gainToDecibels(maxLevel, -60.0f);
    outputLevel.store(dbLevel);
}
```

**Verify:** Plugin builds successfully

---

## Step 6: Build and Test

### 6.1 Clean Build

```bash
cd build
cmake --build . --config Release --clean-first
```

### 6.2 Test in Standalone

```bash
open TapeAge.app  # macOS
# or
./TapeAge  # Linux
```

**Expected:**
- Window opens at 500x450px
- Deep brown radial gradient background with subtle scan lines
- VU meter with dark recessed panel and warm gold needle
- "TAPE AGE" title in Helvetica Neue (light weight, wide spacing)
- 3 dark bronze knobs with gold indicator lines
- Knobs rotate smoothly - lighting stays fixed, only indicator rotates
- VU meter needle animates with audio input

### 6.3 Test in DAW

Load plugin in your DAW (Logic, Ableton, etc.)

**Test:**
- [ ] UI loads correctly with vintage hardware aesthetic
- [ ] Knobs respond to vertical mouse drag
- [ ] Lighting/shadows stay fixed as knobs rotate (realistic behavior)
- [ ] Parameter automation works
- [ ] VU meter updates in real-time with warm gold glow
- [ ] UI survives plugin state save/restore
- [ ] No crashes when opening/closing editor

---

## Step 7: Troubleshooting

### Issue: WebView shows blank white screen

**Cause:** HTML not loaded or JavaScript error

**Fix:**
1. Check BinaryData includes v3-ui.html:
   ```bash
   grep "v3ui_html" build/JuceLibraryCode/BinaryData.h
   ```
2. Check browser console (if available)
3. Verify HTML syntax in v3-ui.html

### Issue: Knobs don't respond to drag

**Cause:** JavaScript event handlers not attached

**Fix:**
1. Check JavaScript console for errors
2. Verify `window.addEventListener('DOMContentLoaded', ...)` runs
3. Ensure `data-param` attributes match parameter IDs

### Issue: VU meter doesn't animate

**Cause:** Timer not running or `getOutputLevel()` not implemented

**Fix:**
1. Verify `startTimerHz(60)` called in constructor
2. Check `getOutputLevel()` returns valid dB values (-60 to +3)
3. Ensure `updateVUMeter()` calls JavaScript correctly

### Issue: Knob lighting rotates with indicator (unrealistic)

**Cause:** Lighting effects placed on rotating layer

**Fix:**
1. Verify HTML structure has separate `knob-body` (static) and `knob-rotatable` layers
2. Check that gradients/shadows are on `knob-body` and `knob-body::before`
3. Only `knob-rotatable::after` (indicator line) should rotate
4. See: `docs/troubleshooting/rotating-lighting-on-knobs.md` for detailed solution

### Issue: Parameters don't sync between UI and DAW automation

**Cause:** Message handlers not connected

**Fix:**
1. Implement `handleWebViewMessage()` to receive UI changes
2. Implement `updateWebViewParameter()` to send host changes
3. Verify parameter IDs match between C++ and JavaScript

### Issue: Plugin crashes on close

**Cause:** WebView not properly destroyed

**Fix:**
1. Ensure `webView.reset()` or destructor runs
2. Call `stopTimer()` in destructor
3. Check for dangling pointers

---

## Step 8: Final Validation

### 8.1 Visual Check

- [ ] Window size exactly 500x450px
- [ ] Deep brown radial gradient background (#4a3a2a to #2a1a0a)
- [ ] Subtle horizontal scan lines visible (2px spacing)
- [ ] Heavy vignette around edges
- [ ] VU meter with dark recessed panel (#1a0a00) and bronze frame (#3a2a1a)
- [ ] "TAPE AGE" title in Helvetica Neue, light weight (300), wide spacing (0.4em)
- [ ] 3 dark bronze knobs with brushed metal centers and gold indicator lines
- [ ] Labels in bronze (#c49564), medium weight (500), spacing (0.3em)
- [ ] Warm gold needle glow effect on VU meter
- [ ] Professional vintage hardware aesthetic throughout

### 8.2 Functional Check

- [ ] All knobs rotate smoothly (-135° to +135°)
- [ ] Lighting and shadows stay fixed - only indicator rotates (realistic)
- [ ] Knob interaction feels responsive (0.05s transition, 1.005/0.995 scale)
- [ ] VU meter needle moves with audio
- [ ] Red zone indicators visible above 0 dB
- [ ] Parameters persist across plugin instances
- [ ] No memory leaks (test with Activity Monitor)

### 8.3 Cross-Platform Check (if applicable)

- [ ] macOS: Tested in Logic Pro / Ableton
- [ ] Windows: Tested in Reaper / FL Studio
- [ ] Linux: Tested in Ardour / Bitwig

---

## Step 9: Mark Stage Complete

Once all checks pass:

```bash
# Update PLUGINS.md
# Update .continue-here.md
# Commit changes
git add plugins/TapeAge/Source/
git commit -m "feat(TapeAge): Stage 5 - WebView GUI complete (v3 vintage hardware)"
```

---

## Reference Files

**Mockup files:**
- `.ideas/mockups/v3-ui.yaml` - Design specification (vintage hardware)
- `.ideas/mockups/v3-ui-test.html` - Browser test version
- `.ideas/mockups/v3-ui.html` - Production HTML (copy to Source/ui/)
- `.ideas/mockups/v3-PluginEditor.h` - C++ header boilerplate
- `.ideas/mockups/v3-PluginEditor.cpp` - C++ implementation boilerplate
- `.ideas/mockups/v3-CMakeLists.txt` - Build configuration snippets
- `.ideas/mockups/v3-integration-checklist.md` - This file

**Contract files:**
- `.ideas/creative-brief.md` - Original plugin vision
- `.ideas/parameter-spec.md` - Parameter definitions (immutable)
- `.ideas/architecture.md` - DSP architecture
- `.ideas/plan.md` - Implementation plan

**Documentation:**
- `docs/troubleshooting/rotating-lighting-on-knobs.md` - Fixed lighting solution

---

## Design Notes (v3 Specific)

**Visual Identity:**
- Deep brown professional studio hardware aesthetic
- Heavy shadows and recessed surfaces
- Warm gold accents on active elements
- Subtle scan lines for retro hardware feel

**Typography:**
- Helvetica Neue (classic, clean, geometric sans-serif)
- Light weights for elegance (300 title, 500 labels)
- Wide letter spacing for professional appearance

**Lighting System:**
- Static lighting from top-left (simulates room overhead light)
- Shadows and highlights stay fixed on knob surface
- Only position indicator rotates (physically realistic)
- See troubleshooting doc for implementation details

**Color Palette:**
- Background gradient: #4a3a2a → #2a1a0a (warm to deep brown)
- Metal: #2f2015 (dark bronze base), #3a2a1a (primary)
- Accents: #d4a574 (warm gold), #c49564 (bronze)
- Earth tones: #8a6a4a, #6a5a4a (muted secondaries)

---

## Success Criteria

Stage 5 (GUI) is complete when:

✅ Plugin loads with WebView UI in standalone and DAW
✅ All 3 parameters controllable via UI
✅ VU meter animates in real-time with warm gold glow
✅ UI matches v3 mockup design exactly (vintage hardware aesthetic)
✅ Knob lighting stays fixed - only indicator rotates (realistic behavior)
✅ No crashes, memory leaks, or UI glitches
✅ Parameters sync with DAW automation
✅ Ready for Stage 6 (Validation)

---

**Next Stage:** Stage 6 - Validation (Pluginval, presets, final testing)
