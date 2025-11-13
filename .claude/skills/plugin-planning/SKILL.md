---
name: plugin-planning
description: Interactive research and planning (Stages 0-1) for JUCE plugin development. Invoke via /plan command after creative-brief.md exists or as first step of /implement workflow.
allowed-tools:
  - Read # For contracts and references
  - Write # For architecture.md, plan.md
  - Edit # For state updates (PLUGINS.md, .continue-here.md)
  - Bash # For git commits, precondition checks
  - WebSearch # For professional plugin research
  - Grep # For searching existing implementations
  - Glob # For finding reference files
preconditions:
  - creative-brief.md must exist in plugins/[Name]/.ideas/
  - Plugin must NOT already be past Stage 1
---

# plugin-planning Skill

**Purpose:** Handle Stages 0-1 (Research and Planning) through interactive contract creation without subagents. This skill creates the foundation contracts (architecture.md, plan.md) that guide implementation.

**Invoked by:** `/plan` command (to be created) or as first step of `/implement` workflow

---

## Entry Point

**Check preconditions first:** See [references/preconditions.md](references/preconditions.md) for detailed validation logic.

Quick validation:
1. creative-brief.md must exist at `plugins/[Name]/.ideas/`
2. Plugin status must be ≤ Stage 1 (not already in implementation)
3. Detect existing contracts (architecture.md, plan.md) for resume logic

If all preconditions pass → proceed to Stage 0 or Stage 1 based on resume logic

---

## Stage 0: Research

**Goal:** Create DSP architecture specification (architecture.md)
**Duration:** 5-30 minutes (complexity-dependent)

**Process:** Execute 6 sequential research steps with strict enforcement:

1. Read creative brief
2. Identify technical approach (plugin type, I/O config, processing domain)
3. Research JUCE DSP modules (≥2 classes required)
4. Research professional plugins (≥3 examples required)
5. Research parameter ranges (industry standards)
6. Design sync check (if mockup exists)

**For detailed step-by-step enforcement:** See [references/stage-0-research.md](references/stage-0-research.md)

**Workflow Checklist** - Copy and track progress:

```
Stage 0 Progress:
- [ ] Step 1: Read creative brief
- [ ] Step 2: Identify technical approach
- [ ] Step 3: Research JUCE DSP modules (≥2 classes)
- [ ] Step 4: Research professional plugins (≥3 examples)
- [ ] Step 5: Research parameter ranges
- [ ] Step 6: Design sync check (if mockup exists)
- [ ] Create architecture.md from template
- [ ] Update .continue-here.md
- [ ] Update PLUGINS.md status
- [ ] Git commit changes
- [ ] Present decision menu
```

**Skill Routing:**

- **If JUCE class not found:** Invoke deep-research skill: `Skill(skill='deep-research')`
- **If mockup exists:** Invoke design-sync skill: `Skill(skill='design-sync')`
- **If dependencies missing:** Invoke system-setup skill: `Skill(skill='system-setup')`

**Output:** `plugins/[Name]/.ideas/architecture.md` (see assets/architecture-template.md)

**State management:** Update .continue-here.md, PLUGINS.md status, git commit (see [references/git-operations.md](references/git-operations.md))

**Decision menu:** Present menu from assets/decision-menu-stage-0.md, WAIT for user response

**Anti-pattern:** ❌ NEVER skip research steps 2-5 or jump directly to architecture.md creation

**VALIDATION GATE: Before proceeding to Stage 1:**

1. Verify: architecture.md exists and is not empty
2. Verify: architecture.md contains all required sections (Core Components, Processing Chain, Parameter Mapping)
3. If any verification fails:
   - Display error: "Stage 0 incomplete - architecture.md missing or invalid"
   - Return to Stage 0
4. Only proceed when all verifications pass

---

## Stage 1: Planning

**Goal:** Calculate complexity score and create implementation plan (plan.md)
**Duration:** 2-5 minutes

**Preconditions:** parameter-spec.md AND architecture.md must exist (BLOCKING)

**Process:**

1. Read contracts (parameter-spec.md, architecture.md)
2. Calculate complexity score: `min(param_count/5, 2.0) + algorithm_count + feature_count`
3. Determine strategy: Simple (≤2.0) = single-pass, Complex (≥3.0) = phased
4. Create phase breakdown if complex
5. Generate plan.md from template

**For detailed complexity calculation and phase breakdown:** See [references/stage-1-planning.md](references/stage-1-planning.md)

**Workflow Checklist** - Copy and track progress:

```
Stage 1 Progress:
- [ ] Verify contracts exist (parameter-spec.md, architecture.md)
- [ ] Read all contracts
- [ ] Calculate complexity score
- [ ] Determine implementation strategy
- [ ] Create phase breakdown (if complex)
- [ ] Generate plan.md from template
- [ ] Update .continue-here.md
- [ ] Update PLUGINS.md status
- [ ] Git commit changes
- [ ] Present decision menu
```

**Output:** `plugins/[Name]/.ideas/plan.md` (see assets/plan-template.md)

**State management:** Update .continue-here.md, PLUGINS.md status, git commit (see [references/git-operations.md](references/git-operations.md))

**Decision menu:** Present menu from assets/decision-menu-stage-1.md, WAIT for user response

---

## Handoff to Implementation

<handoff_protocol id="planning-to-implementation">
<state_requirement>
CRITICAL: Handoff file must be updated to Stage 2. Execute steps EXACTLY in sequence.
</state_requirement>

**When user chooses to proceed to Stage 2:**

<critical_sequence>
1. Update handoff file at plugin root:
```bash
# Update existing plugins/${PLUGIN_NAME}/.continue-here.md for Stage 2
# Use template from assets/implementation-handoff-template.md
cat > plugins/${PLUGIN_NAME}/.continue-here.md <<'EOF'
[template content from assets/implementation-handoff-template.md]
EOF
```

2. Verify handoff:
```bash
test -f "plugins/${PLUGIN_NAME}/.continue-here.md" || { echo "✗ Handoff not created"; exit 1; }
echo "✓ Handoff verified"
```
</critical_sequence>

<verification_step>
After handoff, verify:
- plugins/[PluginName]/.continue-here.md exists at root
- PLUGINS.md status updated to 🚧 Stage 2
</verification_step>
</handoff_protocol>

Display to user:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ Planning Complete
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Plugin: [PluginName]
Complexity: [X.X] ([Simple/Complex])
Strategy: [Single-pass | Phased implementation]

Contracts created:
✓ creative-brief.md
✓ parameter-spec.md
✓ architecture.md
✓ plan.md

Ready to build. Run: /implement [PluginName]
```

---

## Reference Files

Detailed stage implementations are in:
- `references/stage-0-research.md` - Research stage details
- `references/stage-1-planning.md` - Planning stage details

Templates are in:
- `assets/architecture-template.md` - DSP architecture contract template
- `assets/plan-template.md` - Implementation plan template
