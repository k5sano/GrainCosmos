# Design Drift Detection - Detailed Examples

This reference contains detailed scenario examples for each drift category. The categorization thresholds are defined in SKILL.md Step 4.

## Example Scenarios

### No Drift ✅

**Example scenario:**
```
✓ Design-brief alignment verified

- Parameter count: 12 (matches brief)
- All features present
- Visual style aligned: Modern minimal with analog warmth

What's next?
1. Continue implementation (recommended) - Alignment confirmed
2. Review details - See full comparison
3. Other
```

### Acceptable Evolution ✅

**Indicators:**
- Parameter count increased slightly (brief: 8, mockup: 10)
- Visual polish added (animations, micro-interactions not in brief)
- Layout refined for better UX
- Mockup is superset of brief (adds but doesn't remove)

**Why acceptable:**
- User iterated on design (normal)
- Additions improve UX
- Core concept intact
- No contradictions

**Action:** Update brief's "Evolution" section, document decisions

**Output:**
```
⚠️ Design evolution detected (acceptable)

**Changes from brief:**
- Parameter count: 12 (brief: 8) +4 parameters
- Added features: Visual feedback meter
- Visual refinements: Animation polish

**Assessment:** Reasonable evolution based on design iteration

What's next?
1. Update brief and continue (recommended) - Document evolution
2. Review changes - See detailed comparison
3. Revert to original - Simplify mockup to match brief
4. Other
```

### Drift Requiring Attention ⚠️

**Indicators:**
- Missing features from brief (brief: "preset browser", mockup: no presets)
- Visual style mismatch (brief: vintage, mockup: modern)
- Significant scope reduction (brief promises more than mockup delivers)
- Parameter count significantly different (brief: 5, mockup: 15)

**Why problematic:**
- User expectations not met
- Brief and mockup tell different stories
- Implementation will surprise user
- May indicate misunderstanding

**Action:** Present options: Update mockup / Update brief / Override

**Output:**
```
⚠️ Design drift detected

**Issues found:**
1. Missing feature: "Preset browser" mentioned in brief, absent in mockup
2. Visual style mismatch: Brief emphasizes "vintage warmth", mockup is "stark modern"
3. Parameter count: 5 (brief: 12) - significant reduction

**Recommendation:** Address drift before implementation

What's next?
1. Update mockup - Add missing features, adjust style
2. Update brief - Brief overpromised, mockup is realistic
3. Continue anyway (override) - Accept drift, proceed with mockup as-is
4. Review detailed comparison
5. Other
```

### Critical Drift ❌

**Indicators:**
- Mockup contradicts brief's core concept
- Missing essential features (brief: "tempo-synced delay", mockup: basic delay)
- Massive scope change (brief: 5 parameters, mockup: 25)
- Visual style completely opposite (brief: minimal, mockup: skeuomorphic)

**Why blocking:**
- Implementation would not match user's vision
- High risk of rework
- Core concept broken
- User will be surprised/disappointed

**Action:** BLOCK implementation, require resolution

**Output:**
```
❌ Critical design drift - Implementation BLOCKED

**Critical issues:**
1. Brief concept: "Tempo-synced rhythmic delay"
   Mockup delivers: Basic feedback delay (no tempo sync)
2. Parameter count: 25 (brief: 5) - 5x scope creep

**Action required:** Resolve drift before implementation can proceed

What's next?
1. Update mockup - Align with brief's core concept
2. Update brief - Revise concept to match mockup
3. Start over - New mockup from brief
4. Other

(Option to override not provided - critical drift must be resolved)
```

## Evolution Documentation

When acceptable evolution detected, update brief:

```markdown
## Design Evolution

### [Date] - UI Mockup Finalization

**Added:**
- 4 additional parameters (modulation depth, feedback tone, etc.)
- Visual meter for feedback level

**Rationale:**
- User testing revealed need for more tonal control
- Meter improves usability during performance

**Approved:** [User confirmation]
```

## Quantitative Thresholds

**Parameter count:**
- Match (0-1 difference): No drift
- Small increase (2-4 added): Acceptable evolution
- Large increase (5+ added): Drift requiring attention
- Massive increase (2x or more): Critical drift
- Any decrease: Drift requiring attention (scope reduction)

**Feature completeness:**
- All mentioned features present: No drift
- 1 minor feature missing: Acceptable (if justified)
- 2+ features missing: Drift requiring attention
- Core feature missing: Critical drift

**Visual style:**
- Aligned with brief aesthetic: No drift
- Refined/polished (same direction): Acceptable evolution
- Different direction: Drift requiring attention
- Opposite direction: Critical drift

## Implementation Notes

**Semantic validation requires:**
- Extended thinking (8k budget)
- Analysis of:
  - Brief: Concept, Use Cases, UI Vision
  - Mockup: YAML structure, component types, visual description
  - Comparison: Does mockup deliver on brief promise?

**Confidence levels:**
- HIGH: Clear quantitative and semantic alignment/misalignment
- MEDIUM: Some ambiguity in semantic interpretation
- LOW: Unclear if differences are intentional or drift

**When uncertain:**
- Ask user: "Is this change intentional?"
- Show side-by-side comparison
- Let user categorize (evolution vs drift)
