# State Management and Workflow Functions

**Context:** This file is part of the plugin-workflow skill.
**Invoked by:** All stages for state updates, git commits, and handoff management
**Purpose:** Centralized state machine functions, git operations, decision menus, and handoff file management

---

## State Machine Functions

### validateRegistryConsistency(pluginName)

**Purpose:** Detect registry table drift by comparing status between registry table and full entry.

**Implementation:**
```bash
#!/bin/bash
# Verify registry table matches full entry section

PLUGIN_NAME=$1

# Extract status from registry table (format: | PluginName | Status | Version | Date |)
TABLE_STATUS=$(grep "^| ${PLUGIN_NAME} |" PLUGINS.md | awk -F'|' '{print $3}' | xargs)

# Extract status from full entry (after ### PluginName header)
ENTRY_STATUS=$(grep -A 10 "^### ${PLUGIN_NAME}$" PLUGINS.md | grep "^\*\*Status:\*\*" | sed 's/\*\*//g; s/Status://g' | xargs)

# Normalize for comparison (remove emojis, trim whitespace)
TABLE_NORMALIZED=$(echo "$TABLE_STATUS" | sed 's/[^a-zA-Z0-9 ]//g' | xargs)
ENTRY_NORMALIZED=$(echo "$ENTRY_STATUS" | sed 's/[^a-zA-Z0-9 ]//g' | xargs)

# Compare
if [ "$TABLE_NORMALIZED" != "$ENTRY_NORMALIZED" ]; then
  echo "ERROR: Registry drift detected for ${PLUGIN_NAME}"
  echo "  Registry table: ${TABLE_STATUS}"
  echo "  Full entry: ${ENTRY_STATUS}"
  return 1
fi

return 0
```

**When to call:**
- After every `updatePluginStatus()` call (validates atomic update succeeded)
- At checkpoint verification (step 3.5 in checkpoint protocol)
- Before reading status in `/implement` command

**Returns:**
- Exit 0: Both locations match (consistent)
- Exit 1: Mismatch detected (drift)

**Error handling:**
If drift detected, present retry menu:
```
Registry drift detected - inconsistent status

What would you like to do?
1. Auto-fix (sync registry table to match full entry) - recommended
2. Show both values (investigate mismatch)
3. Manual fix (I'll fix it myself)
4. Other
```

### updatePluginStatus(pluginName, newStatus)

**Purpose:** Update plugin status emoji in PLUGINS.md in BOTH locations atomically.

**Valid statuses for plugin-workflow (stages 2-5):**
- `🔨 Building System` - Build system and parameters in progress (Stage 2 internal)
- `🎵 Processing Audio` - DSP implementation in progress (Stage 3 internal)
- `🎨 Designing Interface` - UI integration in progress (Stage 4 internal)
- `✅ Validating` - Final validation and polish (Stage 5 internal)
- `✅ Ready to Install` - Validation complete, not deployed
- `📦 Installed` - Deployed to system folders

**Note:** Statuses `💡 Concept Ready` and planning statuses are managed by plugin-planning skill.
**Internal:** Stage numbers (2-5) are used internally but never shown to users.

**Implementation (ATOMIC - updates both locations):**

**APPROACH 1 (Using Edit tool - recommended):**
```
1. Read PLUGINS.md to get current state
2. Use Edit tool to update full entry section:
   - Find section: ### [pluginName]
   - Update line: **Status:** [old] → **Status:** [new]
3. Use Edit tool to update registry table:
   - Find line: | [pluginName] | [old status] | ...
   - Update to: | [pluginName] | [new status] | ...
4. Call validateRegistryConsistency(pluginName)
5. If validation fails: Rollback via git checkout PLUGINS.md
6. Return success/failure
```

