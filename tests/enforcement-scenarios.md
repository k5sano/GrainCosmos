# Control Flow Enforcement - End-to-End Scenarios

**Date:** 2025-11-12
**Purpose:** Document test scenarios for the 4 control flow enforcement fixes

---

## Scenario 1: Complex Plugin Phase Dispatch (DrumRoulette Case)

**Setup:**
- Plugin: TestComplexPlugin
- Complexity: 5.0 (from plan.md)
- GUI Phases: 5.1, 5.2, 5.3 (in plan.md)

**Test Steps:**
1. Create mock plugin with:
   ```markdown
   # plan.md
   **Complexity Score:** 5.0 / 5.0

   ## Stage 5: GUI Implementation

   ### Phase 5.1: Layout and Basic Controls
   - Main container and control layout
   - Basic parameter controls (sliders, knobs)

   ### Phase 5.2: Advanced UI Elements
   - Visualizers and meters
   - Animation system

   ### Phase 5.3: Polish and Styling
   - Color scheme application
   - Responsive behavior
   ```

2. Invoke plugin-workflow for Stage 5
3. Observe how gui-agent is invoked

**Expected Behavior:**
- Phase detection triggers (complexity ≥3 ✓)
- Phase parsing finds 3 phases
- gui-agent invoked 3 times (once per phase)
- Checkpoint after each invocation
- Decision menu presented between phases
- User MUST confirm before proceeding to next phase
- "implement all phases" prompt NEVER sent

**Pass Criteria:**
✓ Phase detection algorithm executes before invoking gui-agent
✓ 3 separate Task tool invocations (one per phase)
✓ Each invocation receives phase-specific prompt
✓ Checkpoints between phases
✓ No "implement all 3 phases" in any prompt

**Failure Modes:**
✗ gui-agent invoked once with all phases
✗ Phase detection skipped
✗ Auto-proceeds between phases without user confirmation

---

## Scenario 2: Simple Plugin Single-Pass (No Phases)

**Setup:**
- Plugin: TestSimplePlugin
- Complexity: 2.0 (from plan.md)
- No phase markers in plan.md

**Test Steps:**
1. Create mock plugin with:
   ```markdown
   # plan.md
   **Complexity Score:** 2.0 / 5.0

   ## Stage 5: GUI Implementation

   Simple single-screen interface with 3 knobs.
   ```

2. Invoke plugin-workflow for Stage 5
3. Observe execution mode

**Expected Behavior:**
- Phase detection runs (always executes)
- Detects complexity <3
- No phase markers found
- Routes to single-pass implementation
- gui-agent invoked ONCE for entire stage
- Uses single-pass prompt template
- Standard checkpoint after stage completes

**Pass Criteria:**
✓ Phase detection runs (even for simple plugins)
✓ Single gui-agent invocation
✓ Full stage implemented in one pass
✓ No phase iteration loop executed

**Failure Modes:**
✗ Phase detection skipped
✗ Attempts to find phases when complexity <3
✗ Multiple invocations when not needed

---

## Scenario 3: Stage 0 Research Step Sequence

**Setup:**
- Plugin: TestResearchPlugin
- Starting Stage 0 from creative brief

**Test Steps:**
1. Create creative-brief.md with plugin concept
2. Invoke plugin-planning skill for Stage 0
3. Attempt to skip step 3 (Research JUCE DSP Modules)

**Expected Behavior:**
- Step 1 executes: Read creative brief
- Step 2 executes: Identify technical approach
- Step 3 BLOCKED if skipped:
  ```
  ✗ Step 3 incomplete: Insufficient JUCE research

  Found: 0 classes (minimum 2 required)
  Plugin type: effect

  Suggested search: "JUCE dsp effect modules"

  Cannot proceed without identifying DSP components.
  ```
- Steps 4-6 BLOCKED until step 3 completes
- Final verification gate checks all 6 steps before proceeding

**Pass Criteria:**
✓ All 6 steps execute in order
✓ Each step has verification that blocks progression
✓ Step 3 blocks if JUCE research insufficient
✓ Final gate verifies all outputs before creating architecture.md
✓ Cannot skip to architecture.md creation without completing steps 1-6

