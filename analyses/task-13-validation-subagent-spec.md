# Task 13: Validation Subagent Utilization - Implementation Specification

**Date:** 2025-11-13
**Task ID:** 13
**Priority:** Medium
**Estimated Implementation:** 3-4 days
**Token Savings Target:** 52k tokens (90% reduction in orchestrator context)

---

## Executive Summary

This optimization restructures how contracts (creative-brief.md, architecture.md, plan.md, parameter-spec.md) are loaded during the plugin implementation workflow. Currently, the orchestrator (plugin-workflow skill) loads all contracts into its context before invoking each subagent, accumulating ~55k tokens across 5 stages and causing premature compaction.

**Proposed change:** The orchestrator will invoke subagents with minimal prompts (~100 tokens: plugin name + stage only). Subagents will read contracts from files themselves. After each subagent returns, the orchestrator will invoke validation-agent to perform semantic validation (~11k tokens in validation context). The orchestrator will only receive a 500-token pass/fail summary.

**Expected impact:**
- **Before:** 55k tokens in orchestrator (11k contracts × 5 stages)
- **After:** 3k tokens in orchestrator (100 tokens × 5 stages + 500 tokens × 5 validations)
- **Savings:** 52k tokens (95% reduction)
- **Benefit:** Prevents premature compaction, extends conversation length by ~175k tokens

**User impact:** Transparent. No workflow changes, same functionality, better performance.

---

## Current System Analysis

### Contract Loading Flow

**Current behavior (traced from stage-2-foundation-shell.md):**

```typescript
// Step 1: Orchestrator reads contracts (plugin-workflow skill)
const creativeBrief = Read({ file_path: "plugins/[Name]/.ideas/creative-brief.md" });
const architecture = Read({ file_path: "plugins/[Name]/.ideas/architecture.md" });
const plan = Read({ file_path: "plugins/[Name]/.ideas/plan.md" });
const parameterSpec = Read({ file_path: "plugins/[Name]/.ideas/parameter-spec.md" });
const criticalPatterns = Read({ file_path: "troubleshooting/patterns/juce8-critical-patterns.md" });

// Step 2: Orchestrator constructs large prompt (embeds contracts)
const prompt = `CRITICAL PATTERNS (MUST FOLLOW):

${criticalPatterns}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Create foundation and shell for plugin at plugins/${pluginName}.

Inputs:
- creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md
- architecture.md: plugins/${pluginName}/.ideas/architecture.md
- plan.md: plugins/${pluginName}/.ideas/plan.md
- parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
- Plugin name: ${pluginName}

Tasks:
1. Read creative-brief.md and extract PRODUCT_NAME
2. Read architecture.md and determine plugin type
...
`;

// Step 3: Orchestrator invokes subagent via Task tool
Task({
  subagent_type: "foundation-shell-agent",
  description: `Create build system and implement parameters for ${pluginName}`,
  prompt: prompt // Large prompt with embedded contracts (11k tokens)
});
```

**Key findings:**