**APPROACH 2 (Using bash sed - for scripting contexts):**
```bash
#!/bin/bash
# Atomic status update - updates both locations or neither

PLUGIN_NAME=$1
NEW_STATUS=$2

# Backup current state
cp PLUGINS.md PLUGINS.md.backup

# Update full entry section (canonical source)
sed -i '' "/^### ${PLUGIN_NAME}$/,/^###/ s/^\*\*Status:\*\* .*$/\*\*Status:\*\* ${NEW_STATUS}/" PLUGINS.md

# Update registry table (derived view)
# Extract current row and preserve other columns
CURRENT_ROW=$(grep "^| ${PLUGIN_NAME} |" PLUGINS.md)
NEW_ROW=$(echo "$CURRENT_ROW" | awk -F'|' -v status=" ${NEW_STATUS} " '{print $1 "|" $2 "|" status "|" $4 "|" $5}')
sed -i '' "s/^| ${PLUGIN_NAME} | .*/$(echo "$NEW_ROW" | sed 's/[&/\]/\\&/g')/" PLUGINS.md

# Validate both updates succeeded
if validateRegistryConsistency "$PLUGIN_NAME"; then
  rm PLUGINS.md.backup
  echo "✓ Status updated atomically: ${NEW_STATUS}"
  return 0
else
  echo "ERROR: Atomic update failed. Reverting..."
  mv PLUGINS.md.backup PLUGINS.md
  return 1
fi
```

**CRITICAL:** This function ensures BOTH locations update together (atomic operation).

**Validation:**
- After update, `validateRegistryConsistency()` must return 0
- If validation fails, rollback changes (all or nothing)

**Example:**
```markdown
Before:
  ### TapeDelay
  **Status:** 🚧 Stage 4

  Registry table:
  | TapeDelay | 🚧 Stage 4 | 1.0.0 | 2025-11-12 |

After:
  ### TapeDelay
  **Status:** 🚧 Stage 5

  Registry table:
  | TapeDelay | 🚧 Stage 5 | 1.0.0 | 2025-11-12 |
```

### createPluginEntry(pluginName, type, brief)

**Purpose:** Create initial PLUGINS.md entry when starting new plugin.

**Implementation:**
1. Read PLUGINS.md
2. Check if entry already exists (search for `### [pluginName]`)
3. If not exists, append new entry:
   ```markdown
   ### [pluginName]
   **Status:** 💡 Ideated
   **Type:** [Audio Effect | MIDI Instrument | Synth]
   **Created:** [YYYY-MM-DD]

   [Brief description from creative-brief.md]

   **Lifecycle Timeline:**
   - **[YYYY-MM-DD]:** Creative brief created

   **Last Updated:** [YYYY-MM-DD]
   ```
4. Write back to PLUGINS.md

### updatePluginTimeline(pluginName, stage, description)

**Purpose:** Add timeline entry to PLUGINS.md when stage completes.

**Implementation:**
1. Read PLUGINS.md
2. Find plugin entry
3. Find `**Lifecycle Timeline:**` section
4. Append new entry:
   ```markdown
   - **[YYYY-MM-DD] (Stage N):** [description]
   ```
5. Update `**Last Updated:**` field
6. Write back to PLUGINS.md

### getPluginStatus(pluginName)

**Purpose:** Return current status emoji for routing logic. Reads from FULL ENTRY (canonical source).

**Implementation (ROBUST - handles Markdown formatting):**
```bash
#!/bin/bash
# Parse status from full entry section (canonical source)

PLUGIN_NAME=$1

# Extract status from full entry (handles **bold**, _italic_, etc.)
STATUS_LINE=$(grep -A 10 "^### ${PLUGIN_NAME}$" PLUGINS.md | grep "^\*\*Status:\*\*" | head -1)

# Strip all Markdown formatting
STATUS_TEXT=$(echo "$STATUS_LINE" | sed 's/\*\*//g; s/__//g; s/_//g; s/Status://g' | xargs)

# Parse emoji and stage
EMOJI=$(echo "$STATUS_TEXT" | awk '{print $1}')
STAGE_INFO=$(echo "$STATUS_TEXT" | sed 's/^[^ ]* //')

# For 🚧 status, extract stage/phase numbers
if echo "$EMOJI" | grep -q "🚧"; then
  # Handle both "Stage 4" and "Stage 4.1" formats
  STAGE_NUM=$(echo "$STAGE_INFO" | grep -o 'Stage [0-9.]*' | awk '{print $2}')
  echo "{ \"emoji\": \"$EMOJI\", \"stage\": \"$STAGE_NUM\", \"text\": \"$STATUS_TEXT\" }"
else
  echo "{ \"emoji\": \"$EMOJI\", \"text\": \"$STATUS_TEXT\" }"
fi
```

