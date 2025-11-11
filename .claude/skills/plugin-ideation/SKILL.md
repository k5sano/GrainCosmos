---
name: plugin-ideation
description: Adaptive brainstorming for plugin concepts and improvements
allowed-tools:
  - Read
  - Write
  - Bash
preconditions:
  - None (entry point)
---

# plugin-ideation Skill

**Purpose:** Interactive brainstorming and creative vision capture for new plugins and improvements to existing plugins.

## Mode Detection

**Check if plugin exists:**

```bash
grep "^### $PLUGIN_NAME$" PLUGINS.md
```

- If found → **Improvement Mode**
- If not found → **New Plugin Mode**

## New Plugin Mode

### Phase 1: Free-Form Collection

Start with open question:
```
What would you like to build?

Tell me about your plugin idea. Share as much or as little as you want—I'll ask follow-ups for anything missing.
```

**Extract from response:**
- Plugin name (if mentioned)
- Plugin type (effect/synth/utility)
- Core concept and sonic goals
- Parameter ideas and ranges
- UI vision and layout preferences
- Use cases and target users
- Inspirations and references

### Phase 2: Gap Analysis and Question Prioritization

**Question Priority Tiers:**

- **Tier 1 (Critical):** Plugin type (effect/synth/utility), core concept (what it does)
- **Tier 2 (Functional):** Parameters and ranges, processing behavior, signal flow
- **Tier 3 (Context):** Use cases, inspirations, special features (presets, MIDI, modulation)
- **Tier 4 (NEVER ASK):** UI details - if user volunteers UI info, capture it in the brief but NEVER prompt for UI in ideation phase

**Extract from Phase 1 response, then identify gaps:**

1. Parse user's free-form description
2. Check which tiers are covered
3. Identify missing critical/functional information
4. Never ask about already-provided information

**Example of smart extraction:**

```
User: "I want a tape delay with wow and flutter modulation. Should have three knobs and a vintage aesthetic."

Extracted:
- Type: Effect ✓
- Core concept: Tape delay with modulation ✓
- Parameters: wow, flutter (2 mentioned, 3 total) ✓
- UI: vintage, three knobs ✓ (capture but don't expand)

Gaps identified:
- What should the third knob control? (Tier 2)
- What ranges for wow/flutter? (Tier 2)
- Specific tape reference? (Tier 3)
- Primary use case? (Tier 3)
```

### Phase 3: Question Batch Generation

**Generate exactly 4 questions using AskUserQuestion based on identified gaps.**

**Rules:**
- If 4+ gaps exist: ask top 4 by tier priority
- If fewer gaps exist: pad with "nice to have" tier 3 questions
- Provide meaningful options (not just open text prompts)
- Always include "Other" option for custom input
- Users can skip questions via "Other" option and typing "skip"

**Example question batch (via AskUserQuestion):**

For the tape delay example above:

```
Question 1:
  question: "What should the third knob control?"
  header: "Third control"
  options:
    - label: "Mix", description: "Blend dry/wet signal"
    - label: "Feedback", description: "Delay regeneration amount"
    - label: "Tone", description: "High-frequency filtering"
    - label: "Other", description: "Custom input"

Question 2:
  question: "What ranges for wow and flutter?"
  header: "Modulation depth"
  options:
    - label: "Subtle 0-5%", description: "Natural tape variation"
    - label: "Moderate 0-15%", description: "Noticeable character"
    - label: "Extreme 0-50%", description: "Creative warping"
    - label: "Other", description: "Custom ranges"

Question 3:
  question: "Any specific tape reference?"
  header: "Inspiration"
  options:
    - label: "Echoplex 1950s-60s", description: "Classic tube preamp warmth"
    - label: "Space Echo 1970s-80s", description: "Spring reverb character"
    - label: "Modern tape sim", description: "Clean, controllable"
    - label: "Other", description: "Different reference or none"

Question 4:
  question: "Primary use case?"
  header: "Usage"
  options:
    - label: "Mixing depth", description: "Subtle layering and space"
    - label: "Creative effects", description: "Aggressive, noticeable"
    - label: "Both", description: "Versatile range"
    - label: "Other", description: "Different use case"
```

**After receiving answers:**
1. Accumulate context with previous responses
2. Re-analyze gaps
3. Proceed to decision gate

### Phase 3.5: Decision Gate

**Use AskUserQuestion with 3 options after each question batch:**

