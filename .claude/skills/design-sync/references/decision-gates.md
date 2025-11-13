# Decision Gate Examples

This reference contains detailed menu examples for each drift category. These menus are presented in Step 5 based on the drift categorization from Step 4.

## Contents
- [No Drift Confirmation](#no-drift-confirmation)
- [Acceptable Evolution Choice](#acceptable-evolution-choice)
- [Drift Requiring Attention](#drift-requiring-attention)
- [Critical Drift (Blocked)](#critical-drift-blocked)

---

## No Drift Confirmation

**When to use**: Parameter counts match (±1), all features present, style aligned

**Menu example**:

```
✓ Design-brief alignment verified

- Parameter count: 12 (matches brief)
- All features present: preset browser, bypass, meter
- Visual style aligned: Modern minimal with analog warmth

What's next?
1. Continue implementation (recommended) - Alignment confirmed
2. Review details - See full comparison
3. Other
```

**Decision gate properties**:
- Blocking: true
- Wait for user: REQUIRED
- Do NOT auto-select option 1

---

## Acceptable Evolution Choice

**When to use**: Minor parameter increase (2-4), core concept intact, justified additions

**Menu example**:

```
⚠️ Design evolution detected (acceptable)

**Changes from brief:**
- Parameter count: 12 (brief: 8) +4 parameters
  - Added: TONE, DRIVE, MIX, WIDTH
- Added features: Visual feedback meter (not in brief)
- Visual refinements: Animation polish, gradient backgrounds

**Assessment:** Reasonable evolution based on design iteration

**Reasoning:**
Added parameters (TONE, DRIVE, MIX, WIDTH) provide necessary tonal shaping not
anticipated in original brief. Visual meter improves usability during performance.
Core concept ("vintage tape delay") preserved.

What's next?
1. Update brief and continue (recommended) - Document evolution
2. Review changes - See detailed comparison
3. Revert to original - Simplify mockup to match brief
4. Other
```

**Decision gate properties**:
- Blocking: true
- Wait for user: REQUIRED
- Provides override option ("Update brief and continue")

---

## Drift Requiring Attention

**When to use**: Missing features, style mismatch, significant scope change (±5 parameters)

**Menu example**:

```
⚠️ Design drift detected

**Issues found:**
1. Missing feature: "Preset browser" mentioned in brief, absent in mockup
   - Brief line 42: "Include preset system for patch recall"
   - Mockup: No preset-related components
2. Visual style mismatch:
   - Brief: "Vintage warmth with analog aesthetics"
   - Mockup: Stark modern minimal with flat colors
3. Parameter count: 5 (brief: 12) - significant reduction
   - Removed: FEEDBACK_TONE, MOD_DEPTH, SPREAD, MIX, WIDTH, DRY, WET

**Recommendation:** Address drift before implementation

**Confidence:** HIGH (clear quantitative + semantic misalignment)

What's next?
1. Update mockup - Add missing features, adjust style to match brief
2. Update brief - Brief overpromised, mockup is realistic scope
3. Continue anyway (override) - Accept drift, proceed with mockup as-is
4. Review detailed comparison - See side-by-side analysis
5. Other
```

**Decision gate properties**:
- Blocking: true
- Wait for user: REQUIRED
- Provides override option ("Continue anyway")

---

## Critical Drift (Blocked)

**When to use**: Contradicts core concept, massive scope change (2x parameters), fundamental misalignment

**Menu example**:

```
❌ Critical design drift - Implementation BLOCKED

**Critical issues:**
1. Brief core concept: "Tempo-synced rhythmic delay with modulation"
   Mockup delivers: Basic feedback delay (no tempo sync, no modulation)
2. Parameter count: 25 (brief: 5) - 5x scope creep
   - Brief: DELAY_TIME, FEEDBACK, MIX, TONE, DRIVE
   - Mockup: 20 additional parameters for features not mentioned in brief

**Action required:** Resolve drift before implementation can proceed

**Why blocking:**
Mockup contradicts core concept. Implementation would not match user's vision.
High risk of complete rework.

What's next?
1. Update mockup - Align with brief's core concept (tempo sync + modulation)
2. Update brief - Revise concept to match mockup's approach (basic delay)
3. Start over - Create new mockup from brief
4. Other

(Option to override not provided - critical drift must be resolved)
```

**Decision gate properties**:
- Blocking: true
- Wait for user: REQUIRED
- NO override option (critical drift must be resolved)

---

## Usage Notes

**In Step 5 (SKILL.md):**
- Categorize drift in Step 4
- Select appropriate menu template from this reference
- Populate with actual findings from Steps 2-3
- Present menu and WAIT for user selection
- Never auto-proceed, even for "No Drift" category