**Key improvements:**
1. Reads from full entry section (canonical source, not registry table)
2. Strips ALL Markdown formatting (`**bold**`, `__underline__`, `_italic_`)
3. Handles phased stages (e.g., "Stage 4.1")
4. Returns structured JSON for easy parsing

**Example outputs:**
```bash
# Stage in progress
{ "emoji": "🚧", "stage": "4", "text": "🚧 Stage 4" }

# Phased stage
{ "emoji": "🚧", "stage": "4.1", "text": "🚧 Stage 4.1" }

# Completed
{ "emoji": "✅", "text": "✅ Working" }

# Installed
{ "emoji": "📦", "text": "📦 Installed" }
```

**Why read from full entry:**
- Full entry is canonical source (more detailed, always updated first)
- Registry table is derived view (for quick scanning only)
- Avoids false-negatives from registry drift

### validateTransition(currentStatus, newStatus)

**Purpose:** Enforce legal state machine transitions for plugin-workflow (stages 2-6).

**Legal transitions for plugin-workflow:**
```
🚧 Stage 0 → 🚧 Stage 2 (start implementation after planning)
🚧 Stage N → 🚧 Stage N+1 (sequential stages 2-6)
🚧 Stage N → 🚧 Stage N.M (enter phased implementation)
🚧 Stage N.M → 🚧 Stage N.M+1 (next phase within stage)
🚧 Stage N.M → 🚧 Stage N+1 (complete phased stage)
🚧 Stage 6 → ✅ Working (validation complete)
✅ Working → 📦 Installed (install plugin)
📦 Installed → 🚧 Improving (start improvement)
🚧 Improving → 📦 Installed (improvement complete)
```

**Illegal transitions:**
```
🚧 Stage 2 → 🚧 Stage 5 (can't skip stages 3-4)
🚧 Stage 4 → 🚧 Stage 2 (can't go backward)
✅ Working → 🚧 Stage 3 (use /improve instead)
```

**Note:** Transitions involving `💡 Ideated` and `🚧 Stage 0-1` are managed by plugin-planning skill.

**Implementation:**
1. Parse current and new status
2. Check transition against rules
3. Return: `{ allowed: true }` or `{ allowed: false, reason: "..." }`


## Interactive Decision Menu System

### presentDecisionMenu(context)

**Purpose:** Present context-aware decision menu at every checkpoint.

**Context parameters:**
- `stage`: Current stage number (0-6)
- `completionStatement`: What was just accomplished
- `pluginName`: Plugin being worked on
- `errors`: Any errors/failures (optional)
- `options`: Custom options (optional)

<decision_menu_format>
  <display_format type="inline_numbered_list" forbidden_tool="AskUserQuestion">
    Structure:
    ```
    ✓ [Completion statement]

    What's next?
    1. [Primary action] (recommended)
    2. [Secondary action]
    3. [Discovery option] ← User discovers [feature]
    4. [Alternative path]
    5. [Escape hatch]
    6. Other

    Choose (1-6): _
    ```
  </display_format>

  <rendering_sequence enforce_order="true">
    <step order="1">Load context-appropriate options from assets/decision-menus.json</step>
    <step order="2">Format as inline numbered list (see format above)</step>
    <step order="3">Display to user</step>
    <step order="4" blocking="true">Wait for response (number, keyword, or "Other")</step>
    <step order="5">Parse response and validate</step>
    <step order="6">
      IF valid: Execute chosen action
      ELSE: Display error and re-present menu
    </step>
  </rendering_sequence>

  <keyword_shortcuts>
    - "continue" → Option 1 (primary action)
    - "pause" → Pause option (creates checkpoint)
    - "review" → Review option (show code/context)
  </keyword_shortcuts>

  <other_handling>
    When user selects "Other":
    1. Prompt: "What would you like to do?"
    2. Accept free-form text
    3. Process custom request
    4. Re-present decision menu after completing request
  </other_handling>

  <progressive_disclosure>
    Use discovery markers to surface hidden features:
    - "Save as template ← Add to UI template library"
    - "Design sync ← Validate brief matches mockup"
    - "/research ← Deep investigation for complex problems"
  </progressive_disclosure>
