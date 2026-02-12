# Stage 3 (GUI) Integration Checklist - v3

**Plugin:** GrainCosmos
**Mockup Version:** v3
**Generated:** 2026-02-12

## 1. Copy UI Files

- [ ] Copy v3-ui.html to Source/ui/public/index.html
- [ ] Copy JUCE frontend library to Source/ui/public/js/juce/index.js
  - Located in JUCE framework or reference plugin (GainKnob)
  - Required: index.js and check_native_interop.js
- [ ] Verify no viewport units in CSS (100vh, 100vw, 100dvh, 100svh)
- [ ] Verify user-select: none present in body styles

## 2. Update PluginEditor Files

- [ ] Create Source/PluginEditor.h if not exists
- [ ] Replace content with v3-PluginEditor-TEMPLATE.h content
- [ ] Verify member order: relays → webView → attachments
- [ ] Update class name to GrainCosmosAudioProcessorEditor
- [ ] Create Source/PluginEditor.cpp if not exists
- [ ] Replace content with v3-PluginEditor-TEMPLATE.cpp content
- [ ] Verify initialization order matches declaration order
- [ ] Verify all 10 parameters have matching relays and attachments

## 3. Update CMakeLists.txt

- [ ] Append v3-CMakeLists-SNIPPET.txt to CMakeLists.txt
- [ ] Verify juce_add_binary_data includes all UI files (index.html, index.js)
- [ ] Verify JUCE_WEB_BROWSER=1 definition present
- [ ] Verify juce::juce_gui_extra linked
- [ ] Verify NEEDS_WEB_BROWSER TRUE present in juce_add_plugin()

## 4. Build and Test (Debug)

- [ ] Build succeeds without warnings
- [ ] Standalone loads WebView (not blank)
- [ ] Right-click → Inspect works (opens DevTools)
- [ ] Console shows no JavaScript errors
- [ ] window.__JUCE__ object exists in console
- [ ] All 8 knobs rotate smoothly when dragged
- [ ] All 2 toggle switches toggle on click

## 5. Build and Test (Release)

- [ ] Release build succeeds
- [ ] No crashes on plugin reload (test 10 times)
- [ ] Tests member order correctness (attachments destroyed before webView)

## 6. Test Parameter Binding

- [ ] delay_time parameter syncs UI ↔ APVTS
- [ ] grain_size parameter syncs UI ↔ APVTS
- [ ] envelope_shape parameter syncs UI ↔ APVTS
- [ ] distortion_amount parameter syncs UI ↔ APVTS
- [ ] feedback parameter syncs UI ↔ APVTS
- [ ] chaos parameter syncs UI ↔ APVTS
- [ ] character parameter syncs UI ↔ APVTS
- [ ] mix parameter syncs UI ↔ APVTS
- [ ] freeze toggle syncs UI ↔ APVTS
- [ ] tempo_sync toggle syncs UI ↔ APVTS
- [ ] Automation updates UI in real-time
- [ ] Preset recall updates UI
- [ ] Values persist after DAW reload

## 7. WebView-Specific Validation

- [ ] No viewport units in CSS (100vh, 100vw, 100dvh, 100svh)
- [ ] Native feel CSS present (user-select: none)
- [ ] Resource provider returns all files (no 404s in console)
- [ ] Correct MIME types for all resources (text/html, text/javascript)
- [ ] Context menu disabled (right-click shows DevTools in standalone)

## Parameter List (from v3-ui.yaml)

### Slider Parameters (8)