**Failure Modes:**
✗ Steps executed out of order
✗ Step verification doesn't block progression
✗ architecture.md created with incomplete research
✗ Missing JUCE classes in final contract

---

## Scenario 4: UI Mockup Phase A→B Gate

**Setup:**
- Plugin: TestMockupPlugin
- UI design iteration workflow

**Test Steps:**
1. Invoke ui-mockup skill
2. Complete Phase A (generate YAML + test HTML)
3. Receive decision menu at Phase 5.5
4. Select option 1 (Iterate) instead of option 2 (Finalize)
5. Attempt to access Phase B files (PluginEditor.h, etc.)

**Expected Behavior:**
- Phase A completes: v1-ui.yaml, v1-ui-test.html generated
- Decision menu presented:
  ```
  ✓ Mockup v1 design created (2 files)

  What would you like to do?

  1. Iterate - Refine design, adjust layout
  2. Finalize - Validate alignment and complete mockup
  3. Save as template - Add to aesthetic library for reuse
  4. Other

  Choose (1-4): _
  ```
- User selects option 1 (Iterate)
- Collects feedback, increments to v2
- Generates v2-ui.yaml, v2-ui-test.html
- Phase B files (v1-PluginEditor.h, v1-ui.html, etc.) DO NOT EXIST
- Finalization marker `finalized: true` ABSENT from v1-ui.yaml

**Then:**
- User returns to Phase 5.5 menu
- Selects option 2 (Finalize) on v2
- Phase B guard verification passes
- Generates all 5 implementation files:
  - v2-ui.html (production)
  - v2-PluginEditor.h
  - v2-PluginEditor.cpp
  - v2-CMakeLists.txt
  - v2-integration-checklist.md
- Finalization marker written to v2-ui.yaml

**Pass Criteria:**
✓ Phase A files generated without Phase B
✓ Decision menu blocks Phase B until option 2 selected
✓ Iteration creates new version (v2) without Phase B files
✓ Phase B guard verifies finalization marker before proceeding
✓ All 5 Phase B files generated only after finalization

**Failure Modes:**
✗ Phase B files generated without finalization
✗ Decision menu skipped or auto-proceeds
✗ Finalization marker missing from YAML
✗ Phase B guard doesn't block without marker

---

## Scenario 5: Research Handoff Detection (deep-research → plugin-improve)

**Setup:**
- Simulate conversation where deep-research skill was invoked
- deep-research completed Tier 3 investigation
- Findings in conversation history

**Test Steps:**
1. Create conversation history with deep-research markers:
   ```
   Previous messages:
   - User: "Why does TapeAge crash on plugin reload?"
   - Assistant: "Invoking deep-research skill (Tier 3)..."
   - deep-research: "Investigation complete. Root Cause: Member initialization order."
   - deep-research: "Recommended Solution: Reorder members (relays → webView → attachments)"
   ```

2. Invoke plugin-improve skill for TapeAge
3. Phase 0.45 (Research Detection) executes

**Expected Behavior:**
- Phase 0.45 scans conversation history
- Detects markers:
  - "deep-research" skill invocation ✓
  - "Root Cause:" section ✓
  - "Recommended Solution:" section ✓
  - "Investigation complete" ✓
- Extracts findings:
  ```
  ✓ Research handoff detected from deep-research skill

  Investigation: Tier 3 (Deep investigation with extended thinking)
  Problem: TapeAge crashes on plugin reload
  Root Cause: Member initialization order (webView before relays)
  Recommended Solution: Reorder members (relays → webView → attachments)

  Using existing research findings (skipping Phase 0.5 investigation).
  ```
- Presents implementation approval menu
- User selects option 1 (Proceed with recommended solution)
- Skips Phase 0.5 investigation
- Proceeds directly to Phase 0.9 (Backup Verification)

**Pass Criteria:**
✓ Phase 0.45 always executes (MANDATORY)
✓ Conversation history scanned for research markers
✓ Findings extracted when markers present
✓ Phase 0.5 skipped when research detected
✓ Implementation uses extracted findings
✓ CHANGELOG references research findings

**Failure Modes:**
✗ Phase 0.45 skipped
✗ Research markers not detected
✗ Phase 0.5 runs despite research existing
✗ Duplicate investigation performed
✗ Expensive research context lost