</decision_menu_format>

### generateContextualOptions(context)

**Purpose:** Generate situation-specific menu options after subagent completion.

Load menu configurations from `assets/decision-menus.json` and customize based on context (stage number, success/failure, phase information).

**Progressive disclosure:** Menu options include discovery markers (←) to surface features like deep-research, UI template library, design sync validation.

### formatDecisionMenu(completionStatement, options)

**Purpose:** Format options as inline numbered list.

**Implementation:**
```
output = `✓ ${completionStatement}\n\n`
output += `What's next?\n`

options.forEach((opt, i) => {
  output += `${i+1}. ${opt.label}`

  if (opt.recommended) {
    output += ` (recommended)`
  }

  if (opt.discovery) {
    output += ` ← ${opt.discovery}`
  }

  output += `\n`
})

output += `\nChoose (1-${options.length}): _`

return output
```

**Progressive Disclosure:**
Use discovery markers (`← User discovers [feature]`) to surface hidden capabilities:
- "Save as template ← Add to UI template library"
- "Design sync ← Validate brief matches mockup"
- "/research ← Deep investigation for complex problems"

### handleMenuChoice(choice, options, context)

**Purpose:** Parse user response and execute chosen action.

**Implementation:**
```javascript
// Parse response
if (isNumber(choice)) {
  const index = parseInt(choice) - 1
  if (index >= 0 && index < options.length) {
    return executeOption(options[index], context)
  } else {
    return { error: "Invalid choice", reprompt: true }
  }
}

// Handle keyword shortcuts
if (choice.toLowerCase() === "continue") {
  return executeOption(options[0], context) // First option
}

if (choice.toLowerCase() === "pause") {
  const pauseOption = options.find(o => o.label.includes("Pause"))
  return executeOption(pauseOption, context)
}

if (choice.toLowerCase() === "review") {
  const reviewOption = options.find(o => o.label.includes("Review"))
  return executeOption(reviewOption, context)
}

// Handle "Other"
if (choice.toLowerCase() === "other" || options[choice - 1].label === "Other") {
  return { action: "ask_freeform", reprompt: true }
}
```

**After executing action:**
- Re-present menu if action was exploratory (review, show code)
- Continue workflow if action was directive (continue, pause)

## Git Commit Functions

### commitStage(pluginName, stage, description)

**Purpose:** Create standardized git commit after stage completion.

**Commit message format:**
```
feat: [PluginName] Stage [N] - [description]

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
```

**For phased stages:**
```
feat: [PluginName] Stage [N.M] - [phase description]

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
```

**Implementation:**

**CRITICAL:** All state files must be updated and committed in a SINGLE ATOMIC OPERATION to prevent temporal drift windows.

1. Update all state files BEFORE staging:
   ```bash
   # Update .continue-here.md with new stage info
   # Update PLUGINS.md with new status
   # Update plan.md if phased implementation
   ```

2. Stage ALL changes atomically (code + state files):
   ```bash
   git add plugins/[PluginName]/Source/ 2>/dev/null || true
   git add plugins/[PluginName]/.ideas/
   git add plugins/[PluginName]/.continue-here.md
   git add plugins/[PluginName]/plan.md 2>/dev/null || true
   git add PLUGINS.md
   ```

3. Create SINGLE atomic commit with all changes:
   ```bash
   git commit -m "$(cat <<'EOF'
   feat: [PluginName] Stage [N] - [description]

   [ATOMIC] Code + state files committed together

   🤖 Generated with Claude Code

   Co-Authored-By: Claude <noreply@anthropic.com>
   EOF
   )"
   ```

4. Verify commit succeeded:
   ```bash
   git log -1 --format='%h'
   ```

5. Display commit hash to user:
   ```
   ✓ Committed: abc1234 - Stage [N] complete (atomic)
   ```

6. If commit fails:
   - Rollback state file changes (restore from git)
   - Warn user about inconsistency
   - Suggest manual resolution
   - Continue workflow (don't block)

**Atomic state transitions:**
- PLUGINS.md + .continue-here.md + plan.md + code = SINGLE commit
- NO temporal window between state updates
- If commit fails → Rollback ALL state changes to maintain consistency

**Commit variations by stage:**
- Stage 0: `feat: [Plugin] research & planning complete`
- Stage 2: `feat: [Plugin] build system ready`
- Stage 3: `feat: [Plugin] audio engine working`
- Stage 3.1: `feat: [Plugin] audio phase 3.1 - core processing`
- Stage 3.2: `feat: [Plugin] audio phase 3.2 - parameter modulation`
- Stage 4: `feat: [Plugin] UI integrated`
- Stage 4.1: `feat: [Plugin] UI phase 4.1 - layout and bindings`
- Stage 4.2: `feat: [Plugin] UI phase 4.2 - visual polish`
- Stage 5: `feat: [Plugin] validation complete`

**Note:** Commit messages use milestone language internally but still include stage numbers for technical tracking.

### verifyGitAvailable()

**Purpose:** Check git is available before workflow starts.

**Implementation:**
```bash
if ! command -v git &> /dev/null; then
    echo "⚠️ Warning: git not found. Commits will be skipped."
    echo "Install git to enable automatic commit workflow."
    return false
