---
name: plugin-workflow
description: Orchestrates JUCE plugin implementation through stages 2-5 (Foundation, DSP, GUI, Validation) using subagent delegation. Use when implementing plugins after planning completes, or when resuming with /continue command. Invoked by /implement command.
allowed-tools:
  - Task # REQUIRED - All stages 2-5 MUST invoke subagents
  - Bash # For git commits
  - Read # For contracts
  - Write # For documentation
  - Edit # For state updates
preconditions:
  - architecture.md must exist (from /plan)
  - plan.md must exist (from /plan)
  - Status must be 🚧 Stage 0 (complete) OR resuming from 🚧 Stage 1+
  - Plugin must NOT be ✅ Working or 📦 Installed (use /improve instead)
---

# plugin-workflow Skill

**Purpose:** Pure orchestrator for stages 2-5 of JUCE plugin implementation. This skill delegates to specialized subagents and presents decision menus after each stage completes.

## Overview

Implementation milestones:
- **Build System Ready** (Stage 2): Create build system and implement parameters (foundation-shell-agent)
- **Audio Engine Working** (Stage 3): Implement audio processing (dsp-agent)
- **UI Integrated** (Stage 4): Connect WebView interface to audio engine (gui-agent)
- **Plugin Complete** (Stage 5): Factory presets, validation, and final polish (validation-agent)

Stage 1 (Research & Planning) is handled by `plugin-planning` skill.

## Delegation Protocol

**CRITICAL:** Stages 2-5 MUST invoke subagents via Task tool. This skill is a pure orchestrator and NEVER implements plugin code directly.

**Delegation sequence for every stage:**
1. Load contracts in parallel (architecture.md, plan.md, parameter-spec.md, creative-brief.md)
2. Read Required Reading (juce8-critical-patterns.md) once at workflow start
3. Construct minimal prompt with plugin name + stage + Required Reading
4. Invoke subagent via Task tool
5. After subagent returns, invoke validation-agent (stages 2-4 only)
6. Execute checkpoint protocol (see references/checkpoint-protocol.md)

**Stage routing:**
- Stage 2 → foundation-shell-agent
- Stage 3 → dsp-agent
- Stage 4 → gui-agent
- Stage 5 → validation-agent (or direct execution)

## Preconditions

Before starting Stage 2, verify these contract files exist:
- `plugins/$PLUGIN_NAME/.ideas/architecture.md` (from Stage 1)
- `plugins/$PLUGIN_NAME/.ideas/plan.md` (from Stage 1)
- `plugins/$PLUGIN_NAME/.ideas/creative-brief.md` (from ideation)
- `plugins/$PLUGIN_NAME/.ideas/parameter-spec.md` (from UI mockup finalization)

**If parameter-spec-draft.md exists but parameter-spec.md missing:**
Block with message: "Draft parameters found, but full specification required. Complete UI mockup workflow to generate parameter-spec.md. Run: /dream [PluginName] → option 2 (Full UI mockup first)"

**If contracts missing:**
Block and instruct user to run `/plan [PluginName]` to complete Stage 1.

See [references/precondition-checks.md](references/precondition-checks.md) for implementation.

## Resume Entry Point

When resuming via `/continue [PluginName]`:

1. Verify state integrity (see references/state-management.md#verifyStateIntegrity)
2. Parse `.continue-here.md` for current stage and workflow mode
3. Verify contracts unchanged since last checkpoint (checksums match)
4. Verify git working directory clean
5. Verify PLUGINS.md status matches .continue-here.md stage

**If all checks pass:** Resume at stage specified in .continue-here.md
**If any fail:** Present recovery menu (reconcile / clean working directory / review changes)

## Workflow Mode

Determine whether to auto-progress (express mode) or present menus (manual mode).

**Mode sources (priority order):**
1. Environment variables: `WORKFLOW_MODE=express|manual`
2. .continue-here.md field (for resumed workflows)
3. Default to "manual"

**Express mode behavior:**
- Auto-progress through stages without menus
- Drops to manual on ANY error (build failures, test failures, etc.)
- Final menu always appears after Stage 5

See [references/workflow-mode.md](references/workflow-mode.md) for implementation.

## Stage Dispatcher

**Entry point:** Called by /implement or /continue after plugin-planning completes.

**Dispatch flow:**
1. Verify state integrity → BLOCK if corrupted (exit 2 → run /reconcile)
2. Check preconditions → BLOCK if failed
3. **Automatic brief sync** (before Stage 2 only, if mockup exists) → See [references/creative-brief-sync.md](references/creative-brief-sync.md)
4. Route to subagent based on stage
5. Pass contracts and Required Reading to subagent
6. Wait for subagent completion
7. Execute checkpoint protocol

See [references/dispatcher-pattern.md](references/dispatcher-pattern.md) for full algorithm.

## Phase-Aware Dispatch

For Stages 3-4 with complexity ≥3, use phase-aware dispatch to incrementally implement complex plugins.

**When to use:**
- Stage 3 (DSP) or Stage 4 (GUI)
- Complexity score ≥3 (from plan.md)
- plan.md contains phase markers (### Phase 3.X or ### Phase 4.X)

**How it works:**
1. Detect phases by scanning plan.md for phase markers
2. Loop through phases sequentially (Phase 3.1 → 3.2 → 3.3...)
3. Invoke subagent once per phase with phase-specific prompt
4. Execute checkpoint protocol after each phase
5. Present decision menu showing progress ("Phase 2 of 4 complete")

**CRITICAL:** Never send "Implement ALL phases" to subagent. This caused DrumRoulette Stage 3 compilation errors. Phase-aware dispatch is MANDATORY for complex plugins.

See [references/phase-aware-dispatch.md](references/phase-aware-dispatch.md) for detailed algorithm.

## Checkpoint Protocol

After EVERY subagent return, execute this 6-step sequence:

1. **Verify state update:** Check subagent updated .continue-here.md and PLUGINS.md
2. **Fallback state update:** If verification fails, orchestrator updates state
3. **Invoke validation:** Run validation-agent for stages 2-4 (advisory)
4. **Commit stage:** Auto-commit all changes with git
5. **Verify checkpoint:** Validate all steps completed successfully
6. **Handle checkpoint:** Present menu (manual mode) or auto-progress (express mode)

**Checkpoint applies to:**
- Simple plugins (complexity ≤2): After stages 2, 3, 4, 5
- Complex plugins (complexity ≥3): After stage 2 AND after EACH DSP/GUI phase, then 5

See [references/checkpoint-protocol.md](references/checkpoint-protocol.md) for implementation.

## Validation Integration

Stages 2-4 invoke validation-agent for semantic review after subagent completes:
- Advisory only (doesn't block progression unless FAIL + continue_to_next_stage=false)
- Checks contracts match implementation
- Returns JSON report with status, checks, recommendation
- Max 500 tokens per report

See [references/validation-integration.md](references/validation-integration.md) for functions.

## Subagent Handoff Protocol

Subagents update state files AND return JSON report:

```json
{
  "status": "success" | "error",
  "stage": 2-5,
  "completionStatement": "...",
  "filesCreated": [...],
  "nextSteps": [...],
  "stateUpdated": true | false,
  "stateUpdateError": "..." (optional)
}
```

**Verification:**
1. Check `stateUpdated` field in JSON report
2. If true: Verify .continue-here.md actually changed
3. If false/missing: Trigger orchestrator fallback

**Fallback:** Orchestrator reads current state, updates fields, writes back.

See [references/state-management.md](references/state-management.md) for fallback implementation.

## Required Reading Injection

All subagents (stages 2-5) receive `troubleshooting/patterns/juce8-critical-patterns.md` to prevent repeat mistakes.

**Implementation:**
1. Read juce8-critical-patterns.md ONCE at workflow start
2. Prepend to all subagent prompts with clear separator
3. Pass to each subagent invocation from memory (no re-reading)

## Reference Files

Each stage has detailed documentation in references/:

- [stage-2-foundation-shell.md](references/stage-2-foundation-shell.md) - foundation-shell-agent prompt template
- [stage-3-dsp.md](references/stage-3-dsp.md) - dsp-agent prompt template
- [stage-4-gui.md](references/stage-4-gui.md) - gui-agent prompt template
- [stage-5-validation.md](references/stage-5-validation.md) - validation-agent prompt template
- [state-management.md](references/state-management.md) - State functions
- [dispatcher-pattern.md](references/dispatcher-pattern.md) - Routing logic
- [precondition-checks.md](references/precondition-checks.md) - Contract validation
- [phase-aware-dispatch.md](references/phase-aware-dispatch.md) - Complex plugin handling
- [workflow-mode.md](references/workflow-mode.md) - Express vs manual mode
- [checkpoint-protocol.md](references/checkpoint-protocol.md) - 6-step checkpoint sequence
- [validation-integration.md](references/validation-integration.md) - Validation-agent functions
- [creative-brief-sync.md](references/creative-brief-sync.md) - Automatic brief update from mockup
- [error-handling.md](references/error-handling.md) - Error patterns and recovery
- [integration-contracts.md](references/integration-contracts.md) - Component contracts

## Integration Points

**Invoked by:**
- `/implement` command (after plugin-planning completes)
- `/continue` command (for stages 2-5)
- `context-resume` skill (when resuming implementation)

**Invokes via Task tool:**
- `foundation-shell-agent` (Stage 2) - REQUIRED
- `dsp-agent` (Stage 3) - REQUIRED
- `gui-agent` (Stage 4) - REQUIRED
- `validation-agent` (Stages 2-5) - Advisory

**Also invokes:**
- `build-automation` skill (build verification)
- `plugin-testing` skill (validation after stages)
- `plugin-lifecycle` skill (if user chooses to install)

**Reads (contracts):**
- architecture.md, plan.md, creative-brief.md, parameter-spec.md

**Creates:**
- .continue-here.md (handoff file)
- CHANGELOG.md (Stage 5)
- Presets/ directory (Stage 5)

**Updates:**
- PLUGINS.md (status after each stage)
- .continue-here.md (after each stage)

## Error Handling

**Contract files missing before Stage 2:**
Block and instruct user to run `/plan [PluginName]`.

**Build fails during subagent execution:**
Subagent returns error. Present menu:
1. Investigate (deep-research)
2. Show code
3. Show build output
4. Manual fix (resume with /continue)

**State mismatch detected (exit 2):**
BLOCKING error - user must run `/reconcile [PluginName]` to fix.

**Validation fails:**
Present menu with investigation options. Don't auto-proceed unless validation allows.

See [references/error-handling.md](references/error-handling.md) for detailed patterns.

## Decision Menu Protocol

**Use inline numbered menus for:**
- After EVERY stage completion (checkpoint gates)
- Build failure recovery
- Test failure investigation
- Phase completion (for complex plugins)

**Format:**
```
✓ [Milestone name]

What's next?

1. [Next milestone action] (recommended)
2. [Run tests] - Verify implementation
3. [Pause workflow] - Resume anytime
4. [Review code] - See what was implemented
5. Other

Choose (1-5): _
```

**Express mode:** Skip menus and auto-progress to next stage (except final stage).
**Manual mode:** ALWAYS wait for user response.

## Success Criteria

Workflow succeeds when:
- All subagents (stages 2-4) invoked successfully via Task tool
- Plugin compiles without errors at each stage
- All stages completed in sequence (2 → 3 → 4 → 5)
- Decision menus presented after EVERY stage (manual mode)
- PLUGINS.md updated to ✅ Working after Stage 5
- Handoff file updated after each stage
- Git history shows atomic commits for each stage

## Anti-Patterns

Common pitfalls to AVOID:

**CRITICAL:**
- ❌ Implementing stage logic directly in orchestrator
- ✓ ALWAYS use Task tool to invoke appropriate subagent

**CRITICAL:**
- ❌ Sending "Implement ALL phases" to subagent for Stages 3-4
- ✓ ALWAYS detect phases in plan.md and loop through them one at a time

**HIGH:**
- ❌ Not verifying subagent updated state
- ✓ Check stateUpdated field, verify .continue-here.md changed, fallback if needed

**HIGH:**
- ❌ Skipping phase detection for Stages 3-4 when complexity ≥3
- ✓ Read plan.md to check for phases BEFORE invoking dsp-agent or gui-agent

**MEDIUM:**
- ❌ Proceeding to next stage when tests fail
- ✓ Present investigation menu and wait for user decision

**MEDIUM:**
- ❌ Not injecting Required Reading to subagents
- ✓ Always pass juce8-critical-patterns.md to prevent repeat mistakes
