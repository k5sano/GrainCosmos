# RadioMusic - Implementation Plan

**Date:** 2025-01-10
**Complexity Score:** 5.0 (Very Complex)
**Strategy:** Phase-based implementation

## Complexity Factors

- **Parameters:** 4 (0.8 points)
- **Algorithms:** 6 (6 points)
  1. AudioFormatManager (file loading)
  2. AudioFormatReaderSource (streaming playback)
  3. Resampler (variable speed)
  4. File queueing system (custom)
  5. MIDI re-trigger logic (custom)
  6. Waveform downsampling (custom)
- **Features:** MIDI control (1 point)

**Total:** 0.8 + 6 + 1 = 7.8 → Capped at 5.0

## Stages

- Stage 0: Research ✓
- Stage 1: Planning ← Current
- Stage 2: Foundation
- Stage 3: Shell
- Stage 4: DSP (Phased)
- Stage 5: GUI (Phased)
- Stage 6: Validation

## Stage 4: DSP Phases

### Phase 4.1: File Loading & Basic Playback

**Goal:** Load folder of audio files and implement basic continuous playback

**Implementation:**
- File manager: scan folder, cache file list
- AudioFormatManager setup with `registerBasicFormats()`
- AudioFormatReaderSource for streaming current file
- Basic looping (restart from beginning when file ends)
- Station parameter maps to file index

**Test Criteria:**
- [ ] Folder can be loaded via file chooser
- [ ] Files are listed and sorted alphabetically
- [ ] Station parameter changes current file
- [ ] Audio plays continuously and loops
- [ ] No crashes with 100+ files in folder
- [ ] Handles missing/corrupt files gracefully

**Duration:** 30 min

### Phase 4.2: MIDI Triggering & Start Position

**Goal:** Implement MIDI note-on re-triggering and Start position control

**Implementation:**
- Parse MidiBuffer in processBlock
- Detect note-on events (`message.isNoteOn()`)
- Reset playback position to Start parameter (percentage → sample offset)
- Maintain separate playback position vs start position
- Transport start/stop behavior (always resume from Start)

**Test Criteria:**
- [ ] MIDI note-on restarts playback from Start position
- [ ] Start parameter (0-100%) correctly maps to file position
- [ ] Playback continues seamlessly without MIDI input
- [ ] Transport stop/start resets to Start position
- [ ] No clicks or artifacts on re-trigger
- [ ] Sample-accurate triggering

**Duration:** 25 min

### Phase 4.3: Variable Speed/Pitch

**Goal:** Implement tape-style speed/pitch control

**Implementation:**
- Integrate Oboe MultiChannelResampler
- Map speed_pitch parameter (-1.0 to +1.0) to multiplier via `pow(2, value)`
- Quality setting: Medium (balance CPU vs quality)
- Smooth parameter changes to avoid clicks
- Handle resampling at various playback rates

**Test Criteria:**
- [ ] Speed/Pitch parameter affects playback rate
- [ ] -1.0 = 0.5x (half speed, octave down)
- [ ] 0.0 = 1.0x (normal speed)
- [ ] +1.0 = 2.0x (double speed, octave up)
- [ ] No clicks when changing speed
- [ ] Looping works correctly at all speeds
- [ ] CPU usage acceptable

**Duration:** 20 min

### Phase 4.4: Station Queueing & Volume

**Goal:** Implement station change queueing logic and output gain

**Implementation:**
- Queue system: Station changes don't interrupt playback immediately
- If MIDI active: apply new file on next note-on
- If free-running: apply immediately
- Volume parameter: convert dB to linear gain (`pow(10, dB/20)`)
- Apply gain as final stage

**Test Criteria:**
- [ ] Station changes queue next file correctly
- [ ] MIDI triggers apply queued file
- [ ] Free-running mode changes files immediately
- [ ] Volume parameter controls output level
- [ ] -60dB = silence, 0dB = unity, +12dB = 4x
- [ ] No clicks when changing volume
- [ ] Parameter smoothing works

**Duration:** 20 min

## Stage 5: GUI Phases

### Phase 5.1: WebView Setup & Basic Layout

**Goal:** Initialize WebView UI with parameter bindings

**Implementation:**
- Copy v3-ui.html to Source/ui/ui.html
- Set up WebBrowserComponent in PluginEditor
- Load HTML from BinaryData
- Implement JavaScript ↔ C++ message bridge
- Bind 4 knobs to parameters (Station, Start, Speed, Volume)
- Parameter updates flow both directions

**Test Criteria:**
- [ ] WebView loads and displays correctly (700x500)
- [ ] All 4 knobs appear and are interactive
- [ ] Knob drag updates parameters in C++
- [ ] Parameter changes in DAW update UI knobs
- [ ] Speed and Volume knobs snap to center
- [ ] No WebView errors in console

**Duration:** 25 min

### Phase 5.2: Waveform Display & File Info

**Goal:** Implement real-time waveform visualization and file browser

**Implementation:**
- Background thread for waveform generation
- Downsample current file to ~800 points
- Send waveform data to UI via JavaScript message
- Canvas drawing with Start position masking (grey vs orange)
- Real-time playback cursor (60fps updates)
- File info display (index/total: filename)
- Browse button triggers folder chooser

**Test Criteria:**
- [ ] Waveform displays when file loads
- [ ] Start position masks waveform (dark grey before, orange after)
- [ ] Playback cursor animates smoothly
- [ ] Waveform updates when Station changes
- [ ] Browse button opens folder chooser
- [ ] File info shows correct data
- [ ] No UI lag or performance issues

**Duration:** 30 min

## Estimated Duration

**Total:** ~2.5 hours

- Stage 2: 5 min (Foundation)
- Stage 3: 5 min (Shell)
- Stage 4: 95 min (DSP - phased)
  - 4.1: 30 min (File loading & playback)
  - 4.2: 25 min (MIDI triggering)
  - 4.3: 20 min (Variable speed)
  - 4.4: 20 min (Queueing & volume)
- Stage 5: 55 min (GUI - phased)
  - 5.1: 25 min (WebView & bindings)
  - 5.2: 30 min (Waveform & browser)
- Stage 6: 20 min (Validation)

## Implementation Notes

**Phase Boundaries:**
- Each phase is independently testable
- Git commit after each phase completion
- Test criteria must pass before proceeding
- If tests fail: fix before continuing

**Risk Areas:**
- File I/O performance with large folders
- Waveform generation without blocking audio thread
- Seamless looping at variable speeds
- Thread safety (UI updates vs audio thread)

**Optimization Opportunities:**
- Cache waveform data for faster Station changes
- Use std::atomic for thread-safe state
- Throttle UI updates (60fps max)
- Lazy load only visible waveform data