fi

if ! git rev-parse --git-dir &> /dev/null; then
    echo "⚠️ Warning: Not a git repository. Commits will be skipped."
    echo "Run 'git init' to enable automatic commit workflow."
    return false
fi

return true
```

Call at beginning of Stage 0.

## Handoff Management Functions

### createHandoff(pluginName, stage, context)

**Purpose:** Create initial handoff file after Stage 0 completion.

**Implementation:**
1. Read handoff template from `.claude/skills/plugin-workflow/assets/continue-here-template.md`
2. Calculate contract checksums for tamper detection:
   ```bash
   # Calculate SHA256 checksums of contract files
   BRIEF_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/creative-brief.md | awk '{print $1}')
   PARAM_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/parameter-spec.md | awk '{print $1}')
   ARCH_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/architecture.md | awk '{print $1}')
   PLAN_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/plan.md | awk '{print $1}' 2>/dev/null || echo "null")
   ```
3. Fill in YAML frontmatter:
   - plugin: [pluginName]
   - stage: [stage number]
   - phase: null (only for complex plugins)
   - status: "in_progress"
   - last_updated: [current timestamp]
   - complexity_score: null (filled in Stage 0)
   - phased_implementation: null (filled in Stage 0)
   - orchestration_mode: true (enable dispatcher pattern)
   - next_action: null (filled when stage/phase completes)
   - next_phase: null (filled for phased implementations)
   - contract_checksums:
     * creative_brief: sha256:[checksum]
     * parameter_spec: sha256:[checksum]
     * architecture: sha256:[checksum]
     * plan: sha256:[checksum] (or null if doesn't exist yet)
4. Fill in markdown sections with context:
   - Current State: "Stage [N] - [description]"
   - Completed So Far: [what's done]
   - Next Steps: [prioritized actions]
   - Context to Preserve: [key decisions, files, build status]
5. Write to `plugins/[pluginName]/.continue-here.md`

### updateHandoff(pluginName, stage, completed, nextSteps, complexityScore, phased, nextAction, nextPhase)

**Purpose:** Update handoff file after each stage/phase completion.

**Implementation:**
1. Read existing `plugins/[pluginName]/.continue-here.md`
2. Recalculate contract checksums to detect tampering:
   ```bash
   # Recalculate SHA256 checksums
   BRIEF_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/creative-brief.md | awk '{print $1}')
   PARAM_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/parameter-spec.md | awk '{print $1}')
   ARCH_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/architecture.md | awk '{print $1}')
   PLAN_SHA=$(shasum -a 256 plugins/$PLUGIN_NAME/.ideas/plan.md | awk '{print $1}')
   ```
3. Update YAML frontmatter:
   - stage: [new stage number]
   - phase: [phase number if complex]
   - status: [in_progress | complete]
   - last_updated: [current timestamp]
   - complexity_score: [score if known]
   - phased_implementation: [true/false if known]
   - orchestration_mode: true (keep enabled for dispatcher pattern)
   - next_action: [e.g., "invoke_dsp_agent", "invoke_gui_agent"]
   - next_phase: [e.g., "4.4", "5.1"]
   - contract_checksums:
     * creative_brief: sha256:[new_checksum]
     * parameter_spec: sha256:[new_checksum]
     * architecture: sha256:[new_checksum]
     * plan: sha256:[new_checksum]
4. Append to "Completed So Far" section
5. Update "Next Steps" with new actions
6. Update "Context to Preserve" with latest context
7. Write back to file

**Determining next_action:**
- Stage 2 → "invoke_foundation_agent"
- Stage 3 → "invoke_shell_agent"
- Stage 4 → "invoke_dsp_agent"
- Stage 5 → "invoke_gui_agent"
- Stage 6 → "invoke_validator"
- Phased stages: specify exact phase (e.g., "4.4" for DSP phase 4)

### deleteHandoff(pluginName)

**Purpose:** Remove handoff file when plugin reaches ✅ Working or 📦 Installed.

**Implementation:**
1. Check if `plugins/[pluginName]/.continue-here.md` exists
2. Delete file
3. Log deletion (workflow complete)

**When to call:**
- After Stage 6 complete (status → ✅ Working)
- After plugin installation (status → 📦 Installed)

### verifyStateIntegrity(pluginName)

**Purpose:** Verify state consistency before dispatching to next stage. Prevents workflow resumption when state is corrupted or contracts have been tampered with.

**Implementation:**
```bash
#!/bin/bash
# State integrity verification before stage dispatch