```
Question:
  question: "Ready to finalize the creative brief?"
  header: "Next step"
  options:
    - label: "Yes, finalize it", description: "Create creative-brief.md"
    - label: "Ask me 4 more questions", description: "Continue refining"
    - label: "Let me add more context first", description: "Provide additional details"

Route based on answer:
- Option 1 → Proceed to Phase 4 (document creation)
- Option 2 → Return to Phase 2 (re-analyze gaps, generate next 4 questions)
- Option 3 → Collect free-form text, merge with context, return to Phase 2
```

**Context accumulation example:**

After Batch 1 answers: "Feedback", "Moderate 0-15%", "Space Echo", "Both"

Updated context:
- Parameters: wow (0-15%), flutter (0-15%), feedback (need range) ✓
- Inspiration: Space Echo ✓
- Use case: versatile ✓

New gaps for Batch 2:
- Feedback range? (Tier 2)
- Delay time range? (Tier 2)
- Tempo sync? (Tier 3)
- Specific Space Echo model reference? (Tier 3)

### Phase 3.7: Plugin Name (if not yet provided)

**Before creating documents, check if plugin name was provided at any point during conversation.**

If name NOT yet provided, ask via AskUserQuestion:

```
Question:
  question: "What should this plugin be called?"
  header: "Plugin name"
  options:
    - label: "[SuggestedName1]", description: "Based on core concept"
    - label: "[SuggestedName2]", description: "Alternative naming"
    - label: "[SuggestedName3]", description: "Different approach"
    - label: "Other", description: "I'll provide my own name"

Where suggested names are generated from the core concept.
Examples:
- Tape delay → "TapeAge", "VintageDelay", "FlutterDelay"
- 808 clap → "ClapMachine", "FlamClap", "808Clap"
- Distortion → "SaturnDrive", "WarmClip", "HarmonicDirt"
```

**If name already provided** (in initial description or in additional context), skip this phase entirely.

**Name validation:**
- Must be UpperCamelCase (e.g., "TapeAge", not "tape age" or "tapeage")
- No spaces or special characters
- If user provides invalid name, suggest cleaned version

### Phase 4: Document Creation

When user chooses "finalize" and name is confirmed, create:

**File:** `plugins/[PluginName]/.ideas/creative-brief.md`

**Format:**
```markdown
# [PluginName] - Creative Brief

## Overview

**Type:** [Effect/Synth/Utility]
**Core Concept:** [One-sentence description]
**Status:** 💡 Ideated
**Created:** [Date]

## Vision

[Prose description of plugin concept, sonic goals, inspiration]

## Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| [Name] | [Min-Max] | [Value] | [Purpose] |
| ... | ... | ... | ... |

## UI Concept

**Layout:** [Description]
**Visual Style:** [Description]
**Key Elements:** [List special UI components]

## Use Cases

- [Scenario 1]
- [Scenario 2]
- [Scenario 3]

## Inspirations

- [Plugin/hardware reference 1]
- [Plugin/hardware reference 2]
- [Sonic reference]

## Technical Notes

[Any specific DSP approaches, algorithms, or technical considerations mentioned]

## Next Steps

- [ ] Create UI mockup (`/dream [PluginName]` → option 3)
- [ ] Start implementation (`/implement [PluginName]`)
```

**Also update PLUGINS.md:**

Add entry if doesn't exist:
```markdown
### [PluginName]

**Status:** 💡 Ideated
**Type:** [Effect/Synth/Utility]
**Created:** [Date]
**Description:** [One-sentence summary]
```

### Phase 5: Session Handoff

Create `.continue-here.md` in `plugins/[PluginName]/.ideas/`:

**Format:**
```markdown
---
plugin: [PluginName]
stage: ideation
status: creative_brief_complete
last_updated: [YYYY-MM-DD HH:MM:SS]
---

# Resume Point

## Current State: Creative Brief Complete

Creative brief has been finalized for [PluginName]. Ready to proceed to UI mockup or implementation.

## Completed So Far

**Ideation:** ✓ Complete
- Core concept defined
- Parameters specified
- UI vision captured
- Use cases identified

## Next Steps

1. Create UI mockup to visualize design (recommended)
2. Start implementation directly
3. Research similar plugins for inspiration

## Context to Preserve

**Key Decisions:**
- Plugin type: [Effect/Synth/Utility]
- Core concept: [Brief summary]

**Files Created:**
- plugins/[PluginName]/.ideas/creative-brief.md
```

### Phase 6: Decision Menu

Present next steps:

```
✓ Creative brief complete: [PluginName]

What's next?
1. Create UI mockup (recommended) ← Visualize before building
2. Start implementation
3. Research similar plugins ← Find inspiration and examples
4. Save for later
5. Other

Choose (1-5): _
```