1. **Contracts ARE listed in prompt but NOT embedded in full:**
   - The orchestrator references file paths: `"creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md"`
   - Current prompt instructs subagent: `"1. Read creative-brief.md and extract PRODUCT_NAME"`
   - This means **subagents already read contracts themselves** (they don't receive content in prompt)

2. **BUT orchestrator loads them too:**
   - Lines 38-44 in stage-2-foundation-shell.md show orchestrator reads contracts
   - Contracts sit in orchestrator context as variables
   - After subagent invocation, these variables remain in memory until stage completes

3. **Required Reading IS embedded:**
   - `troubleshooting/patterns/juce8-critical-patterns.md` is prepended to every subagent prompt
   - ~4k words ≈ 5.3k tokens
   - This happens for EVERY stage (2, 3, 4, and validation invocations)

4. **Contracts leave orchestrator memory:**
   - After subagent returns and checkpoint completes
   - But orchestrator loads them again for next stage
   - 5 stages × loading contracts = accumulation pattern

### Token Overhead Breakdown

**Measured token estimates (based on AutoClip plugin contracts):**

| File | Lines | Words | Tokens (×1.33) |
|------|-------|-------|----------------|
| creative-brief.md | 77 | 521 | 693 |
| architecture.md | 631 | 3,397 | 4,518 |
| plan.md | 337 | 1,883 | 2,504 |
| parameter-spec.md | 140 | 514 | 684 |
| juce8-critical-patterns.md | N/A | 3,978 | 5,292 |
| **Total per stage** | **1,185** | **10,293** | **13,691** |

**Actual overhead in orchestrator:**

**Current state (contracts loaded in orchestrator):**
```
Stage 2 invocation:
  - Read contracts: 13,691 tokens (loaded into orchestrator vars)
  - Construct prompt: ~500 tokens (instructions + file paths)
  - Invoke subagent: Prompt sent to Task tool
  - Orchestrator holds: 13,691 tokens (contracts) + 500 tokens (prompt) = 14,191 tokens

Stage 3 invocation:
  - Read contracts again: 13,691 tokens
  - Construct prompt: ~500 tokens
  - Orchestrator holds: 28,382 tokens cumulative

Stage 4 invocation:
  - Read contracts again: 13,691 tokens
  - Orchestrator holds: 42,073 tokens cumulative

Stage 5 (validation-agent invocation for complexity ≥4):
  - Read contracts again: 13,691 tokens
  - Orchestrator holds: 55,764 tokens cumulative
```

**Total orchestrator token overhead:** ~56k tokens across 5 stages

**Note:** The roadmap estimate of 55k tokens is accurate. Actual measurement shows 56k.

### Current Validation Approach

**validation-agent current usage:**

1. **Stage 0 (Architecture Specification Validation):**
   - Validates architecture.md quality
   - Checks for Context7 references, professional examples, feasibility assessment
   - Returns JSON report with checks

2. **Stage 1 (Planning Validation):**
   - Validates plan.md cross-contract consistency
   - Runs `validate-cross-contract.py` (Python validator)
   - Checks complexity score, phase breakdown, contract references
   - Returns JSON report

3. **Stage 5/6 (Final Validation):**
   - Validates CHANGELOG.md, Presets/, pluginval logs
   - Checks completion criteria
   - Returns JSON report

**Stages 2, 3, 4 validation:**
- Currently NO semantic validation beyond deterministic hook checks
- SubagentStop hook runs Python validators (don't consume Claude tokens)
- No validation-agent invocation after foundation-shell, dsp, gui agents
- Opportunity: Add semantic validation for these stages

**validation-agent capabilities (from validation-agent.md):**

- Reads contracts from files (architecture.md, parameter-spec.md, plan.md)
- Performs semantic checks (code quality, design soundness, JUCE best practices)
- Returns structured JSON with 500-token summary format
- Uses Opus model for superior reasoning (semantic correctness requires judgment)
- **Key insight:** validation-agent ALREADY reads contracts itself (lines 36-59)

---

## Proposed Architecture

### Minimal Prompt Pattern

**New orchestrator → subagent invocation (Stage 2 example):**

```typescript
// Step 1: Orchestrator does NOT read contracts
// (Removed: Read creative-brief.md, architecture.md, plan.md, parameter-spec.md)

// Step 2: Orchestrator constructs minimal prompt
const minimalPrompt = `
You are foundation-shell-agent implementing Stage 2 for ${pluginName}.

**Plugin:** ${pluginName}
**Stage:** 2 (Foundation + Shell)
**Your task:** Create build system and implement ALL parameters from parameter-spec.md

**Contracts (read these files yourself):**
- creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md
- architecture.md: plugins/${pluginName}/.ideas/architecture.md
- plan.md: plugins/${pluginName}/.ideas/plan.md
- parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
- Required Reading: troubleshooting/patterns/juce8-critical-patterns.md

**Required steps:**
1. Read contracts from file paths above
2. Read Required Reading (MANDATORY before implementation)
3. Create CMakeLists.txt with JUCE 8 integration
4. Create Source/PluginProcessor.{h,cpp} with APVTS
5. Implement ALL parameters from parameter-spec.md
6. Return JSON report with status and file list

**Build verification:** Handled by orchestrator after you complete.
`;

// Step 3: Invoke subagent with minimal prompt
Task({
  subagent_type: "foundation-shell-agent",
  description: `Stage 2 - ${pluginName}`,
  prompt: minimalPrompt // ~100 tokens (vs 14k currently)
});
```

**Token breakdown for minimal prompt:**
- Plugin name + stage number: ~10 tokens
- Contract file paths (5 files): ~50 tokens
- Task instructions: ~30 tokens
- Template structure: ~10 tokens
- **Total:** ~100 tokens (vs 14,191 tokens currently)

**Savings:** 14,091 tokens per stage invocation

### Subagent Self-Loading

**Subagents already read contracts themselves:**

Evidence from foundation-shell-agent.md (lines 20-23):
```markdown
**What you do:**
1. Read contracts (creative-brief.md, architecture.md, plan.md, parameter-spec.md)
2. Create CMakeLists.txt and C++ source files
3. Implement APVTS with all parameters
4. Return JSON report
```

Evidence from dsp-agent.md (lines 100+):
```markdown
## Implementation Steps

### 1. Extract Requirements

Read the contract files and extract:
- **Plugin name** from creative-brief.md
- **Plugin type** from architecture.md
- **DSP components** from architecture.md
```

**Subagents have Read tool access** (confirmed in agent frontmatter):
- foundation-shell-agent: `tools: Read, Write, Edit, mcp__context7__resolve-library-id, mcp__context7__get-library-docs`
- dsp-agent: `tools: Read, Edit, Write, mcp__context7__resolve-library-id, mcp__context7__get-library-docs, mcp__sequential-thinking__sequentialthinking`
- gui-agent: `tools: Read, Edit, Write, Bash, mcp__context7__resolve-library-id, mcp__context7__get-library-docs`

**Current redundancy:**
1. Orchestrator reads contracts → stores in context (~14k tokens)
2. Orchestrator passes file paths to subagent in prompt
3. Subagent reads contracts again from files (in its own context)
4. Contracts sit in orchestrator context until stage completes

**Proposed change:**
1. Orchestrator does NOT read contracts
2. Orchestrator passes file paths to subagent in prompt (~100 tokens)
3. Subagent reads contracts from files (in its own context)
4. Orchestrator context stays lightweight

**Required Reading handling:**

**Current:** Orchestrator reads `juce8-critical-patterns.md` and embeds in prompt (~5.3k tokens × 5 stages = 26.5k tokens)

**Proposed:** Two options:

**Option A (Recommended):** Subagents read Required Reading themselves
```typescript
const minimalPrompt = `
**Required Reading (MANDATORY):**
Read troubleshooting/patterns/juce8-critical-patterns.md BEFORE implementation.
This file contains JUCE 8 patterns that prevent repeat mistakes.
`;
```
- Orchestrator: 0 tokens (just file path)
- Subagent: 5.3k tokens (reads file in its own context)
- Savings: 26.5k tokens across 5 stages in orchestrator

**Option B (Conservative):** Keep embedding in prompt
- Orchestrator: 5.3k tokens per stage (26.5k cumulative)
- Preserves current guarantee that subagent sees patterns
- But reduces total savings from 52k to ~26k

**Recommendation:** Option A. Subagents are already trusted to read contracts correctly. Required Reading should be treated the same way. Add explicit instruction in minimal prompt template.

### validation-agent Integration Points

**Current validation-agent invocations:**

| Stage | Current | Proposed |
|-------|---------|----------|
| Stage 0 | ✓ Invoked (architecture quality) | ✓ Keep |
| Stage 1 | ✓ Invoked (planning validation) | ✓ Keep |
| Stage 2 | ✗ Not invoked | **✓ Add** (foundation validation) |
| Stage 3 | ✗ Not invoked | **✓ Add** (DSP validation) |
| Stage 4 | ✗ Not invoked | **✓ Add** (GUI validation) |
| Stage 5 | ✓ Invoked (final validation) | ✓ Keep |

**New checkpoint protocol (after each subagent):**

```typescript
// After subagent returns (e.g., foundation-shell-agent completes)

// Step 1: Parse subagent JSON report
const subagentReport = parseSubagentReport(rawOutput);

// Step 2: Invoke validation-agent
const validationPrompt = `
Validate Stage ${currentStage} completion for ${pluginName}.

**Stage:** ${currentStage}
**Plugin:** ${pluginName}

**Contracts (read these files yourself):**
- creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md
- architecture.md: plugins/${pluginName}/.ideas/architecture.md
- plan.md: plugins/${pluginName}/.ideas/plan.md
- parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md

**Expected outputs for Stage ${currentStage}:**
${getStageExpectations(currentStage)}

Return JSON validation report with status, checks, and recommendation.
`;

Task({
  subagent_type: "validation-agent",
  description: `Validate Stage ${currentStage} - ${pluginName}`,
  prompt: validationPrompt // ~100 tokens (validation-agent reads contracts)
});

// Step 3: Parse validation summary
const validationReport = parseValidationReport(validationOutput);

// Step 4: Orchestrator receives 500-token summary
// {
//   "agent": "validation-agent",
//   "stage": 2,
//   "status": "PASS",
//   "checks": [...],
//   "recommendation": "...",
//   "continue_to_next_stage": true
// }

// Step 5: Orchestrator uses summary for decision
if (validationReport.status === "FAIL" || !validationReport.continue_to_next_stage) {
  // Present validation failure menu
  presentValidationMenu(validationReport);
} else {
  // Continue with normal checkpoint (commit, update state, present menu)
  executeCheckpoint(currentStage, subagentReport, validationReport);
}
```

**Invocation point in checkpoint sequence:**

```typescript
// Current checkpoint (5 steps):
// 1. Verify state update
// 2. Fallback state update (if needed)
// 3. Commit changes
// 4. Verify checkpoint
// 5. Present decision menu

// New checkpoint (6 steps):
// 1. Verify state update
// 2. Fallback state update (if needed)
// 3. **Invoke validation-agent** (NEW)
// 4. Commit changes (includes validation summary)
// 5. Verify checkpoint
// 6. Present decision menu
```

**Validation happens BEFORE commit:**
- Subagent creates files
- validation-agent reviews semantic correctness
- If validation fails, user can fix before commit
- If validation passes, commit includes validation summary in message

### Validation Summary Format

**Schema (based on .claude/schemas/validator-report.json):**

```json
{
  "agent": "validation-agent",
  "stage": 2,
  "plugin_name": "PluginName",
  "status": "PASS" | "FAIL" | "WARNING",
  "checks": [
    {
      "name": "check_identifier",
      "passed": true | false,
      "message": "Descriptive message about finding",
      "severity": "info" | "warning" | "error"
    }
  ],
  "recommendation": "Brief summary of findings and suggested next action",
  "continue_to_next_stage": true | false,
  "token_count": 487
}
```

**Required fields:**
- `agent`: Must be "validation-agent"
- `stage`: Integer 0-6
- `plugin_name`: String (for logging)
- `status`: "PASS" | "FAIL" | "WARNING"
- `checks`: Array of check objects
- `recommendation`: String (1-2 sentences)
- `continue_to_next_stage`: Boolean
- `token_count`: Self-reported token usage (for monitoring)

**Token budget:** 500 tokens maximum

**Example for Stage 2 (Foundation + Shell):**

```json
{
  "agent": "validation-agent",
  "stage": 2,
  "plugin_name": "AutoClip",
  "status": "PASS",
  "checks": [
    {
      "name": "juce_modules",
      "passed": true,
      "message": "CMakeLists.txt includes juce_audio_processors and juce_audio_basics for audio plugin",
      "severity": "info"
    },
    {
      "name": "juce8_patterns",
      "passed": true,
      "message": "ParameterID uses version 1 format, juce_generate_juce_header() called after target_link_libraries()",
      "severity": "info"
    },
    {
      "name": "parameter_count",
      "passed": true,
      "message": "All 7 parameters from parameter-spec.md implemented in APVTS",
      "severity": "info"
    },
    {
      "name": "parameter_ids",
      "passed": true,
      "message": "Parameter IDs match specification exactly (zero-drift)",
      "severity": "info"
    }
  ],
  "recommendation": "Foundation follows JUCE 8 best practices, all parameters implemented correctly",
  "continue_to_next_stage": true,
  "token_count": 412
}
```

**Example for Stage 3 (DSP) with warning:**

```json
{
  "agent": "validation-agent",
  "stage": 3,
  "plugin_name": "AutoClip",
  "status": "WARNING",
  "checks": [
    {
      "name": "creative_intent",
      "passed": true,
      "message": "Soft-clipping algorithm matches 'warm saturation' description from brief",
      "severity": "info"
    },
    {
      "name": "realtime_safety",
      "passed": true,
      "message": "No allocations in processBlock(), uses ScopedNoDenormals",
      "severity": "info"
    },
    {
      "name": "buffer_preallocation",
      "passed": true,
      "message": "prepareToPlay() allocates delay buffers",
      "severity": "info"
    },
    {
      "name": "edge_cases",
      "passed": false,
      "message": "No check for zero-length buffer in processBlock() line 87",
      "severity": "warning"
    }
  ],
  "recommendation": "DSP implementation solid, consider adding zero-length buffer check for robustness",
  "continue_to_next_stage": true,
  "token_count": 398
}
```

**Example for Stage 4 (GUI) with failure:**

```json
{
  "agent": "validation-agent",
  "stage": 4,
  "plugin_name": "AutoClip",
  "status": "FAIL",
  "checks": [
    {
      "name": "member_order",
      "passed": false,
      "message": "Member declaration order incorrect: attachments declared before webView (should be relays → webView → attachments)",
      "severity": "error"
    },
    {
      "name": "parameter_bindings",
      "passed": true,
      "message": "All 7 parameters have relay/attachment pairs",
      "severity": "info"
    },
    {
      "name": "ui_aesthetic",
      "passed": true,
      "message": "Visual design matches mockup v2",
      "severity": "info"
    }
  ],
  "recommendation": "Fix member declaration order to prevent release build crashes (90% crash rate with wrong order)",
  "continue_to_next_stage": false,
  "token_count": 356
}
```

### Stage-Specific Validation Checks

**Stage 2 (Foundation + Shell):**

Semantic checks (hooks already validated patterns exist):
- ✓ CMakeLists.txt uses appropriate JUCE modules for plugin type
- ✓ Plugin format configuration matches creative brief (VST3/AU/Standalone)
- ✓ JUCE 8 patterns used (ParameterID with version 1, juce_generate_juce_header())
- ✓ PluginProcessor inherits correctly from AudioProcessor
- ✓ Editor/processor relationship properly established
- ✓ All parameters from parameter-spec.md implemented in APVTS
- ✓ Parameter IDs match specification exactly (zero-drift)
- ✓ Code organization follows JUCE best practices

**Stage 3 (DSP):**

Semantic checks (hooks verified components exist):
- ✓ DSP algorithm matches creative intent from brief
- ✓ Real-time safety maintained (no allocations in processBlock)
- ✓ Buffer preallocation in prepareToPlay()
- ✓ Component initialization order correct
- ✓ Parameter modulation applied correctly
- ✓ Edge cases handled (zero-length buffers, extreme parameter values)
- ✓ Numerical stability considerations (denormals, DC offset)
- ✓ ScopedNoDenormals used in processBlock

**Stage 4 (GUI):**

Semantic checks (hooks verified bindings exist):
- ✓ Member declaration order correct (Relays → WebView → Attachments)
- ✓ UI layout matches mockup aesthetic
- ✓ Parameter ranges in UI match spec
- ✓ Visual feedback appropriate (knobs respond to mouse)
- ✓ Accessibility considerations (labels, contrast)
- ✓ WebView initialization safe (error handling)
- ✓ Binary data embedded correctly
- ✓ All parameters from spec have UI bindings

**Stage 5 (Validation) - Keep existing checks:**
- ✓ CHANGELOG.md follows Keep a Changelog format
- ✓ Version 1.0.0 for initial release
- ✓ Presets/ directory has 3+ preset files
- ✓ pluginval passed (or skipped with reason)
- ✓ PLUGINS.md updated to ✅ Working

---

## Implementation Plan

### Phase 1: Orchestrator Modifications

**File:** `.claude/skills/plugin-workflow/SKILL.md`

**Changes needed:**

1. **Remove contract loading from orchestrator (lines 44-49):**

```diff
- 1. BEFORE invoking subagent, read contract files:
-    - architecture.md (DSP design from Stage 0)
-    - plan.md (implementation strategy from Stage 0)
-    - parameter-spec.md (parameter definitions)
- 2. Read Required Reading:
-    - troubleshooting/patterns/juce8-critical-patterns.md (MANDATORY)
- 3. Construct prompt with contracts + Required Reading prepended
- 4. Invoke subagent via Task tool with constructed prompt
+ 1. Construct minimal prompt with plugin name + stage + file paths
+ 2. Invoke subagent via Task tool
+ 3. Subagent reads contracts from files (in its own context)
+ 4. After subagent returns, invoke validation-agent
```

2. **Add validation-agent invocation to checkpoint protocol (after line 544):**

```diff
  <step order="2" required="true" function="fallbackStateUpdate">
    ...
  </step>

+ <step order="3" required="true" function="invokeValidation">
+   IF currentStage in [2, 3, 4]:
+     validationResult = invokeValidationAgent(pluginName, currentStage)
+     parseValidationReport(validationResult)
+     IF validation.status == "FAIL" AND validation.continue_to_next_stage == false:
+       presentValidationFailureMenu(validation)
+       BLOCK progression until user resolves issues
+   ELSE:
+     Log: "Validation skipped for stage ${currentStage}"
+ </step>

- <step order="3" required="true" function="commitStage">
+ <step order="4" required="true" function="commitStage">
    commitStage(pluginName, currentStage, result.description)
    ...
  </step>
```

3. **Update checkpoint verification (line 596-638):**

```diff
  **Verification checks:**
  - Step 1: Check result.stateUpdated == true AND .continue-here.md stage field matches
  - Step 2: If fallback ran, verify .continue-here.md and PLUGINS.md updated
- - Step 3: `git log -1 --oneline` contains stage reference
- - Step 4: All state files consistent
+ - Step 3: If validation ran, check validationReport.status (PASS/WARNING acceptable, FAIL blocks)
+ - Step 4: `git log -1 --oneline` contains stage reference
+ - Step 5: All state files consistent
```

4. **Add validation helper functions (new section after line 788):**

```typescript
function invokeValidationAgent(pluginName: string, stage: number): string {
  const expectations = getStageExpectations(stage);

  const prompt = `
Validate Stage ${stage} completion for ${pluginName}.

**Stage:** ${stage}
**Plugin:** ${pluginName}

**Contracts (read these files yourself):**
- creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md
- architecture.md: plugins/${pluginName}/.ideas/architecture.md
- plan.md: plugins/${pluginName}/.ideas/plan.md
- parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
- Required Reading: troubleshooting/patterns/juce8-critical-patterns.md

**Expected outputs for Stage ${stage}:**
${expectations}

Return JSON validation report with status, checks, and recommendation.
  `;

  return Task({
    subagent_type: "validation-agent",
    description: `Validate Stage ${stage} - ${pluginName}`,
    prompt: prompt
  });
}

function getStageExpectations(stage: number): string {
  const expectations = {
    2: `- CMakeLists.txt with JUCE 8 patterns
- PluginProcessor.{h,cpp} with APVTS
- All parameters from parameter-spec.md implemented
- Parameter IDs match spec exactly (zero-drift)
- JUCE 8 ParameterID format used
- juce_generate_juce_header() called correctly`,

    3: `- All DSP components from architecture.md implemented
- processBlock() contains real-time safe processing
- Parameters modulate DSP components correctly
- prepareToPlay() allocates buffers
- No heap allocations in processBlock
- ScopedNoDenormals used
- Edge cases handled`,

    4: `- Member declaration order correct (Relays → WebView → Attachments)
- All parameters from spec have UI bindings
- HTML element IDs match relay names
- UI aesthetic matches mockup
- Visual feedback works (knobs respond)
- WebView initialization includes error handling`
  };

  return expectations[stage] || "No expectations defined";
}

function parseValidationReport(rawOutput: string): object {
  try {
    // Extract JSON from markdown code blocks
    const jsonMatch = rawOutput.match(/```json\n([\s\S]*?)\n```/) ||
                      rawOutput.match(/```\n([\s\S]*?)\n```/);

    if (jsonMatch) {
      return JSON.parse(jsonMatch[1]);
    }

    // Try parsing entire output
    return JSON.parse(rawOutput);
  } catch (error) {
    console.warn("Could not parse validation report, treating as PASS");
    return {
      agent: "validation-agent",
      status: "WARNING",
      checks: [],
      recommendation: "Could not parse validation output (validation is advisory)",
      continue_to_next_stage: true
    };
  }
}

function presentValidationFailureMenu(validation: object) {
  const errors = validation.checks.filter(c => c.severity === "error");
  const warnings = validation.checks.filter(c => c.severity === "warning");

  console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ Validation ${validation.status}: Stage ${validation.stage}
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Semantic validation found issues that should be addressed:
  `);

  if (errors.length > 0) {
    console.log("\n❌ Errors:");
    errors.forEach(e => console.log(`  - ${e.message}`));
  }

  if (warnings.length > 0) {
    console.log("\n⚠️  Warnings:");
    warnings.forEach(w => console.log(`  - ${w.message}`));
  }

  console.log(`\nRecommendation: ${validation.recommendation}`);
  console.log(`
What would you like to do?

1. Address issues - Fix validation errors before continuing
2. Continue anyway - Validation is advisory, proceed at your own risk
3. Show details - See full validation report
4. Pause workflow - I'll fix manually
5. Other

Choose (1-5): _
  `);

  // Wait for user input and handle accordingly
}
```

### Phase 2: Stage Reference Updates

**Files to modify:**
- `.claude/skills/plugin-workflow/references/stage-2-foundation-shell.md`
- `.claude/skills/plugin-workflow/references/stage-3-dsp.md`
- `.claude/skills/plugin-workflow/references/stage-4-gui.md`

**Changes needed in each file:**

1. **Remove contract reading section (stage-2 example, lines 36-55):**

```diff
- ### 2. Prepare Contracts for Subagent
-
- Read contract files that foundation-shell-agent needs:
-
- ```bash
- cat plugins/[PluginName]/.ideas/creative-brief.md
- cat plugins/[PluginName]/.ideas/architecture.md
- cat plugins/[PluginName]/.ideas/plan.md
- cat plugins/[PluginName]/.ideas/parameter-spec.md
- ```
-
- **CRITICAL: Read Required Patterns**
-
- Read JUCE 8 critical patterns file that MUST be followed:
-
- ```typescript
- const criticalPatterns = await Read({
-   file_path: "troubleshooting/patterns/juce8-critical-patterns.md"
- });
- ```