PLUGIN_NAME=$1
HANDOFF_FILE="plugins/${PLUGIN_NAME}/.continue-here.md"

# Check if handoff file exists
if [ ! -f "$HANDOFF_FILE" ]; then
  echo "❌ No handoff file found for ${PLUGIN_NAME}"
  echo "Run /plan ${PLUGIN_NAME} to start workflow"
  exit 1
fi

# Extract current stage from handoff
HANDOFF_STAGE=$(grep "^stage:" "$HANDOFF_FILE" | awk '{print $2}')

# Extract status from PLUGINS.md
PLUGINS_STATUS=$(grep -A1 "^### ${PLUGIN_NAME}$" PLUGINS.md | grep "^**Status:**" | sed 's/.*Stage \([0-9]\+\).*/\1/')

# Verify stage consistency between handoff and PLUGINS.md
if [ "$HANDOFF_STAGE" != "$PLUGINS_STATUS" ]; then
  echo "❌ State mismatch detected:"
  echo "   .continue-here.md: Stage ${HANDOFF_STAGE}"
  echo "   PLUGINS.md: Stage ${PLUGINS_STATUS}"
  echo ""
  echo "Run /reconcile ${PLUGIN_NAME} to fix inconsistency"
  exit 2
fi

# Verify contract checksums (detect tampering)
echo "Verifying contract integrity..."

# Extract stored checksums from handoff
STORED_BRIEF=$(grep "creative_brief:" "$HANDOFF_FILE" | awk '{print $2}')
STORED_PARAM=$(grep "parameter_spec:" "$HANDOFF_FILE" | awk '{print $2}')
STORED_ARCH=$(grep "architecture:" "$HANDOFF_FILE" | awk '{print $2}')
STORED_PLAN=$(grep "plan:" "$HANDOFF_FILE" | awk '{print $2}')

# Calculate current checksums
CURRENT_BRIEF="sha256:$(shasum -a 256 plugins/${PLUGIN_NAME}/.ideas/creative-brief.md | awk '{print $1}')"
CURRENT_PARAM="sha256:$(shasum -a 256 plugins/${PLUGIN_NAME}/.ideas/parameter-spec.md | awk '{print $1}')"
CURRENT_ARCH="sha256:$(shasum -a 256 plugins/${PLUGIN_NAME}/.ideas/architecture.md | awk '{print $1}')"
CURRENT_PLAN="sha256:$(shasum -a 256 plugins/${PLUGIN_NAME}/.ideas/plan.md | awk '{print $1}')"

# Compare checksums
TAMPERED=false

if [ "$STORED_BRIEF" != "$CURRENT_BRIEF" ] && [ "$STORED_BRIEF" != "null" ]; then
  echo "⚠️  creative-brief.md has been modified"
  TAMPERED=true
fi

