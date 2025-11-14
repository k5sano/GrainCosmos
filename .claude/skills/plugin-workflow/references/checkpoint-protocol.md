# Checkpoint Protocol

## Overview

After EVERY subagent return (whether full stage or phase completion), orchestrator executes this 6-step checkpoint sequence.

## Required Steps (in order)

### Step 1: Verify State Update

```javascript
verifyStateUpdate(pluginName, currentStage, result)
```

Check if subagent updated state files:
- Read `result.stateUpdated` field from JSON report
- If true: Verify `.continue-here.md` stage field matches currentStage
- If true: Verify `PLUGINS.md` status updated
- If false or missing: Proceed to fallback

### Step 2: Fallback State Update

```javascript
IF verifyStateUpdate() returned false:
  Log: "Subagent did not update state (stateUpdated=false), orchestrator handling"
  updateHandoff(pluginName, currentStage, result.completed, result.nextSteps)
  updatePluginStatus(pluginName, `🚧 Stage ${currentStage}`)
  updatePluginTimeline(pluginName, currentStage, result.description)
  validateRegistryConsistency(pluginName)
  Log: "State updated by orchestrator (fallback)"
ELSE:
  Log: "State updated by subagent, verified"
```

### Step 3: Invoke Validation (Stages 1-3 only)

```javascript
IF currentStage in [1, 2, 3]:
  Log: "Invoking validation-agent for semantic review..."
  validationResult = invokeValidationAgent(pluginName, currentStage)
  validation = parseValidationReport(validationResult)

  IF validation.status == "FAIL" AND validation.continue_to_next_stage == false:
    presentValidationFailureMenu(validation)
    BLOCK progression until user resolves issues
  ELSE:
    Log: `✓ Validation ${validation.status}: ${validation.recommendation}`
ELSE:
  Log: "Validation skipped for stage ${currentStage}"
```

### Step 4: Commit Stage

```javascript
commitStage(pluginName, currentStage, result.description)
```

Auto-commit all changes (code + state files).
Verify git commit succeeded (check exit code).

### Step 5: Verify Checkpoint

```javascript
verifyCheckpoint(pluginName, currentStage)
```

Validate all checkpoint steps completed successfully.
BLOCK if any step failed - present retry menu before continuing.

### Step 6: Handle Checkpoint (Mode-Aware)

```javascript
handleCheckpoint({ stage, completionStatement, pluginName, workflowMode, hasErrors })

IF workflowMode == "express" AND currentStage < 4 AND hasErrors == false:
  # Express mode: Auto-progress to next stage
  displayProgressMessage(currentStage, nextStage)
  Log: "Express mode: Auto-progressing to Stage ${nextStage}"
  # No menu, no wait, continue immediately
ELSE:
  # Manual mode OR final stage OR error occurred
  presentDecisionMenu({ stage, completionStatement, pluginName })
  WAIT for user response (blocking)
```

## Checkpoint Verification

After steps 1-5 complete, verify all succeeded:

```
CHECKPOINT VERIFICATION:
✓ Step 1: State update verified (subagent updated: true)
✓ Step 2: Fallback skipped (not needed) OR Fallback completed
✓ Step 3: Validation passed (or skipped for Stage 4)
✓ Step 4: Git commit [commit-hash]
✓ Step 5: All checkpoint steps validated

IF all verified:
  Proceed to decision menu

IF any failed:
  Display failure report with retry menu
```

## Verification Checks

- **Step 1:** Check `result.stateUpdated == true` AND `.continue-here.md` stage field matches
- **Step 2:** If fallback ran, verify `.continue-here.md` and `PLUGINS.md` updated
- **Step 3:** If validation ran, check `validationReport.status` (PASS/WARNING acceptable, FAIL blocks)
- **Step 4:** `git log -1 --oneline` contains stage reference
- **Step 5:** All state files consistent

## Why Critical

Incomplete checkpoints cause state corruption:
- Missing state update → /continue can't resume
- Missing commit → changes lost on crash
- Inconsistent state → workflow cannot recover

## Applies To

- **Simple plugins** (complexity ≤2): After stages 1, 2, 3, 4
- **Complex plugins** (complexity ≥3): After stages 1 AND after EACH DSP/GUI phase (2.1, 2.2, 2.3+, 3.1, 3.2, 3.3+), then 4

Note: Phase count determined by plan.md (varies by complexity)
