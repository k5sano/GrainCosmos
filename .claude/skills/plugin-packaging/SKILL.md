---
name: plugin-packaging
description: Create branded PKG installers for plugin distribution
allowed-tools:
  - Bash
  - Read
  - Write
preconditions:
  - Plugin status is 📦 Installed (verified in step 1)
  - VST3 and AU binaries exist in system folders (verified in step 1)
---

# plugin-packaging Skill

**Purpose:** Create professional PKG installers for sharing plugins with others.

## Overview

Generates macOS PKG installers with branded UI, automated plugin installation, and Gatekeeper bypass instructions.

## Workflow

<critical_sequence enforcement="strict" blocking="true">

**Track your progress:**

```
Plugin Packaging Progress:
- [ ] 1. Prerequisites verified (plugin installed, binaries exist)
- [ ] 2. Metadata extracted (version, description, parameters)
- [ ] 3. Branding files created (Welcome, ReadMe, Conclusion)
- [ ] 4. Base package built (pkgbuild complete)
- [ ] 5. Branded installer created (productbuild complete)
- [ ] 6. Distribution package output (files in plugins/[PluginName]/dist/)
```

---

### 1. Verify Prerequisites

Check plugin is ready for packaging:
- Read PLUGINS.md, verify status is 📦 Installed
- Verify VST3 exists: `~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3`
- Verify AU exists: `~/Library/Audio/Plug-Ins/Components/[ProductName].component`

**Blocking:** If not installed, guide user to run `/install-plugin [PluginName]` first.

### 2. Extract Plugin Metadata

Gather information for branding files:
- Read PLUGINS.md entry for plugin:
  - Version number
  - Description
  - Parameter list (name, range, defaults)
  - Use cases
- Extract PRODUCT_NAME from `plugins/[PluginName]/CMakeLists.txt` (use grep + sed, see Section 1.2 in references/pkg-creation.md)
- Store metadata for template population

**Template variables to extract:**
- {{PLUGIN_NAME}}, {{VERSION}}, {{DESCRIPTION}}
- {{PRODUCT_NAME}}, {{PARAM_COUNT}}, {{PARAMETERS}}
- {{FEATURES}}, {{QUICK_START_PRESETS}}

### 3. Create Branding Files

Generate three branding text files by reading templates from `assets/` and replacing {{VARIABLE}} placeholders with actual plugin metadata (see Section 3 in references/pkg-creation.md for bash commands):

**Welcome.txt:**
- Plugin name with TÂCHES branding
- Brief intro (1-2 sentences)
- What's being installed (VST3, AU, parameter count)
- "Click Continue to begin"

**ReadMe.txt:**
- Full feature list
- Parameter descriptions with ranges
- Installation location details
- Gatekeeper bypass instructions (step-by-step)
- System requirements
- Support contact info

**Conclusion.txt:**
- Installation success message
- Where to find plugin in DAW
- Quick start preset suggestions (3-5 settings)
- Thank you message with TÂCHES signature

Templates are in `assets/` (welcome-template.txt, readme-template.txt, conclusion-template.txt). See Section 3 in references/pkg-creation.md for population commands.

### 4. Build Base Package

Create foundational PKG with installation logic:

**4a. Create temp directory structure:**
```bash
mkdir -p /tmp/[PluginName]-installer/payload/[PluginName]
mkdir -p /tmp/[PluginName]-installer/scripts
```

**4b. Copy binaries to payload:**
```bash
cp -R ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3 /tmp/[PluginName]-installer/payload/[PluginName]/
cp -R ~/Library/Audio/Plug-Ins/Components/[ProductName].component /tmp/[PluginName]-installer/payload/[PluginName]/
```

**Validation:** Verify binaries copied successfully:
```bash
ls /tmp/[PluginName]-installer/payload/[PluginName]/
# Should show: [ProductName].vst3 and [ProductName].component
```
ONLY proceed to 4c when both files are present.

**4c. Create postinstall script:**
- Script gets actual user (not root during install)
- Creates plugin directories if needed
- Copies plugins from /tmp to user's ~/Library
- Sets correct ownership and permissions
- Cleans up temp files

See `references/pkg-creation.md` Section 4b for complete script.

**4d. Run pkgbuild:**
```bash
pkgbuild --root payload \
         --scripts scripts \
         --identifier com.taches.[pluginname] \
         --version [X.Y.Z] \
         --install-location /tmp \
         [PluginName]-Installer.pkg
```

**Validation:** Verify base PKG created:
```bash
ls [PluginName]-Installer.pkg
# File should exist and be 3-5 MB
```
ONLY proceed to step 5 when PKG file exists.

### 5. Build Branded Installer

Wrap base package with branding:

**5a. Create Distribution.xml:**
- Title: "[PluginName] by TÂCHES"
- Organization: com.taches
- Reference branding files (Welcome/ReadMe/Conclusion)
- Reference base PKG

See `references/pkg-creation.md` Section 5a for complete XML structure.