if [ "$STORED_PARAM" != "$CURRENT_PARAM" ] && [ "$STORED_PARAM" != "null" ]; then
  echo "⚠️  parameter-spec.md has been modified"
  TAMPERED=true
fi

if [ "$STORED_ARCH" != "$CURRENT_ARCH" ] && [ "$STORED_ARCH" != "null" ]; then
  echo "⚠️  architecture.md has been modified"
  TAMPERED=true
fi

if [ "$STORED_PLAN" != "$CURRENT_PLAN" ] && [ "$STORED_PLAN" != "null" ]; then
  echo "⚠️  plan.md has been modified"
  TAMPERED=true
fi

if [ "$TAMPERED" = true ]; then
  echo ""
  echo "⚠️  CONTRACT DRIFT DETECTED"
  echo "Contracts have been modified since last checkpoint."
  echo "This violates immutability principle - contracts must not change during implementation."
  echo ""
  echo "Options:"
  echo "1. Restore contracts from git (recommended)"
  echo "2. Update checksums and proceed (if changes were intentional)"
  echo "3. Use /improve instead of /continue (for post-completion changes)"
  exit 3
fi

# Check for stale handoffs from completed plugins
if grep -q "status: workflow_complete" "$HANDOFF_FILE"; then
  PLUGIN_STATUS=$(grep -A1 "^### ${PLUGIN_NAME}$" PLUGINS.md | grep "^**Status:**")

  if echo "$PLUGIN_STATUS" | grep -q "Working\|Installed"; then
    echo "⚠️  STALE HANDOFF DETECTED"
    echo "${PLUGIN_NAME} is marked as complete in PLUGINS.md but handoff file still exists."
    echo "This handoff should have been deleted at Stage 6 completion."
    echo ""
    echo "Cleaning up stale handoff..."
    rm "$HANDOFF_FILE"
    echo "✓ Handoff file deleted"
    exit 4
  fi
fi

echo "✓ State integrity verified"
exit 0
```

**When to call:**
- BEFORE dispatching to any stage (2-6)
- At workflow resume (/continue command)
- After manual code changes (user says "resume automation")

**Error codes:**
- Exit 0: All checks passed, safe to proceed
- Exit 1: Missing handoff file (run /plan first)
- Exit 2: State mismatch between .continue-here.md and PLUGINS.md (run /reconcile)
- Exit 3: Contract tampering detected (restore or update checksums)
- Exit 4: Stale handoff cleaned up (workflow already complete)

## Checkpoint Types

### Hard Checkpoints (MUST pause for user decision)

**All implementation stages are hard checkpoints:**
- Stage 2: Foundation complete (after foundation-agent returns)
- Stage 3: Shell complete (after shell-agent returns)
- Stage 4: DSP complete (after dsp-agent returns)
- Stage 5: GUI complete (after gui-agent returns)
- Stage 6: Validation complete

**Behavior (ENFORCED by orchestrator):**
1. Subagent completes and returns to orchestrator
2. Orchestrator auto-commits changes
3. Orchestrator updates handoff file
4. Orchestrator updates PLUGINS.md
5. Orchestrator presents decision menu
6. **Orchestrator WAITS for user response** - NEVER auto-continue
7. Orchestrator executes user choice

**Note:** Stage 0 checkpoints are managed by plugin-planning skill.

### Soft Checkpoints (can auto-continue with user permission)

**Phases within complex stages (complexity ≥3):**
- Stage 4.1, 4.2, 4.3: DSP phases (managed by dsp-agent)
- Stage 5.1, 5.2: GUI phases (managed by gui-agent)

**Behavior:**
1. Subagent completes phase
2. Subagent auto-commits changes
3. Subagent updates handoff file with phase info
4. Subagent presents decision menu: "Continue to next phase" (recommended)
5. If user chooses continue: subagent proceeds to next phase
6. If user chooses pause: subagent returns to orchestrator

### Decision Checkpoints

**Occur before significant choices:**
- Build failures (show 4-option menu)
- Validation failures (show 3-option menu)
- Manual pause requests

**Behavior:**
1. Subagent or orchestrator updates handoff with current context
2. Present situation-specific menu
3. Wait for user choice
4. Execute chosen path

## Resume Handling

**Support "resume automation" command:**

If user paused and says "resume automation" or chooses to continue:

1. Read `.continue-here.md` to determine current stage/phase
2. Parse YAML frontmatter for stage, phase, complexity_score, phased_implementation
3. Continue from documented "Next Steps"
4. Load relevant context (contracts, research, plan)

---

<stage_boundary_protocol>
  <trigger>When subagent completes and returns to orchestrator</trigger>

  <sequence enforce_order="true">
    <action order="1" actor="orchestrator">
      Display completion statement:
      "✓ Stage [N] complete: [description]"
    </action>

    <action order="2" actor="orchestrator" conditional="stages_4_5_6_only">
      Run automated tests:
      - Invoke plugin-testing skill
      - IF tests fail: STOP, show results, present investigation menu
      - IF tests pass: Continue to next action
    </action>

    <action order="3" actor="orchestrator" required="true">
      Update state files:
      - Update .continue-here.md:
        * stage: [new stage number]
        * phase: [phase number if complex]
        * last_updated: [timestamp]
        * next_action: [which subagent to invoke next]
        * next_phase: [phase number if phased implementation]
      - Update PLUGINS.md:
        * Status: 🚧 Stage [N]
        * Last Updated: [date]
        * Lifecycle Timeline: Append new entry
    </action>

    <action order="4" actor="orchestrator" required="true">
      ATOMIC commit (single operation for code + state):
      ```bash
      git add plugins/[PluginName]/Source/ 2>/dev/null || true
      git add plugins/[PluginName]/.ideas/
      git add plugins/[PluginName]/.continue-here.md
      git add plugins/[PluginName]/plan.md 2>/dev/null || true
      git add PLUGINS.md
      git commit -m "feat: [Plugin] Stage [N] - [description]