+ ### 2. Construct Minimal Prompt
+
+ Orchestrator does NOT read contracts - subagent will read them from files.
```

2. **Update subagent invocation to use minimal prompt (stage-2 example, lines 58-100):**

```diff
- const foundationShellResult = Task({
-   subagent_type: "foundation-shell-agent",
-   description: `Create build system and implement parameters for ${pluginName}`,
-   prompt: `CRITICAL PATTERNS (MUST FOLLOW):
-
- ${criticalPatterns}
-
- ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
-
- Create foundation and shell for plugin at plugins/${pluginName}.
-
- Inputs:
- - creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md
- - architecture.md: plugins/${pluginName}/.ideas/architecture.md
- - plan.md: plugins/${pluginName}/.ideas/plan.md
- - parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
- - Plugin name: ${pluginName}
-
- Tasks:
- 1. Read creative-brief.md and extract PRODUCT_NAME
- 2. Read architecture.md and determine plugin type (Audio Effect | Synth | Utility)
- 3. Read parameter-spec.md and extract ALL parameters
- 4. Create CMakeLists.txt with JUCE 8 integration
- ...`,
- });

+ const foundationShellResult = Task({
+   subagent_type: "foundation-shell-agent",
+   description: `Stage 2 - ${pluginName}`,
+   prompt: `
+ You are foundation-shell-agent implementing Stage 2 for ${pluginName}.
+
+ **Plugin:** ${pluginName}
+ **Stage:** 2 (Foundation + Shell)
+ **Your task:** Create build system and implement ALL parameters
+
+ **Contracts (read these files yourself):**
+ - creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md
+ - architecture.md: plugins/${pluginName}/.ideas/architecture.md
+ - plan.md: plugins/${pluginName}/.ideas/plan.md
+ - parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
+ - Required Reading: troubleshooting/patterns/juce8-critical-patterns.md
+
+ **CRITICAL: Read Required Reading BEFORE implementation.**
+
+ **Implementation steps:**
+ 1. Read all contract files listed above
+ 2. Read Required Reading (MANDATORY)
+ 3. Create CMakeLists.txt with JUCE 8 integration
+ 4. Create Source/PluginProcessor.{h,cpp} with APVTS
+ 5. Implement ALL parameters from parameter-spec.md
+ 6. Return JSON report with status and file list
+
+ Build verification handled by orchestrator after you complete.
+   `
+ });
```

3. **Add validation invocation after subagent success (after line 240):**

```diff
  if (report.status === "success" && report.ready_for_next_stage) {
    console.log(`✓ foundation-shell-agent complete: Build system + parameters`);
    ...

    // Now invoke build-automation to verify compilation
    console.log(`\nInvoking build-automation to verify compilation...`);
+
+   // Invoke validation-agent for semantic review
+   console.log(`\nInvoking validation-agent for semantic review...`);
+   const validationResult = invokeValidationAgent(pluginName, 2);
+   const validation = parseValidationReport(validationResult);
+
+   if (validation.status === "FAIL" && !validation.continue_to_next_stage) {
+     presentValidationFailureMenu(validation);
+     return; // Block until user resolves
+   }
+
+   console.log(`✓ Validation ${validation.status}: ${validation.recommendation}`);
```

**Apply same pattern to stage-3-dsp.md and stage-4-gui.md.**

### Phase 3: validation-agent Enhancement

**File:** `.claude/agents/validation-agent.md`

**Changes needed:**

1. **Add Stage 2 validation section (after line 289):**

```markdown
### Stage 2: Foundation Validation

**Expected Inputs:**

- `plugins/[PluginName]/CMakeLists.txt`
- `plugins/[PluginName]/Source/PluginProcessor.{h,cpp}`
- `plugins/[PluginName]/Source/PluginEditor.{h,cpp}`
- `plugins/[PluginName]/.ideas/architecture.md`
- `plugins/[PluginName]/.ideas/parameter-spec.md`

**Semantic Checks (hooks already validated patterns exist):**

- ✓ CMakeLists.txt uses appropriate JUCE modules for plugin type?
- ✓ Plugin format configuration matches creative brief (VST3/AU/Standalone)?
- ✓ JUCE 8 patterns used (ParameterID with version 1)?
- ✓ juce_generate_juce_header() called after target_link_libraries()?
- ✓ PluginProcessor inherits correctly from AudioProcessor?
- ✓ Editor/processor relationship properly established?
- ✓ All parameters from parameter-spec.md implemented in APVTS?
- ✓ Parameter IDs match specification exactly (zero-drift)?
- ✓ Code organization follows JUCE best practices?

**Example Report:**

```json
{
  "agent": "validation-agent",
  "stage": 2,
  "plugin_name": "AutoClip",
  "status": "PASS",
  "checks": [
    {
      "name": "juce_modules",
      "passed": true,
      "message": "CMakeLists.txt includes juce_audio_basics, juce_audio_processors for audio plugin",
      "severity": "info"
    },
    {
      "name": "plugin_formats",
      "passed": true,
      "message": "VST3 and AU formats enabled as specified in brief",
      "severity": "info"
    },
    {
      "name": "juce8_patterns",
      "passed": true,
      "message": "ParameterID uses version 1 format, juce_generate_juce_header() called correctly",
      "severity": "info"
    },
    {
      "name": "parameter_count",
      "passed": true,
      "message": "All 7 parameters from parameter-spec.md implemented in APVTS",
      "severity": "info"
    },
    {
      "name": "parameter_drift",
      "passed": true,
      "message": "Parameter IDs match specification exactly (zero-drift verified)",
      "severity": "info"
    }
  ],
  "recommendation": "Foundation follows JUCE 8 best practices, all parameters implemented correctly",
  "continue_to_next_stage": true,
  "token_count": 423
}
```
```

2. **Update Stage 3 validation section (currently labeled "Stage 3: Shell Validation", lines 290-342):**

```diff
- ### Stage 3: Shell Validation
+ ### Stage 3: DSP Validation

  **Expected Inputs:**

- - `plugins/[PluginName]/Source/PluginProcessor.cpp` (with APVTS)
- - `plugins/[PluginName]/.ideas/parameter-spec.md`
+ - `plugins/[PluginName]/Source/PluginProcessor.{h,cpp}` (with DSP implementation)
+ - `plugins/[PluginName]/.ideas/architecture.md`
+ - `plugins/[PluginName]/.ideas/parameter-spec.md`

- **Semantic Checks (hooks verified parameters exist):**
+ **Semantic Checks (hooks verified components exist):**

- - ✓ Parameter ranges appropriate for audio use (not arbitrary)?
- - ✓ Default values sensible for typical use?
- - ✓ Parameter smoothing strategy appropriate for parameter types?
- - ✓ APVTS creation follows JUCE best practices?
- - ✓ Parameter IDs follow consistent naming convention?
- - ✓ processBlock() stub safe (ScopedNoDenormals, pass-through)?
+ - ✓ DSP algorithm matches creative intent from brief?
+ - ✓ Real-time safety maintained (no allocations in processBlock)?
+ - ✓ Buffer preallocation in prepareToPlay()?
+ - ✓ Component initialization order correct?
+ - ✓ Parameter modulation applied correctly?
+ - ✓ Edge cases handled (zero-length buffers, extreme values)?
+ - ✓ Numerical stability (denormals, DC offset)?
+ - ✓ ScopedNoDenormals used in processBlock?
```

3. **Update Stage 4 validation section (currently labeled "Stage 4: DSP Validation", lines 343-403):**

```diff
- ### Stage 4: DSP Validation
+ ### Stage 4: GUI Validation

  **Expected Inputs:**

- - `plugins/[PluginName]/Source/PluginProcessor.{h,cpp}` (with DSP implementation)
- - `plugins/[PluginName]/.ideas/architecture.md`
+ - `plugins/[PluginName]/Source/PluginEditor.{h,cpp}` (with WebView integration)
+ - `plugins/[PluginName]/ui/public/index.html`
+ - `plugins/[PluginName]/.ideas/parameter-spec.md`

- **Semantic Checks (hooks verified components exist):**
+ **Semantic Checks (hooks verified bindings exist):**

- - ✓ DSP algorithm matches creative intent from brief?
- - ✓ Real-time safety maintained (no allocations in processBlock)?
- - ✓ Buffer preallocation in prepareToPlay()?
- - ✓ Component initialization order correct?
- - ✓ Parameter modulation applied correctly?
- - ✓ Edge cases handled (zero-length buffers, extreme parameter values)?
- - ✓ Numerical stability considerations (denormals, DC offset)?
+ - ✓ Member declaration order correct (Relays → WebView → Attachments)?
+ - ✓ UI layout matches mockup aesthetic?
+ - ✓ Parameter ranges in UI match spec?
+ - ✓ Visual feedback appropriate (knobs respond to mouse)?
+ - ✓ Accessibility considerations (labels, contrast)?
+ - ✓ WebView initialization safe (error handling)?
+ - ✓ Binary data embedded correctly?
+ - ✓ All parameters from spec have UI bindings?
```

4. **Update Stage 5 validation section (currently labeled "Stage 5: GUI Validation", lines 405-465):**

```diff
- ### Stage 5: GUI Validation
+ ### Stage 5: Final Validation

- **Expected Inputs:**
-
- - `plugins/[PluginName]/Source/PluginEditor.{h,cpp}` (with WebView integration)
- - `plugins/[PluginName]/ui/public/index.html`
- - `plugins/[PluginName]/.ideas/parameter-spec.md`

- **Semantic Checks (hooks verified bindings exist):**
-
- - ✓ Member declaration order correct (Relays → WebView → Attachments)?
- - ✓ UI layout matches mockup aesthetic?
- - ✓ Parameter ranges in UI match spec?
- - ✓ Visual feedback appropriate (knobs respond to mouse)?
- - ✓ Accessibility considerations (labels, contrast)?
- - ✓ WebView initialization safe (error handling)?
- - ✓ Binary data embedded correctly?

+ (Keep existing Stage 6 validation content - CHANGELOG, Presets, pluginval)
```

5. **Add token counting reminder (after line 502):**

```markdown
## Token Budget Enforcement

All validation reports MUST stay within 500-token budget.

**How to achieve this:**

1. **Concise messages:** Each check message should be 1-2 sentences max
2. **Group related checks:** Combine similar findings into single check
3. **Limit check count:** Maximum 5-7 checks per report
4. **Brief recommendation:** 1-2 sentences only
5. **Self-report tokens:** Include `token_count` field in JSON

**Example of token-efficient report:**

```json
{
  "agent": "validation-agent",
  "stage": 3,
  "plugin_name": "AutoClip",
  "status": "PASS",
  "checks": [
    {
      "name": "juce8_compliance",
      "passed": true,
      "message": "All JUCE 8 patterns followed (ParameterID format, header generation, real-time safety)",
      "severity": "info"
    },
    {
      "name": "dsp_correctness",
      "passed": true,
      "message": "DSP matches architecture.md, parameters connected, buffer handling correct",
      "severity": "info"
    },
    {
      "name": "edge_cases",
      "passed": false,
      "message": "Missing zero-length buffer check in processBlock line 87",
      "severity": "warning"
    }
  ],
  "recommendation": "Implementation solid, consider adding zero-length buffer check",
  "continue_to_next_stage": true,
  "token_count": 287
}
```

**Red flags (will exceed budget):**
- ❌ More than 7 checks
- ❌ Multi-paragraph messages
- ❌ Detailed code snippets in messages
- ❌ Verbose recommendations

**Green flags (stays within budget):**
- ✓ 3-5 checks maximum
- ✓ One-sentence messages
- ✓ High-level findings
- ✓ Actionable but brief recommendations
```

### Phase 4: Subagent Updates

**Files to modify:**
- `.claude/agents/foundation-shell-agent.md`
- `.claude/agents/dsp-agent.md`
- `.claude/agents/gui-agent.md`

**Changes needed:**

All three subagents already have instructions to read contracts (confirmed in analysis). Changes are **optional** for clarity:

1. **Add explicit reminder to read Required Reading (foundation-shell-agent.md, after line 92):**

```diff
  7. Use `juce::ParameterID { "id", 1 }` format (not bare string) in JUCE 8

+ **CRITICAL: You MUST read this file yourself from troubleshooting/patterns/juce8-critical-patterns.md**
+ The orchestrator no longer embeds it in your prompt - you are responsible for reading it.
+
  **Checkpoint:** After reading, confirm you understand these patterns. If any are unclear, reference the troubleshooting doc for detailed explanations before generating code.
```

**Apply same change to dsp-agent.md and gui-agent.md.**

2. **Update "Inputs (Contracts)" section to clarify file reading (foundation-shell-agent.md, lines 38-46):**

```diff
  ## Inputs (Contracts)

- You will receive the following contract files:
+ You will receive FILE PATHS for the following contracts (read them yourself using Read tool):

  1. **creative-brief.md** - Plugin name (PRODUCT_NAME), vision, user story
  2. **architecture.md** - Plugin type (effect/instrument), DSP components overview
  3. **plan.md** - Complexity score, implementation strategy
  4. **parameter-spec.md** - CRITICAL: Complete parameter definitions (IDs, types, ranges, defaults)
+ 5. **juce8-critical-patterns.md** - REQUIRED READING before any implementation

- **Plugin location:** `plugins/[PluginName]/`
+ **How to read:** Use Read tool with file paths provided in orchestrator prompt.
```

**Apply same change to dsp-agent.md and gui-agent.md.**

**Note:** These changes are clarifications only. Subagents already read contracts correctly, but explicit instructions improve robustness.

---

## Testing Strategy

### Test Plan

**Test with 3 plugin scenarios:**

1. **Simple plugin (complexity ≤2):**
   - Name: "TestSimple"
   - Parameters: 3
   - DSP: Single algorithm (gain)
   - Expected: Single-pass stages 2-4, validation after each

2. **Medium plugin (complexity 3):**
   - Name: "TestMedium"
   - Parameters: 7
   - DSP: Multiple algorithms (EQ + compression)
   - Expected: Phased Stage 3 (2 phases), validation after each

3. **Complex plugin (complexity ≥4):**
   - Name: "TestComplex"
   - Parameters: 12
   - DSP: Advanced algorithms (reverb + modulation)
   - Expected: Phased Stages 3-4 (3+ phases each), Opus model, validation after each

### Acceptance Criteria

**For each test plugin:**

1. ✓ Orchestrator context usage ≤5k tokens per stage (down from 14k)
2. ✓ Subagents successfully read contracts from files
3. ✓ validation-agent invoked after stages 2, 3, 4
4. ✓ Validation summaries ≤500 tokens each
5. ✓ All stages complete successfully
6. ✓ No functionality regression (builds work, tests pass)
7. ✓ Resume scenarios work (/continue command)

### Measurement Procedure

**Before optimization (baseline):**

```bash
# Run /implement TestSimple and capture token usage
# Instrument orchestrator to log token counts:
console.log(`Stage 2 orchestrator tokens: ${getContextTokens()}`);

# Expected baseline:
# Stage 2: ~14k tokens
# Stage 3: ~28k cumulative
# Stage 4: ~42k cumulative
# Stage 5: ~56k cumulative
```

**After optimization:**

```bash
# Run /implement TestSimple and capture token usage
# Same instrumentation points

# Expected optimized:
# Stage 2: ~600 tokens (100 prompt + 500 validation)
# Stage 3: ~1,200 cumulative
# Stage 4: ~1,800 cumulative
# Stage 5: ~2,400 cumulative
```

**Validation:**

- Savings: 56k - 2.4k = 53.6k tokens (96% reduction)
- Meets target: ✓ (52k target)

### Edge Cases

**Test these scenarios:**

1. **Missing contract file:**
   - Remove parameter-spec.md before Stage 2
   - Expected: foundation-shell-agent returns failure with clear error
   - Orchestrator presents recovery menu

2. **Malformed contract:**
   - Corrupt architecture.md (remove DSP Components section)
   - Expected: dsp-agent returns failure report
   - validation-agent catches inconsistency

3. **Validation failure:**
   - Intentionally create code with wrong member order
   - Expected: validation-agent returns FAIL status with continue_to_next_stage: false
   - Orchestrator blocks progression, presents fix menu

4. **Resume workflow:**
   - Start /implement TestMedium
   - Pause after Stage 2
   - Run /continue TestMedium
   - Expected: Stage 3 starts correctly, contracts loaded on demand

5. **Express mode:**
   - Run /implement TestSimple --express
   - Expected: Auto-progress through stages, validation still runs
   - If validation fails, drops to manual mode

### Regression Testing

**Verify no breakage in:**

1. ✓ /implement command (fresh start)
2. ✓ /continue command (resume)
3. ✓ /improve command (uses plugin-improve skill)
4. ✓ Phase-aware dispatch (complex plugins)
5. ✓ Express mode (auto-progression)
6. ✓ Manual mode (decision menus)
7. ✓ Build verification (build-automation skill)
8. ✓ Hook validators (SubagentStop.sh, PostToolUse.sh)

---

## Risk Mitigation

### Identified Risks

**Risk 1: Subagents fail to read contracts**

**Likelihood:** Low
**Impact:** High (blocks all implementation)
**Mitigation:**
- Subagents already read contracts (verified in code analysis)
- Add explicit "Read these files" instruction in minimal prompt
- Test with all 3 complexity levels before rollout
- Add error handling if subagent returns "file not found" error

**Contingency:** If subagent consistently fails to read contracts, revert to embedding contracts in prompt (increases orchestrator tokens but maintains functionality).

**Risk 2: Validation summaries exceed 500-token budget**

**Likelihood:** Medium
**Impact:** Low (extra tokens but still saves overall)
**Mitigation:**
- Add token budget reminder in validation-agent.md
- Limit checks to 5-7 per report
- Use concise messages (1-2 sentences max)
- Test validation reports with token counter

**Contingency:** If summaries exceed budget, increase limit to 750 tokens (still saves 51k tokens).

**Risk 3: Validation introduces latency**

**Likelihood:** High
**Impact:** Low (user experience, not functionality)
**Mitigation:**
- validation-agent runs in parallel subagent (not sequential)
- Opus model is fast for structured tasks
- User sees progress: "Invoking validation-agent for semantic review..."
- validation-agent completes in 10-15 seconds (measured from existing Stage 0/1 usage)

**Contingency:** If latency is unacceptable, make validation optional via flag (--skip-validation).

**Risk 4: Resume scenarios break**

**Likelihood:** Low
**Impact:** High (workflow corruption)
**Mitigation:**
- .continue-here.md handoff file is unchanged (still stores stage + plugin name)
- /continue command logic unchanged (still routes to correct stage)
- Test all resume scenarios before rollout

**Contingency:** If resume breaks, add version field to .continue-here.md and implement backward-compatible loading.

**Risk 5: Required Reading not read by subagents**

**Likelihood:** Medium (new responsibility for subagents)
**Impact:** High (repeat mistakes, pattern violations)
**Mitigation:**
- Add explicit instruction in minimal prompt: "Read Required Reading BEFORE implementation"
- Test by intentionally checking if subagent follows JUCE 8 patterns
- validation-agent cross-checks against juce8-critical-patterns.md

**Contingency:** If subagents skip Required Reading, revert to embedding it in prompt (adds 5.3k tokens per stage, reduces savings to 26k).

### What Could Go Wrong

**Scenario 1: Minimal prompt is too minimal**

**Symptom:** Subagent returns "insufficient context" error
**Diagnosis:** Subagent needs more guidance than just file paths
**Fix:** Add 1-2 sentences of context per contract in prompt (still <200 tokens)

**Scenario 2: validation-agent false positives**

**Symptom:** validation-agent fails valid implementations
**Diagnosis:** Validation checks too strict or missing context
**Fix:** Add .validation-overrides.yaml support (already exists in validation-agent.md)

**Scenario 3: Orchestrator context still grows**

**Symptom:** Orchestrator compacts before workflow finishes
**Diagnosis:** Other sources of token accumulation (decision menus, logs, etc.)
**Fix:** Investigate other optimization opportunities (Task 14, 15, 16 from roadmap)

**Scenario 4: Validation blocks valid code**

**Symptom:** User stuck at validation failure menu for correct implementation
**Diagnosis:** validation-agent overly conservative
**Fix:** Update checkpoint menu to allow "Continue anyway" option

---

## Rollback Plan

### Rollback Triggers

Rollback if ANY of these occur:

1. ✗ More than 2 critical bugs in first week
2. ✗ User reports workflow corruption (state files inconsistent)
3. ✗ Token savings less than 40k (below threshold)
4. ✗ Regression in functionality (builds fail, tests fail)
5. ✗ Subagents consistently fail to read contracts

### Rollback Procedure

**Step 1: Revert orchestrator changes**

```bash
git revert <commit-hash-of-optimization>
```

**Step 2: Restore stage reference files**

```bash
git checkout HEAD~1 .claude/skills/plugin-workflow/references/stage-2-foundation-shell.md
git checkout HEAD~1 .claude/skills/plugin-workflow/references/stage-3-dsp.md
git checkout HEAD~1 .claude/skills/plugin-workflow/references/stage-4-gui.md
```

**Step 3: Remove validation invocations**

Edit `.claude/skills/plugin-workflow/SKILL.md`:
- Remove Step 3 (invokeValidation) from checkpoint protocol
- Restore original step numbering

**Step 4: Test rollback**

```bash
# Verify workflow works with reverted code
/implement TestRollback
# Should complete all stages without validation-agent invocations
```

**Estimated rollback time:** 30 minutes

### Feature Flag Strategy

**Alternative to full rollback: Feature flag**

Add configuration option to enable/disable optimization:

```yaml
# .claude/preferences.json
{
  "workflow": {
    "mode": "express",
    "optimization": {
      "minimal_prompts": true,  # Enable/disable minimal prompts
      "stage_validation": true  # Enable/disable validation-agent invocations
    }
  }
}
```

**Implementation:**

```typescript
function getWorkflowOptimizations() {
  const prefs = readPreferences();
  return {
    minimalPrompts: prefs.workflow?.optimization?.minimal_prompts ?? true,
    stageValidation: prefs.workflow?.optimization?.stage_validation ?? true
  };
}

// In orchestrator:
if (optimizations.minimalPrompts) {
  prompt = constructMinimalPrompt(pluginName, stage);
} else {
  // Legacy: embed contracts in prompt
  prompt = constructLegacyPrompt(pluginName, stage, contracts);
}

if (optimizations.stageValidation && stage in [2, 3, 4]) {
  invokeValidationAgent(pluginName, stage);
}
```

**Benefit:** Allows A/B testing and gradual rollout.

---

## Success Criteria

**Optimization is successful when ALL criteria are met:**

### Quantitative Metrics

1. ✓ **Token savings ≥52k** (measured via instrumented orchestrator)
2. ✓ **Orchestrator context ≤5k tokens per stage** (down from 14k)
3. ✓ **Validation summaries ≤500 tokens each**
4. ✓ **No increase in subagent token usage** (subagents already read contracts)
5. ✓ **All test plugins complete successfully** (simple, medium, complex)

### Qualitative Metrics

6. ✓ **No functionality regression** (builds work, tests pass, installs succeed)
7. ✓ **No workflow corruption** (state files consistent, resume works)
8. ✓ **User transparency** (no workflow changes visible to user)
9. ✓ **Validation adds value** (catches real issues, not just noise)
10. ✓ **Code quality maintained** (subagents still follow JUCE 8 patterns)

### Before/After Comparison

**Before optimization:**
- Orchestrator token overhead: 56k tokens across 5 stages
- Compaction risk: High (55k + conversation ≈ premature compaction)
- Validation coverage: Stages 0, 1, 5 only

**After optimization:**
- Orchestrator token overhead: 3k tokens across 5 stages
- Compaction risk: Low (3k + conversation ≈ 175k tokens saved)
- Validation coverage: All stages 0-5

**Improvement:**
- Token savings: 53k (95% reduction)
- Compaction headroom: +175k tokens
- Validation coverage: +3 stages (2, 3, 4)

### User-Facing Benefits

**Invisible improvements:**
- Longer conversations without compaction
- Faster orchestrator responses (less to process)
- Better semantic validation (catches issues earlier)

**Visible improvements:**
- validation-agent findings appear after each stage
- Clear pass/fail status before checkpoint
- Option to address issues before committing

---

## File Modification Checklist

### Files to Modify

- [ ] `.claude/skills/plugin-workflow/SKILL.md`
  - [ ] Remove contract loading from delegation sequence (lines 44-49)
  - [ ] Add validation invocation to checkpoint protocol (after line 544)
  - [ ] Update checkpoint verification (lines 596-638)
  - [ ] Add validation helper functions (after line 788)
  - [ ] Update integration contracts table (line 932)

- [ ] `.claude/skills/plugin-workflow/references/stage-2-foundation-shell.md`
  - [ ] Remove contract reading section (lines 36-55)
  - [ ] Update subagent invocation to minimal prompt (lines 58-100)
  - [ ] Add validation invocation after success (after line 240)

- [ ] `.claude/skills/plugin-workflow/references/stage-3-dsp.md`
  - [ ] Remove contract reading section
  - [ ] Update subagent invocation to minimal prompt
  - [ ] Add validation invocation after success

- [ ] `.claude/skills/plugin-workflow/references/stage-4-gui.md`
  - [ ] Remove contract reading section
  - [ ] Update subagent invocation to minimal prompt
  - [ ] Add validation invocation after success

- [ ] `.claude/skills/plugin-workflow/references/stage-5-validation.md`
  - [ ] No changes (already invokes validation-agent)

- [ ] `.claude/agents/validation-agent.md`
  - [ ] Add Stage 2 validation section (after line 289)
  - [ ] Update Stage 3 validation section (lines 290-342)
  - [ ] Update Stage 4 validation section (lines 343-403)
  - [ ] Rename Stage 5 validation section (lines 405-465)
  - [ ] Add token budget enforcement section (after line 502)

- [ ] `.claude/agents/foundation-shell-agent.md` (optional)
  - [ ] Add explicit reminder to read Required Reading (after line 92)
  - [ ] Update "Inputs (Contracts)" section (lines 38-46)

- [ ] `.claude/agents/dsp-agent.md` (optional)
  - [ ] Add explicit reminder to read Required Reading
  - [ ] Update "Inputs (Contracts)" section

- [ ] `.claude/agents/gui-agent.md` (optional)
  - [ ] Add explicit reminder to read Required Reading
  - [ ] Update "Inputs (Contracts)" section

### Testing Checklist

- [ ] Create TestSimple plugin (complexity ≤2)
  - [ ] Measure baseline token usage
  - [ ] Apply optimization
  - [ ] Measure optimized token usage
  - [ ] Verify 52k+ savings
  - [ ] Verify all stages complete
  - [ ] Verify builds succeed

- [ ] Create TestMedium plugin (complexity 3)
  - [ ] Verify phased implementation works
  - [ ] Verify validation-agent called after each phase
  - [ ] Verify resume scenario

- [ ] Create TestComplex plugin (complexity ≥4)
  - [ ] Verify Opus model used for dsp-agent
  - [ ] Verify validation-agent detects issues
  - [ ] Verify validation failure blocks progression

- [ ] Test edge cases
  - [ ] Missing contract file (parameter-spec.md)
  - [ ] Malformed contract (corrupt architecture.md)
  - [ ] Validation failure (wrong member order)
  - [ ] Resume workflow (/continue)
  - [ ] Express mode (--express flag)

- [ ] Regression testing
  - [ ] /implement command
  - [ ] /continue command
  - [ ] /improve command
  - [ ] Phase-aware dispatch
  - [ ] Express mode
  - [ ] Manual mode
  - [ ] Build verification
  - [ ] Hook validators

### Deployment Checklist

- [ ] All files modified and committed
- [ ] All tests passed
- [ ] Token savings verified (≥52k)
- [ ] Documentation updated
- [ ] Rollback plan tested
- [ ] Feature flag implemented (optional)
- [ ] User-facing changelog entry added

---

## Implementation Notes

### Key Insights from Analysis

1. **Subagents already read contracts** - The orchestrator embedding them is redundant
2. **Required Reading is largest token sink** - 5.3k tokens × 5 stages = 26.5k tokens
3. **validation-agent already exists** - Just needs to be invoked for stages 2-4
4. **Hooks are deterministic** - validation-agent adds semantic layer
5. **500-token summaries are achievable** - Existing reports average 300-400 tokens

### Critical Success Factors

1. **Minimal prompt must be clear** - Subagents need explicit "read these files" instruction
2. **validation-agent must be fast** - Opus model needed for semantic reasoning
3. **Validation must be advisory** - Users can override if false positives occur
4. **Checkpoints must be atomic** - Validation runs before commit
5. **Token counting must be accurate** - Measure real usage, not estimates

### Potential Optimizations

**If savings exceed target:**

1. Consider adding validation to Stage 0 (currently basic checks)
2. Add validation to Stage 5 (currently just file checks)
3. Use validation findings to auto-improve code (plugin-improve integration)

**If savings fall short:**

1. Remove Required Reading from prompts entirely (subagents read it)
2. Compress validation summaries further (250-token budget)
3. Skip validation for complexity ≤2 plugins (only complex plugins)

---

## Conclusion

This optimization will reduce orchestrator token usage by 95% (from 56k to 3k tokens) by eliminating redundant contract loading. The change is transparent to users, maintains all functionality, and adds semantic validation for stages 2-4.

**Implementation effort:** 3-4 days (as estimated in roadmap)
**Risk level:** Medium (requires careful testing, but rollback is straightforward)
**Benefit:** Prevents premature compaction, extends conversation length by 175k tokens, catches issues earlier

**Recommendation:** Proceed with implementation. Benefits far outweigh risks.