---

## Scenario 6: Missing Research (False Negative Prevention)

**Setup:**
- No deep-research invocation in conversation history
- Fresh conversation starting with /improve command

**Test Steps:**
1. User: "/improve TapeAge - add bypass parameter"
2. Invoke plugin-improve skill
3. Phase 0.45 (Research Detection) executes

**Expected Behavior:**
- Phase 0.45 scans conversation history
- No research markers found
- Logs detection completion:
  ```
  No research handoff detected in conversation history.
  Proceeding to Phase 0.5 (Investigation).
  ```
- Continues to Phase 0.5 investigation
- Auto-tiered investigation runs (Tier 1/2/3 based on complexity)
- Investigation findings used for implementation

**Pass Criteria:**
✓ Phase 0.45 executes (not skipped due to lazy evaluation)
✓ Conversation history scanned (even if empty result)
✓ Phase 0.5 runs normally when no research detected
✓ No false positives (doesn't detect research when none exists)
✓ Detection result logged in .improve-context.yaml

**Failure Modes:**
✗ Phase 0.45 skipped assuming no research
✗ False positive detection
✗ Phase 0.5 skipped incorrectly
✗ Detection state not tracked

---

## Edge Case Tests

### Edge Case 1: Malformed plan.md (plugin-workflow)

**Setup:**
- Plugin has complexity 4.0
- Phase markers present but malformed:
  ```markdown
  Phase 5.1: Layout  # Missing ### header
  ```

**Expected:**
- Phase detection runs
- Phase parsing fails gracefully
- Falls back to single-pass implementation
- Logs warning about malformed markers

### Edge Case 2: Missing Contracts (plugin-planning)

**Setup:**
- creative-brief.md missing
- Attempt to run Stage 0

**Expected:**
- Step 1 verification fails
- BLOCKS with clear error:
  ```
  ✗ BLOCKED: creative-brief.md not found
  ```
- Exit skill, wait for user to create brief

### Edge Case 3: Partial Research Findings (plugin-improve)

**Setup:**
- Conversation has "deep-research" marker
- But missing "Root Cause:" section

**Expected:**
- Phase 0.45 detects research invocation
- Extraction finds partial data
- Presents what was found
- Offers to re-run investigation (Phase 0.5) to complete findings

### Edge Case 4: Finalization Marker Written But YAML Deleted (ui-mockup)

**Setup:**
- finalization marker written to v1-ui.yaml
- User manually deletes v1-ui.yaml
- Attempt to proceed to Phase B

**Expected:**
- Phase B guard runs
- File existence check fails
- BLOCKS with error:
  ```
  ✗ ERROR: Phase A incomplete (missing YAML or test HTML)
  ```
- Cannot proceed without both Phase A files

---

## Regression Test Matrix

| Scenario | plugin-workflow | plugin-planning | ui-mockup | plugin-improve |
|----------|----------------|-----------------|-----------|----------------|
| Scenario 1 | ✓ Tested | - | - | - |
| Scenario 2 | ✓ Tested | - | - | - |
| Scenario 3 | - | ✓ Tested | - | - |
| Scenario 4 | - | - | ✓ Tested | - |
| Scenario 5 | - | - | - | ✓ Tested |
| Scenario 6 | - | - | - | ✓ Tested |
| Edge Case 1 | ✓ Tested | - | - | - |
| Edge Case 2 | - | ✓ Tested | - | - |
| Edge Case 3 | - | - | - | ✓ Tested |
| Edge Case 4 | - | - | ✓ Tested | - |

---

## Test Execution Notes

**How to run these tests:**

1. **Unit tests** (Passes 1-2): Automated grep/sed verification (completed)
2. **Scenario tests** (Pass 3): Manual execution with mock data
3. **Integration tests**: Run full workflows with test plugins

**Test data location:**
- Mock plugins: `tests/fixtures/plugins/`
- Mock conversation history: `tests/fixtures/conversations/`
- Expected outputs: `tests/fixtures/expected/`

**Automation potential:**
- Static analysis: ✅ Fully automated
- Logic analysis: ✅ Fully automated
- Scenario simulation: ⚠️ Manual (requires full skill execution)
- Regression validation: ⚠️ Manual (requires comparison with baselines)