[ATOMIC] Code + state files committed together"
      ```
      IF commit fails: Rollback state changes, warn user, continue (non-blocking)
    </action>

    <action order="5" actor="orchestrator" required="true" blocking="true">
      Present decision menu with context-appropriate options
    </action>

    <action order="6" actor="orchestrator" required="true" blocking="true">
      WAIT for user response - NEVER auto-proceed
    </action>
  </sequence>

  <responsibilities>
    <actor name="subagent">
      - Complete stage work
      - Report completion status to orchestrator
      - Return control to orchestrator for checkpoint
      - NEVER commit changes
    </actor>

    <actor name="orchestrator">
      - Commit changes
      - Update state files (.continue-here.md, PLUGINS.md)
      - Present decision menu
      - Wait for user input
      - Invoke next subagent based on user choice
    </actor>
  </responsibilities>

  <critical_invariant>
    Orchestrator NEVER auto-proceeds to next stage without user confirmation.
  </critical_invariant>
</stage_boundary_protocol>

---

## Integration Points

**Invoked by:**

- `/implement` command
- `plugin-ideation` skill (after creative brief)
- `context-resume` skill (when resuming)

**Invokes:**

- `plugin-testing` skill (Stages 4, 5, 6)
- `plugin-lifecycle` skill (after Stage 6, if user chooses install)

**Creates:**

- `.continue-here.md` (handoff file)
- `architecture.md` (Stage 0 - DSP specification)
- `plan.md` (Stage 1)
- `CHANGELOG.md` (Stage 6)
- `Presets/` directory (Stage 6)

**Updates:**

- PLUGINS.md (status changes throughout)

---

## Error Handling

**If contract files missing at Stage 1:**
Block and guide to create UI mockup first.

**If build fails at any stage:**
Present menu:

```
Build error at [stage]:
[Error context]

What would you like to do?
1. Investigate (triggers deep-research)
2. Show me the code
3. Show full build output
4. I'll fix it manually (say "resume automation" when ready)
5. Other

Choose (1-5): _
```

**If tests fail:**
Present menu with investigation options.

**If git staging fails:**
Continue anyway, log warning.

---

## Success Criteria

Workflow is successful when:

- Plugin compiles without errors
- All stages completed in sequence
- Tests pass (if run)
- PLUGINS.md updated to ✅ Working
- Handoff file deleted (workflow complete)
- Git history shows all stage commits
- Ready for installation or improvement
