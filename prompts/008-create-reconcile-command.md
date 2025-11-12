<objective>
Create a `/reconcile` slash command that ensures all workflow state files are properly updated and committed when Claude switches between or nests workflows. This solves the problem where checkpoint protocols get forgotten during workflow transitions (e.g., ui-mockup → design-sync → back to ui-mockup).

The command should help Claude maintain consistent state tracking across all workflows in the Plugin Freedom System, preventing situations where work is completed but state files aren't updated.
</objective>

<context>
The Plugin Freedom System uses multiple workflows (plugin-workflow, ui-mockup, design-sync, plugin-improve, etc.) that each maintain their own state files. When workflows nest or transition, the checkpoint protocol can be forgotten, leaving state files stale or uncommitted.

Key workflows and their state requirements:
- plugin-workflow: Stages 0-6, updates PLUGINS.md status, .continue-here.md
- ui-mockup: Phases 1-9, generates mockup files, parameter-spec.md
- design-sync: Validates mockup vs brief, updates creative-brief.md
- plugin-improve: Version management, CHANGELOG.md updates

Read @.claude/CLAUDE.md to understand the checkpoint protocol requirements.
</context>

<requirements>
1. **Self-Assessment Component**: Ask Claude to introspect its current workflow state
2. **Rule-Based Validation**: Define reconciliation rules for each workflow
3. **Gap Analysis**: Compare current state vs expected state
4. **Decision Tree Presentation**: Show findings with clear fix options
5. **Safe Execution**: Preview changes before applying them
6. **Nested Workflow Support**: Handle workflow stacks when skills invoke other skills

The command should be invokable at any point to "catch up" checkpoint state.
</requirements>

<implementation>
Create the command file at: `./prompts/reconcile.md`

Structure the command with these phases:

## Phase 1: Self-Assessment
Include a prompt that asks Claude to thoroughly analyze and report:
- Current workflow name and stage/phase
- Last completion point
- Files created/modified in current session
- Whether in nested workflow (parent workflow if any)
- Plugin name being worked on

## Phase 2: Reconciliation Rules
Define hard-coded rules for each workflow. For each workflow, specify:
- Required state files (.continue-here.md, PLUGINS.md, etc.)
- Content requirements for each stage/phase
- Files that should be committed
- Appropriate commit message templates

Example structure:
```javascript
reconciliationRules = {
  "ui-mockup": {
    phase6: {
      stateFiles: [".continue-here.md", "parameter-spec.md", "PLUGINS.md"],
      requiredArtifacts: ["v{n}-ui.html", "v{n}-PluginEditor.h", etc.],
      commitMessage: "feat({name}): Complete UI mockup v{n} with implementation files"
    }
  }
}
```

## Phase 3: Gap Analysis Logic
Compare current filesystem state against rules:
- Check if required files exist
- Verify content is current (not stale)
- Identify unstaged/uncommitted changes
- Detect missing updates

## Phase 4: Reconciliation Report
Present findings in this format:

```
📋 Reconciliation Report for [PluginName]

Current Workflow: [workflow] ([stage/phase] - [status])

State File Analysis:
✗ .continue-here.md
  Current: [current state]
  Expected: [expected state]
  Action: UPDATE

✗ PLUGINS.md
  Current: [current entry]
  Expected: [expected entry]
  Action: UPDATE

Git Status:
✗ Unstaged: [files]
✓ Staged: [files]

Proposed Actions:
1. [Action 1]
2. [Action 2]

What should I do?
1. Fix everything automatically - Execute all updates and commit
2. Show me the diffs first - Preview file changes
3. Fix files only (no commit) - Update state files only
4. Update .continue-here.md only - Minimal checkpoint
5. Skip reconciliation - Manual handling
6. Other
```

## Phase 5: Execution
Based on user choice:
- Option 1: Update all files, stage, commit
- Option 2: Generate and show diffs, then ask again
- Option 3: Update files but don't stage/commit
- Option 4: Only update .continue-here.md
- Option 5: Exit without changes

Include appropriate file update logic for each workflow's state files.
</implementation>

<output>
Create the slash command file at: `./.claude/commands/reconcile.md`

The file should:
1. Start with the self-assessment prompt
2. Include reconciliation rules for all major workflows
3. Implement the gap analysis logic
4. Present the reconciliation report format
5. Handle user decision flow
6. Execute chosen reconciliation actions

Include example usage at the end showing how the command helps in workflow transitions.
</output>

<verification>
Before completing, verify:
- All major workflows have reconciliation rules defined
- Self-assessment prompt is clear and will elicit needed information
- Report format clearly shows what needs fixing
- Decision options cover all reasonable user preferences
- Execution logic handles each option correctly
- Command can be invoked at any workflow stage
</verification>

<success_criteria>
The command is successful when:
- It can accurately detect current workflow context through self-assessment
- It identifies all stale or missing state files
- It presents clear, actionable reconciliation options
- It safely updates state files based on user choice
- It prevents checkpoint amnesia during workflow transitions
</success_criteria>