| Parameter ID | Type | Range | Default | UI Control | Relay Type |
|--------------|------|-------|---------|-------------|-------------|
| delay_time | Float | 0.0 - 2.0s | 0.5s | DELAY knob | WebSliderRelay |
| grain_size | Float | 10.0 - 500.0ms | 100.0ms | GRAIN knob | WebSliderRelay |
| envelope_shape | Float | 0.0 - 1.0 | 0.5 | ENVELOPE knob | WebSliderRelay |
| distortion_amount | Float | 0.0 - 100.0% | 0.0% | DISTORTION knob | WebSliderRelay |
| feedback | Float | 0.0 - 95.0% | 40.0% | FEEDBACK knob | WebSliderRelay |
| chaos | Float | 0.0 - 100.0% | 20.0% | CHAOS knob | WebSliderRelay |
| character | Float | 0.0 - 100.0% | 50.0% | CHARACTER knob (glow) | WebSliderRelay |
| mix | Float | 0.0 - 100.0% | 30.0% | MIX knob | WebSliderRelay |

### Toggle Parameters (2)

| Parameter ID | Type | Range | Default | UI Control | Relay Type |
|--------------|------|-------|---------|-------------|-------------|
| freeze | Bool | On/Off | Off (false) | FREEZE toggle | WebToggleButtonRelay |
| tempo_sync | Bool | On/Off | Off (false) | SYNC toggle | WebToggleButtonRelay |

**Total:** 10 parameters (8 sliders + 2 toggles)

## Design Specifications (v3)

**Window Dimensions:** 800×500px (fixed, not resizable)

**Visual Style:**
- Chassis: Olive-gray vintage (#4a4a3a to #2a2a1a) with aging effects
- Knobs: Retro silver measurement instrument style
  - Tapered conical shape (15-20 degree angle)
  - Knurled/gripping texture (45-degree cross-hatch pattern)
  - Chrome/metallic silver finish (#c0c0c0 to #e8e8e8 to #a0a0a0)
  - Chrome white highlights (#ffffff)
  - Dark gray shadows (#404040)
- Indicators: Purple gradient (#9b4dca to #b47eff)
- Text: Gold color (#ffd89b) with wide letter-spacing
- Logo/Title: Japanese brush pen / handwritten marker font
- Aging effects: Scratches, rust patches, worn paint, chipped edges, grease smudges

**Layout:**
- Header (60px): Plugin title
- Top Row (y: 80): DELAY, GRAIN, ENVELOPE, DISTORTION
- Bottom Row (y: 240): FEEDBACK, CHAOS, CHARACTER, MIX
- Footer (y: 410): FREEZE toggle (x: 640), SYNC toggle (x: 60)

**Special Effects:**
- CHARACTER knob has purple glow effect for focal differentiation

## Member Order Verification

**Declaration Order (PluginEditor.h):**
1. Relays (8 slider + 2 toggle = 10 relays)
2. WebView (1 instance)
3. Attachments (8 slider + 2 toggle = 10 attachments)

**Verification:**
- [ ] Relay count: 10
- [ ] Attachment count: 10
- [ ] Relay count matches attachment count ✓
- [ ] Relays declared before webView
- [ ] webView declared before attachments
- [ ] Initialization order in .cpp matches declaration order in .h

## Critical JUCE 8 Patterns Applied

✓ Member order: relays → webView → attachments (prevents release crashes)
✓ Resource provider: Explicit URL mapping (not generic loop)
✓ WebView options: .withOptionsFrom(*relay) for all 10 parameters
✓ Parameter attachments: 3-parameter constructor (param, relay, undoManager)
✓ ES6 modules: type="module" in script tags
✓ Boolean parameters: getToggleState() API (not getSliderState())
✓ Knob interaction: Relative drag (frame-delta pattern)
✓ valueChangedEvent: No callback parameters (read getNormalisedValue())
✓ No viewport units: html, body { height: 100%; }
✓ user-select: none for native application feel

## Next Steps

After integration complete:
- [ ] Test in standalone mode
- [ ] Test in DAW (Ableton Live, Logic Pro, FL Studio)
- [ ] Verify automation recording/playback
- [ ] Verify preset save/load
- [ ] Verify undo/redo (if implemented)
- [ ] Performance profiling (CPU usage)
- [ ] Create preset library