**Handle responses:**
- Option 1 → Invoke `ui-mockup` skill
- Option 2 → Invoke `plugin-workflow` skill (after warning about contracts)
- Option 3 → Invoke `deep-research` skill (Phase 7 - stub for now, just acknowledge)
- Option 4 → Confirm handoff file created, exit
- Option 5 → Ask what they'd like to do

## Improvement Mode

### Phase 0: Vagueness Detection

Check if request is specific:

**Request IS specific if it has:**
- Feature name (e.g., "resonance parameter")
- Action (e.g., "add", "fix", "change")
- Acceptance criteria (e.g., "range 0-1", "increase to 500px")

**Request IS vague if lacking above:**
- "improve the filters"
- "better presets"
- "UI feels cramped"

**If vague, present choice:**
```
Your request is somewhat vague. How should I proceed?

1. Brainstorm approaches first (recommended) ← Explore options together
2. Implement something reasonable ← I'll investigate and propose a solution

Choose (1-2): _
```

If option 1 chosen, continue with improvement brainstorming.
If option 2 chosen, exit to plugin-improve skill.

### Phase 1: Free-Form Collection

Ask:
```
What would you like to improve in [PluginName]?

Describe what you want to change, add, or fix. I'll ask follow-ups for anything missing.
```

**Extract from response:**
- What aspect (DSP/Parameters/UI/Features)
- Current pain point or limitation
- Proposed change
- Why this improvement matters
- Backward compatibility concerns
- How to test success

### Phase 2: Gap Analysis and Question Prioritization

**Question Priority Tiers:**

- **Tier 1 (Critical):** What aspect (DSP/Parameters/UI/Features), current state vs proposed change
- **Tier 2 (Implementation):** Implementation details, testing criteria, backward compatibility
- **Tier 3 (Context):** Rationale, success metrics, version impact

**Extract from Phase 1 response, then identify gaps:**

1. Parse user's improvement description
2. Check which tiers are covered
3. Identify missing critical information
4. Never ask about already-provided information

### Phase 3: Question Batch Generation

**Generate exactly 4 questions using AskUserQuestion based on identified gaps.**

**Rules:**
- If 4+ gaps exist: ask top 4 by tier priority
- If fewer gaps exist: pad with "nice to have" tier 3 questions
- Provide meaningful options (not just open text prompts)
- Always include "Other" option for custom input

**Example question batch (via AskUserQuestion):**

```
Question 1:
  question: "Which aspect would you like to improve?"
  header: "Aspect"
  options:
    - label: "Audio processing (DSP)", description: "Change how it sounds"
    - label: "Parameters", description: "Add/modify/remove controls"
    - label: "User interface", description: "Layout or visual changes"
    - label: "Features/workflow", description: "Presets, MIDI, utilities"

Question 2:
  question: "What's the current behavior you want to change?"
  header: "Current state"
  options:
    - label: "It's broken", description: "Bug or error"
    - label: "It's limited", description: "Missing functionality"
    - label: "It's inefficient", description: "Performance issue"
    - label: "Other", description: "Different issue"

Question 3:
  question: "Version impact of this change?"
  header: "Version bump"
  options:
    - label: "Patch (bugfix)", description: "v1.0.0 → v1.0.1"
    - label: "Minor (new feature)", description: "v1.0.0 → v1.1.0"
    - label: "Major (breaking change)", description: "v1.0.0 → v2.0.0"
    - label: "Other", description: "Not sure"

Question 4:
  question: "How to verify success?"
  header: "Testing"
  options:
    - label: "A/B test audio", description: "Compare before/after sound"
    - label: "Check parameter behavior", description: "Test controls work"
    - label: "Visual inspection", description: "UI looks correct"
    - label: "Other", description: "Different testing approach"
```

**After receiving answers:**
1. Accumulate context with previous responses
2. Re-analyze gaps
3. Proceed to decision gate

### Phase 3.5: Decision Gate

**Use AskUserQuestion with 3 options after each question batch:**

```
Question:
  question: "Ready to finalize the improvement brief?"
  header: "Next step"
  options:
    - label: "Yes, finalize it", description: "Create improvement proposal"
    - label: "Ask me 4 more questions", description: "Continue refining"
    - label: "Let me add more context first", description: "Provide additional details"

Route based on answer:
- Option 1 → Proceed to Phase 4 (document creation)
- Option 2 → Return to Phase 2 (re-analyze gaps, generate next 4 questions)
- Option 3 → Collect free-form text, merge with context, return to Phase 2
```

