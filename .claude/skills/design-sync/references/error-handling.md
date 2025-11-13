# Error Handling Scenarios

This reference contains detailed error scenarios with full menu examples for when design-sync encounters missing files, malformed data, or ambiguous findings.

## Contents
- [Missing Contracts](#missing-contracts)
- [No Finalized Mockup](#no-finalized-mockup)
- [Ambiguous Findings](#ambiguous-findings)
- [Override Tracking](#override-tracking)

---

## Missing Contracts

**Trigger**: creative-brief.md, parameter-spec.md, or mockup YAML not found

**Error menu**:

```
❌ Cannot validate design sync

Missing required files:
- creative-brief.md: [EXISTS/MISSING]
- parameter-spec.md: [EXISTS/MISSING]
- mockups/v[N]-ui.yaml: [EXISTS/MISSING]

design-sync requires finalized mockup (parameter-spec.md generated).

Actions:
1. Generate parameter-spec.md - Finalize mockup first
2. Create creative brief - Document vision before mockup
3. Skip validation - Proceed without sync (not recommended)
4. Other
```

**Action**: BLOCK validation, present menu, wait for user to resolve.

---

## No Finalized Mockup

**Trigger**: Mockup YAML files exist but parameter-spec.md not generated

**Error menu**:

```
❌ Cannot validate: No finalized mockup found

design-sync requires:
- mockups/v[N]-ui.yaml (finalized mockup spec)
- parameter-spec.md (generated from mockup)

Current state:
- Mockups: v1-ui.yaml, v2-ui.yaml (no parameter-spec.md)

Actions:
1. Finalize mockup - Generate parameter-spec.md via ui-mockup
2. Skip validation - Proceed without sync (not recommended)
3. Other
```

**Action**: BLOCK validation, route user to ui-mockup skill to finalize.

---

## Ambiguous Findings

**Trigger**: Semantic validation returns MEDIUM confidence or unclear alignment

**Clarification menu**:

```
⚠️ Drift assessment uncertain

Quantitative checks:
- Parameter count: 10 (brief: 8) +2 parameters ← Minor difference
- Features: All present

Semantic validation:
- Visual style: MEDIUM confidence alignment
  - Brief aesthetic: "Vintage warmth"
  - Mockup aesthetic: "Brushed metal with warm colors"
  - Ambiguous: Is brushed metal "vintage"?

**Asking for user input:**
Is this mockup style aligned with your "vintage warmth" vision?

1. Yes - Style matches intent (acceptable evolution)
2. No - Style misses the mark (drift requiring attention)
3. Review side-by-side - See comparison
4. Other
```

**Action**: Present clarification menu, wait for user input, use response to finalize categorization.

---

## Override Tracking

**When user overrides drift warnings**, log to `.validator-overrides.yaml`:

```yaml
overrides:
  - timestamp: 2025-11-10T14:32:00Z
    validator: design-sync
    plugin: DelayPlugin
    mockup-version: v3
    finding: parameter-count-increase
    severity: attention
    details: "Brief: 8 parameters, Mockup: 12 parameters (+4)"
    override-reason: "User confirmed: added parameters are intentional (TONE, DRIVE, MIX, WIDTH)"
    approved-by: User
```

**Purpose**: Audit trail for "Why did we proceed with drift?"

**Location**: `plugins/[PluginName]/.validator-overrides.yaml`

---

## Usage Notes

**In SKILL.md:**
- Check for file existence in Step 1
- If missing, select appropriate error menu from this reference
- Present menu and WAIT for user to resolve
- Block progression until files exist
- Log all overrides to `.validator-overrides.yaml`