**5b. Run productbuild:**
```bash
productbuild --distribution Distribution.xml \
             --resources resources \
             --package-path . \
             "[PluginName]-by-TACHES.pkg"
```

**Validation:** Verify branded PKG created:
```bash
ls [PluginName]-by-TACHES.pkg
# File should exist and be slightly larger than base PKG
```
ONLY proceed to step 6 when branded PKG exists.

Result: Branded PKG with custom installer screens.

### 6. Output Distribution Package

Finalize and present to user:

**6a. Create dist directory:**
```bash
mkdir -p plugins/[PluginName]/dist
```

**6b. Copy installer:**
```bash
cp /tmp/[PluginName]-installer/[PluginName]-by-TACHES.pkg plugins/[PluginName]/dist/
```

**6c. Generate install-readme.txt:**
- File list (what to send)
- Installation steps
- Gatekeeper bypass instructions
- Troubleshooting tips

Template structure: installation steps, Gatekeeper bypass instructions, plugin info, uninstallation, support contact. See Section 6c in references/pkg-creation.md for complete template.

**6d. Display summary:**
```
✓ [PluginName] packaged successfully

Created: plugins/[PluginName]/dist/[PluginName]-by-TACHES.pkg (X.X MB)

Distribution package includes:
- [PluginName]-by-TACHES.pkg (branded installer)
- install-readme.txt (installation guide)

Send both files to your friend.
```

</critical_sequence>

---

## Integration Points

**Invoked by:**
- `/package [PluginName]` command
- Natural language: "Create installer for TapeAge", "Package GainKnob"

**Invokes:**
- None (terminal skill - does not invoke other skills)

**Reads:**
- `PLUGINS.md` → Plugin metadata
- `plugins/[PluginName]/CMakeLists.txt` → PRODUCT_NAME extraction
- `~/Library/Audio/Plug-Ins/VST3/[Product].vst3` → Source binary
- `~/Library/Audio/Plug-Ins/Components/[Product].component` → Source binary

**Creates:**
- `plugins/[PluginName]/dist/[PluginName]-by-TACHES.pkg` → Branded installer
- `plugins/[PluginName]/dist/install-readme.txt` → Installation guide

**Updates:**
- None (packaging doesn't modify plugin state)

---

## Decision Menu

<decision_gate type="checkpoint" enforcement="strict">

After successful packaging, present this menu and WAIT for user response:

```
✓ [PluginName] packaged successfully

Created: plugins/[PluginName]/dist/[PluginName]-by-TACHES.pkg (X.X MB)

What's next?
1. Test installer (recommended) → Verify PKG works correctly
2. Package another plugin → /package [OtherPlugin]
3. View installation guide → Show install-readme.txt contents
4. Share distribution files → Instructions for sending to friend
5. Other

Choose (1-5): _
```

**Option handlers:**
1. **Test installer** → Provide testing checklist (see Testing Checklist in references/pkg-creation.md Section 6)
2. **Package another** → Prompt for plugin name, re-invoke skill
3. **View guide** → Display install-readme.txt with `cat` command
4. **Share instructions** → Explain what files to send, how to compress if needed
5. **Other** → Open-ended response

</decision_gate>

---

## Error Handling

Common error scenarios:

**Plugin not installed:**
- Error: "Cannot package [PluginName] - plugin not installed"
- Solution: Guide to run `/install-plugin [PluginName]` first

**Binaries not found:**
- Error: "VST3 or AU not found in system folders"
- Solution: Verify installation, check PRODUCT_NAME matches

**pkgbuild/productbuild failed:**
- Error: Display build tool error message
- Solution: Check permissions, disk space, tool installation

**Brand name missing:**
- Error: "TÂCHES branding not found in templates"
- Solution: Verify assets/ directory has template files

Common errors are documented above. See references/pkg-creation.md Section 6 for additional error scenarios.

---

## Success Criteria

Packaging succeeds when:
- ✅ Base PKG created with postinstall script
- ✅ Branded PKG created with Welcome/ReadMe/Conclusion screens
- ✅ Installer file copied to `plugins/[PluginName]/dist/`
- ✅ Installation guide generated
- ✅ File sizes reported (PKG should be 3-5 MB typically)
- ✅ User knows what files to share

**NOT required for success:**
- Actually testing installer (recommended but not blocking)
- Signing/notarization (future enhancement)
- Multi-format packages (DMG, ZIP - future modes)

---

## Notes for Claude

**When executing this skill:**

1. Extract PRODUCT_NAME carefully (may contain spaces, use quotes)
2. Populate branding templates with actual plugin metadata (not placeholders)
3. Postinstall script must handle user detection (can't assume /Users/[name])
4. Clean up temp files after success (`rm -rf /tmp/[PluginName]-installer`)
5. Report file size to user (helpful for sharing over email/Dropbox)

**Branding consistency:**
- Always use "TÂCHES" in title, welcome, conclusion
- Format: "[PluginName] by TÂCHES"
- Include Gatekeeper steps in ReadMe (critical for unsigned plugins)