### Phase 4: Document Creation

Create: `plugins/[PluginName]/.ideas/improvements/[feature-name].md`

**Format:**
```markdown
# [PluginName] - [Improvement Name]

**Created:** [Date]
**Type:** [Feature/Enhancement/Fix]
**Aspect:** [DSP/Parameters/UI/Features]
**Version Impact:** [PATCH/MINOR/MAJOR]

## Current State

[Description of current behavior or limitation]

## Proposed Change

[Detailed description of what should change]

## Rationale

[Why this improvement matters]

## Implementation Notes

[Technical considerations, files to modify, algorithms to use]

## Backward Compatibility

[Breaking/Non-breaking, migration strategy if needed]

## Testing Criteria

- [ ] [Test 1]
- [ ] [Test 2]
- [ ] [Test 3]

## Success Metrics

[How to know the improvement is complete and working]
```

### Phase 5: Session Handoff

Create `.continue-here.md` in `plugins/[PluginName]/.ideas/`:

```markdown
---
plugin: [PluginName]
stage: improvement_planning
status: improvement_brief_complete
improvement: [feature-name]
last_updated: [YYYY-MM-DD HH:MM:SS]
---

# Resume Point

## Current State: Improvement Brief Complete

Improvement proposal finalized for [PluginName]: [ImprovementName]

## Completed So Far

**Planning:** ✓ Complete
- Current state analyzed
- Proposed change defined
- Testing criteria established

## Next Steps

1. Start implementation (/improve [PluginName])
2. Research implementation approaches
3. Review existing code

## Context to Preserve

**Improvement:** [feature-name]
**Type:** [Feature/Enhancement/Fix]
**Version Impact:** [PATCH/MINOR/MAJOR]

**Files Created:**
- plugins/[PluginName]/.ideas/improvements/[feature-name].md
```

### Phase 6: Decision Menu

```
✓ Improvement brief complete: [ImprovementName]

What's next?
1. Start implementation (recommended)
2. Research implementation approaches ← Find examples and best practices
3. Review existing code first
4. Save for later
5. Other

Choose (1-5): _
```

**Handle responses:**
- Option 1 → Invoke `plugin-improve` skill
- Option 2 → Invoke `deep-research` skill (Phase 7 - stub)
- Option 3 → Read relevant source files, then re-present menu
- Option 4 → Confirm handoff file created, exit
- Option 5 → Ask what they'd like to do

## Vagueness Detection Rules

**Check for specificity:**

```
Specific indicators:
- Named feature ("resonance parameter", "bypass switch")
- Concrete action ("add", "remove", "change from X to Y")
- Measurable criteria ("range 0-1", "increase to 500px", "reduce by 3dB")

Vague indicators:
- Generic improvements ("better", "improve", "enhance")
- Unspecified targets ("the UI", "presets", "sound")
- No success criteria mentioned
```

If 2+ vague indicators and 0 specific indicators → Present brainstorm vs implement choice.

## Grounded Feasibility

**When user proposes ambitious ideas:**

Don't shut down creativity, but flag for research:

```
That's an interesting direction! [Specific technical consideration] might be complex—we can research approaches in Stage 0 (Research phase).

Continue exploring, or finalize brief with a research note?
```

Examples:
- Physical modeling → "Physical modeling can be CPU-intensive"
- Machine learning → "ML in real-time audio requires careful optimization"
- Complex visualizations → "3D graphics in plugins have rendering considerations"

**Gently note challenges without saying "no."**

## Examples: Question Generation Based on Input Detail

### Example 1: Detailed Input (New Plugin)

```
User: "I want a tape delay with wow and flutter modulation. Should have three knobs and a vintage aesthetic."

Extracted:
- Type: Effect ✓
- Core concept: Tape delay with modulation ✓
- Parameters: wow, flutter (2 mentioned, 3 total) ✓
- UI: vintage, three knobs ✓ (capture but don't expand)

Gaps identified (4 needed):
- What should the third knob control? (Tier 2)
- What ranges for wow/flutter? (Tier 2)
- Specific tape reference? (Tier 3)
- Primary use case? (Tier 3)

Question Batch 1 (via AskUserQuestion):
1. "What should the third knob control?" → [Mix, Feedback, Tone, Other]
2. "What ranges for wow and flutter?" → [Subtle 0-5%, Moderate 0-15%, Extreme 0-50%, Other]
3. "Any specific tape reference?" → [Echoplex 1950s-60s, Space Echo 1970s-80s, Modern, Other]
4. "Primary use case?" → [Mixing depth, Creative effects, Both, Other]

[Then decision gate with 3 options]
```

### Example 2: Vague Input (New Plugin)

```
User: "A distortion plugin"

Extracted:
- Type: Effect ✓
- Core concept: Distortion (very generic)

Gaps identified (4 needed):
- What distortion character? (Tier 1)
- What parameters? (Tier 2)
- Primary use case? (Tier 3)
- Any inspiration? (Tier 3)

Question Batch 1 (via AskUserQuestion):
1. "What distortion character?" → [Tube/tape warmth, Hard clipping, Bit crushing, Other]
2. "What parameters?" → [Drive/tone/mix (3-knob), Drive/tone/mix/bias (4-knob), Extensive multiband, Other]
3. "Primary use case?" → [Mixing saturation, Creative/aggressive, Both, Other]
4. "Any inspiration?" → [Guitar pedals, Console/tape, Modern digital, Other]

[Then decision gate]

If user chooses "Ask me 4 more questions":
- User answered: "Tube warmth", "Drive/tone/mix (3-knob)", "Both", "Console/tape"

Updated context:
- Type: Effect ✓
- Core concept: Tube/tape warmth distortion ✓
- Parameters: drive, tone, mix ✓
- Use case: versatile ✓
- Inspiration: console/tape ✓

New gaps for Batch 2:
- Drive range? (Tier 2)
- Tone frequency range? (Tier 2)
- Specific console reference? (Tier 3)
- Harmonic character? (Tier 3)

Question Batch 2:
1. "Drive control range?" → [Subtle 0-6dB, Moderate 0-12dB, Extreme 0-24dB, Other]
2. "Tone control?" → [High-pass filter, Low-pass filter, Tilt EQ, Other]
3. "Specific console reference?" → [Neve 1073, SSL 4000, API 550, Other]
4. "Harmonic character?" → [Even harmonics (warm), Odd harmonics (aggressive), Balanced, Other]

[Then decision gate again]
```

## Adaptive Questioning Strategy

**Extract first, then fill gaps:**

1. User provides initial description
2. Parse response for covered topics
3. Generate questions only for missing topics
4. Present 4 questions via AskUserQuestion
5. After each batch, re-evaluate what's still missing
6. Present decision gate
7. Repeat until user finalizes

**Don't ask redundant questions.**

If user says "I want a tape delay with wow and flutter, three knobs, vintage look," don't ask:
- ❌ "What type of plugin?" (it's a delay = effect)
- ❌ "What parameters?" (wow, flutter mentioned)
- ❌ "Visual style?" (vintage mentioned)

DO ask:
- ✓ "What should the third knob control?" (only two mentioned)
- ✓ "What range for wow/flutter parameters?"
- ✓ "Any specific tape references for the sound?"

## Continuous Iteration Support

User can request deep dives:

```
User: "Ask me more about the UI"
→ System focuses on UI-specific questions

User: "Let's explore presets"
→ System asks about preset strategy

User: "Tell me what you think about the DSP"
→ System provides feasibility analysis
```

**Support free-form exploration until user says "finalize."**

## Git Integration

After creating documents:

```bash
git add plugins/[PluginName]/.ideas/
git add PLUGINS.md
# Do NOT commit - user handles commits
```

**Stage files but don't commit.**

## Error Handling

**If plugin name contains invalid characters:**
```
Plugin names should be UpperCamelCase with no spaces or special characters.

Suggested: [CleanName]
Use this name? (y/n): _
```

**If improvement file already exists:**
```
Improvement proposal "[feature-name].md" already exists.

Options:
1. Create new version (/improve is better for implementing existing proposals)
2. Choose different name
3. Overwrite existing (not recommended)

Choose (1-3): _
```

**If creative brief already exists:**
```
Creative brief already exists for [PluginName].

Options:
1. View existing brief
2. Create improvement proposal instead (/improve)
3. Overwrite (will lose existing brief)

Choose (1-3): _
```

## Integration Points

**Invoked by:**
- `/dream` command (new plugin or improvement)
- `/dream [PluginName]` command
- Natural language: "I want to make...", "Explore improvements to..."

**Invokes:**
- `ui-mockup` skill (option after creative brief)
- `plugin-workflow` skill (option after creative brief)
- `plugin-improve` skill (option after improvement brief)
- `deep-research` skill (option for research) - Phase 7

## Success Criteria

Skill is successful when:
- Creative brief captures complete vision
- No redundant questions asked
- User feels heard and understood
- Document is actionable for implementation
- Handoff file enables resume
- Next steps are clear and discoverable
