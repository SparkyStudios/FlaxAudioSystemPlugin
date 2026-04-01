# Rename Components to Actors/Scripts Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rename all audio component files and classes to use Flax-appropriate suffixes (Actor/Script), split the `Components/` directory into `Actors/` and `Scripts/`, and update all references.

**Architecture:** Pure mechanical rename + move refactor. Use `git mv` for file moves (preserves history), then global find-and-replace for class names and include paths. Process Actors first, then Scripts, then update external references. Each task is one class rename + move.

**Tech Stack:** C++ (Flax Engine), C# (Flax Editor scripts), git

---

## Rename Map

| Old Class | New Class | Old Path | New Path |
|-----------|-----------|----------|----------|
| `AudioProxyComponent` | `AudioProxyActor` | `Components/AudioProxyComponent.h/.cpp` | `Actors/AudioProxyActor.h/.cpp` |
| `AudioListenerComponent` | `AudioListenerActor` | `Components/AudioListenerComponent.h/.cpp` | `Actors/AudioListenerActor.h/.cpp` |
| `AudioSystemEnvironmentActor` | *(no rename)* | `Components/AudioSystemEnvironmentActor.h/.cpp` | `Actors/AudioSystemEnvironmentActor.h/.cpp` |
| `AudioSphereEnvironmentComponent` | `AudioSphereEnvironmentActor` | `Components/AudioSphereEnvironmentComponent.h/.cpp` | `Actors/AudioSphereEnvironmentActor.h/.cpp` |
| `AudioBoxEnvironmentComponent` | `AudioBoxEnvironmentActor` | `Components/AudioBoxEnvironmentComponent.h/.cpp` | `Actors/AudioBoxEnvironmentActor.h/.cpp` |
| `AudioSystemComponent` | `AudioSystemScript` | `Components/AudioSystemComponent.h/.cpp` | `Scripts/AudioSystemScript.h/.cpp` |
| `AudioSystemProxyDependentComponent` | `AudioProxyDependentScript` | *(in AudioSystemComponent.h/.cpp)* | *(in AudioSystemScript.h/.cpp)* |
| `AudioTriggerComponent` | `AudioTriggerScript` | `Components/AudioTriggerComponent.h/.cpp` | `Scripts/AudioTriggerScript.h/.cpp` |
| `AudioRtpcComponent` | `AudioRtpcScript` | `Components/AudioRtpcComponent.h/.cpp` | `Scripts/AudioRtpcScript.h/.cpp` |
| `AudioSwitchStateComponent` | `AudioSwitchStateScript` | `Components/AudioSwitchStateComponent.h/.cpp` | `Scripts/AudioSwitchStateScript.h/.cpp` |
| `AudioAnimationComponent` | `AudioAnimationScript` | `Components/AudioAnimationComponent.h/.cpp` | `Scripts/AudioAnimationScript.h/.cpp` |

## External Files Requiring Include Path / Type Name Updates

| File | What Changes |
|------|-------------|
| `Core/AudioWorldModule.h` | Forward decls: `AudioProxyComponent` → `AudioProxyActor`, `AudioListenerComponent` → `AudioListenerActor` |
| `Core/AudioWorldModule.cpp` | `#include "../Components/AudioProxyComponent.h"` → `#include "../Actors/AudioProxyActor.h"` |
| `CSharp/AudioManager.cs` | Type refs: `AudioProxyComponent`, `AudioTriggerComponent`, `AudioRtpcComponent`, `AudioSwitchStateComponent` |
| `AudioSystemEditor/CSharp/AudioSystemIcons.cs` | All `typeof()` refs for renamed types |
| `AudioSystemEditor/CSharp/Editors/AudioListenerEditor.cs` | `AudioListenerComponent` → `AudioListenerActor` |
| `AudioSystemEditor/CSharp/Editors/AudioTriggerEditor.cs` | `AudioTriggerComponent` → `AudioTriggerScript` |
| `AudioSystemEditor/CSharp/Editors/AudioRtpcEditor.cs` | `AudioRtpcComponent` → `AudioRtpcScript` |

---

### Task 1: Create directories and move Actor files

**Files:**
- Create: `Source/AudioSystem/Actors/` (directory)
- Create: `Source/AudioSystem/Scripts/` (directory)
- Move: all Actor .h/.cpp from `Components/` to `Actors/`

All commands use `git mv` to preserve history.

- [ ] **Step 1: Create directories and move Actor files**

```bash
cd /Users/na2axl/Projects/SparkyStudios/FlaxAudioSystemPlugin
mkdir -p Source/AudioSystem/Actors Source/AudioSystem/Scripts

# Move Actors
git mv Source/AudioSystem/Components/AudioProxyComponent.h Source/AudioSystem/Actors/AudioProxyActor.h
git mv Source/AudioSystem/Components/AudioProxyComponent.cpp Source/AudioSystem/Actors/AudioProxyActor.cpp
git mv Source/AudioSystem/Components/AudioListenerComponent.h Source/AudioSystem/Actors/AudioListenerActor.h
git mv Source/AudioSystem/Components/AudioListenerComponent.cpp Source/AudioSystem/Actors/AudioListenerActor.cpp
git mv Source/AudioSystem/Components/AudioSystemEnvironmentActor.h Source/AudioSystem/Actors/AudioSystemEnvironmentActor.h
git mv Source/AudioSystem/Components/AudioSystemEnvironmentActor.cpp Source/AudioSystem/Actors/AudioSystemEnvironmentActor.cpp
git mv Source/AudioSystem/Components/AudioSphereEnvironmentComponent.h Source/AudioSystem/Actors/AudioSphereEnvironmentActor.h
git mv Source/AudioSystem/Components/AudioSphereEnvironmentComponent.cpp Source/AudioSystem/Actors/AudioSphereEnvironmentActor.cpp
git mv Source/AudioSystem/Components/AudioBoxEnvironmentComponent.h Source/AudioSystem/Actors/AudioBoxEnvironmentActor.h
git mv Source/AudioSystem/Components/AudioBoxEnvironmentComponent.cpp Source/AudioSystem/Actors/AudioBoxEnvironmentActor.cpp
```

- [ ] **Step 2: Move Script files**

```bash
git mv Source/AudioSystem/Components/AudioSystemComponent.h Source/AudioSystem/Scripts/AudioSystemScript.h
git mv Source/AudioSystem/Components/AudioSystemComponent.cpp Source/AudioSystem/Scripts/AudioSystemScript.cpp
git mv Source/AudioSystem/Components/AudioTriggerComponent.h Source/AudioSystem/Scripts/AudioTriggerScript.h
git mv Source/AudioSystem/Components/AudioTriggerComponent.cpp Source/AudioSystem/Scripts/AudioTriggerScript.cpp
git mv Source/AudioSystem/Components/AudioRtpcComponent.h Source/AudioSystem/Scripts/AudioRtpcScript.h
git mv Source/AudioSystem/Components/AudioRtpcComponent.cpp Source/AudioSystem/Scripts/AudioRtpcScript.cpp
git mv Source/AudioSystem/Components/AudioSwitchStateComponent.h Source/AudioSystem/Scripts/AudioSwitchStateScript.h
git mv Source/AudioSystem/Components/AudioSwitchStateComponent.cpp Source/AudioSystem/Scripts/AudioSwitchStateScript.cpp
git mv Source/AudioSystem/Components/AudioAnimationComponent.h Source/AudioSystem/Scripts/AudioAnimationScript.h
git mv Source/AudioSystem/Components/AudioAnimationComponent.cpp Source/AudioSystem/Scripts/AudioAnimationScript.cpp
```

- [ ] **Step 3: Remove the now-empty Components directory**

```bash
rmdir Source/AudioSystem/Components
```

- [ ] **Step 4: Commit the file moves**

```bash
git add -A
git commit -m "refactor: move Components/ into Actors/ and Scripts/ with renames

Rename files to match Flax conventions: Actor suffix for Actor
subclasses, Script suffix for Script subclasses. The Components/
directory is removed."
```

---

### Task 2: Rename Actor classes and update their internal references

**Files:**
- Modify: `Source/AudioSystem/Actors/AudioProxyActor.h`
- Modify: `Source/AudioSystem/Actors/AudioProxyActor.cpp`
- Modify: `Source/AudioSystem/Actors/AudioListenerActor.h`
- Modify: `Source/AudioSystem/Actors/AudioListenerActor.cpp`
- Modify: `Source/AudioSystem/Actors/AudioSphereEnvironmentActor.h`
- Modify: `Source/AudioSystem/Actors/AudioSphereEnvironmentActor.cpp`
- Modify: `Source/AudioSystem/Actors/AudioBoxEnvironmentActor.h`
- Modify: `Source/AudioSystem/Actors/AudioBoxEnvironmentActor.cpp`
- Modify: `Source/AudioSystem/Actors/AudioSystemEnvironmentActor.h`
- Modify: `Source/AudioSystem/Actors/AudioSystemEnvironmentActor.cpp`

For each Actor file, perform these global replacements within the file:

- [ ] **Step 1: Rename AudioProxyComponent → AudioProxyActor**

In `Actors/AudioProxyActor.h`: replace all `AudioProxyComponent` with `AudioProxyActor` (class name, DECLARE_SCENE_OBJECT, comments, etc.)
In `Actors/AudioProxyActor.cpp`: replace all `AudioProxyComponent` with `AudioProxyActor` (constructor, method definitions, static member, include).

The `#include "AudioProxyComponent.h"` self-include in .cpp becomes `#include "AudioProxyActor.h"`.

- [ ] **Step 2: Rename AudioListenerComponent → AudioListenerActor**

In `Actors/AudioListenerActor.h`: replace all `AudioListenerComponent` with `AudioListenerActor`.
In `Actors/AudioListenerActor.cpp`: replace all `AudioListenerComponent` with `AudioListenerActor`. Update self-include.

- [ ] **Step 3: Rename AudioSphereEnvironmentComponent → AudioSphereEnvironmentActor**

In `Actors/AudioSphereEnvironmentActor.h`: replace all `AudioSphereEnvironmentComponent` with `AudioSphereEnvironmentActor`. Update include from `"AudioSystemEnvironmentActor.h"` (already correct path since both are in Actors/).
In `Actors/AudioSphereEnvironmentActor.cpp`: replace all `AudioSphereEnvironmentComponent` with `AudioSphereEnvironmentActor`. Update self-include and `#include "AudioProxyComponent.h"` → `#include "AudioProxyActor.h"`.

- [ ] **Step 4: Rename AudioBoxEnvironmentComponent → AudioBoxEnvironmentActor**

In `Actors/AudioBoxEnvironmentActor.h`: replace all `AudioBoxEnvironmentComponent` with `AudioBoxEnvironmentActor`.
In `Actors/AudioBoxEnvironmentActor.cpp`: replace all `AudioBoxEnvironmentComponent` with `AudioBoxEnvironmentActor`. Update self-include and `#include "AudioProxyComponent.h"` → `#include "AudioProxyActor.h"`.

- [ ] **Step 5: Update AudioSystemEnvironmentActor includes**

In `Actors/AudioSystemEnvironmentActor.h`: update forward declaration `class AudioProxyComponent;` → `class AudioProxyActor;` and parameter types `const AudioProxyComponent*` → `const AudioProxyActor*`.
In `Actors/AudioSystemEnvironmentActor.cpp`: update include from `"../Components/..."` paths. The file already uses `AudioSystemEnvironmentActor` as its class name (no rename needed), but the includes for sibling files need updating. Check and fix any `#include` paths.

- [ ] **Step 6: Commit**

```bash
git add Source/AudioSystem/Actors/
git commit -m "refactor: rename Actor classes from *Component to *Actor

AudioProxyComponent → AudioProxyActor
AudioListenerComponent → AudioListenerActor
AudioSphereEnvironmentComponent → AudioSphereEnvironmentActor
AudioBoxEnvironmentComponent → AudioBoxEnvironmentActor
Update all internal includes and type references."
```

---

### Task 3: Rename Script classes and update their internal references

**Files:**
- Modify: `Source/AudioSystem/Scripts/AudioSystemScript.h`
- Modify: `Source/AudioSystem/Scripts/AudioSystemScript.cpp`
- Modify: `Source/AudioSystem/Scripts/AudioTriggerScript.h`
- Modify: `Source/AudioSystem/Scripts/AudioTriggerScript.cpp`
- Modify: `Source/AudioSystem/Scripts/AudioRtpcScript.h`
- Modify: `Source/AudioSystem/Scripts/AudioRtpcScript.cpp`
- Modify: `Source/AudioSystem/Scripts/AudioSwitchStateScript.h`
- Modify: `Source/AudioSystem/Scripts/AudioSwitchStateScript.cpp`
- Modify: `Source/AudioSystem/Scripts/AudioAnimationScript.h`
- Modify: `Source/AudioSystem/Scripts/AudioAnimationScript.cpp`

- [ ] **Step 1: Rename AudioSystemComponent → AudioSystemScript and AudioSystemProxyDependentComponent → AudioProxyDependentScript**

In `Scripts/AudioSystemScript.h`:
- Replace all `AudioSystemComponent` with `AudioSystemScript`
- Replace all `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Update forward declaration `class AudioProxyComponent;` → `class AudioProxyActor;`
- Update any `AudioProxyComponent*` types → `AudioProxyActor*`

In `Scripts/AudioSystemScript.cpp`:
- Replace all `AudioSystemComponent` with `AudioSystemScript`
- Replace all `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Update `#include "AudioProxyComponent.h"` → `#include "../Actors/AudioProxyActor.h"`
- Update self-include

- [ ] **Step 2: Rename AudioTriggerComponent → AudioTriggerScript**

In `Scripts/AudioTriggerScript.h`:
- Replace all `AudioTriggerComponent` with `AudioTriggerScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript` in inheritance
- Update `#include "AudioSystemComponent.h"` → `#include "AudioSystemScript.h"`

In `Scripts/AudioTriggerScript.cpp`:
- Replace all `AudioTriggerComponent` with `AudioTriggerScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript` in constructor
- Update `#include "AudioListenerComponent.h"` → `#include "../Actors/AudioListenerActor.h"`
- Update `#include "AudioProxyComponent.h"` → `#include "../Actors/AudioProxyActor.h"`
- Replace any `AudioListenerComponent` type refs → `AudioListenerActor`
- Update self-include

- [ ] **Step 3: Rename AudioRtpcComponent → AudioRtpcScript**

In `Scripts/AudioRtpcScript.h`:
- Replace all `AudioRtpcComponent` with `AudioRtpcScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Update `#include "AudioSystemComponent.h"` → `#include "AudioSystemScript.h"`

In `Scripts/AudioRtpcScript.cpp`:
- Replace all `AudioRtpcComponent` with `AudioRtpcScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Update `#include "AudioProxyComponent.h"` → `#include "../Actors/AudioProxyActor.h"`
- Update self-include

- [ ] **Step 4: Rename AudioSwitchStateComponent → AudioSwitchStateScript**

In `Scripts/AudioSwitchStateScript.h`:
- Replace all `AudioSwitchStateComponent` with `AudioSwitchStateScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Update `#include "AudioSystemComponent.h"` → `#include "AudioSystemScript.h"`

In `Scripts/AudioSwitchStateScript.cpp`:
- Replace all `AudioSwitchStateComponent` with `AudioSwitchStateScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Update self-include

- [ ] **Step 5: Rename AudioAnimationComponent → AudioAnimationScript**

In `Scripts/AudioAnimationScript.h`:
- Replace all `AudioAnimationComponent` with `AudioAnimationScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Update forward declaration `class AudioTriggerComponent;` → `class AudioTriggerScript;`
- Update `#include "AudioSystemComponent.h"` → `#include "AudioSystemScript.h"`

In `Scripts/AudioAnimationScript.cpp`:
- Replace all `AudioAnimationComponent` with `AudioAnimationScript`
- Replace `AudioSystemProxyDependentComponent` with `AudioProxyDependentScript`
- Replace `AudioTriggerComponent` → `AudioTriggerScript`
- Update `#include "AudioTriggerComponent.h"` → `#include "AudioTriggerScript.h"`
- Update self-include

- [ ] **Step 6: Commit**

```bash
git add Source/AudioSystem/Scripts/
git commit -m "refactor: rename Script classes from *Component to *Script

AudioSystemComponent → AudioSystemScript
AudioSystemProxyDependentComponent → AudioProxyDependentScript
AudioTriggerComponent → AudioTriggerScript
AudioRtpcComponent → AudioRtpcScript
AudioSwitchStateComponent → AudioSwitchStateScript
AudioAnimationComponent → AudioAnimationScript
Update all internal includes and type references."
```

---

### Task 4: Update external C++ references (Core layer)

**Files:**
- Modify: `Source/AudioSystem/Core/AudioWorldModule.h`
- Modify: `Source/AudioSystem/Core/AudioWorldModule.cpp`

- [ ] **Step 1: Update AudioWorldModule.h**

- Change `#include "../Components/AudioSystemEnvironmentActor.h"` → `#include "../Actors/AudioSystemEnvironmentActor.h"`
- Change forward declaration `class AudioListenerComponent;` → `class AudioListenerActor;`
- Change forward declaration `class AudioProxyComponent;` → `class AudioProxyActor;`
- Replace all `AudioListenerComponent` type refs → `AudioListenerActor`
- Replace all `AudioProxyComponent` type refs → `AudioProxyActor`

- [ ] **Step 2: Update AudioWorldModule.cpp**

- Change `#include "../Components/AudioProxyComponent.h"` → `#include "../Actors/AudioProxyActor.h"`
- Replace all `AudioProxyComponent` type refs → `AudioProxyActor`

- [ ] **Step 3: Commit**

```bash
git add Source/AudioSystem/Core/
git commit -m "refactor: update Core layer references for Actor/Script rename"
```

---

### Task 5: Update C# references

**Files:**
- Modify: `Source/AudioSystem/CSharp/AudioManager.cs`
- Modify: `Source/AudioSystemEditor/CSharp/AudioSystemIcons.cs`
- Modify: `Source/AudioSystemEditor/CSharp/Editors/AudioListenerEditor.cs`
- Modify: `Source/AudioSystemEditor/CSharp/Editors/AudioTriggerEditor.cs`
- Modify: `Source/AudioSystemEditor/CSharp/Editors/AudioRtpcEditor.cs`

- [ ] **Step 1: Update AudioManager.cs**

- Replace all `AudioProxyComponent` → `AudioProxyActor`
- Replace all `AudioTriggerComponent` → `AudioTriggerScript`
- Replace all `AudioRtpcComponent` → `AudioRtpcScript`
- Replace all `AudioSwitchStateComponent` → `AudioSwitchStateScript`

- [ ] **Step 2: Update AudioSystemIcons.cs**

- Replace all `AudioProxyComponent` → `AudioProxyActor`
- Replace all `AudioListenerComponent` → `AudioListenerActor`
- Replace all `AudioTriggerComponent` → `AudioTriggerScript`
- Replace all `AudioRtpcComponent` → `AudioRtpcScript`
- Replace all `AudioSwitchStateComponent` → `AudioSwitchStateScript`
- Replace all `AudioAnimationComponent` → `AudioAnimationScript`
- Replace all `AudioSphereEnvironmentComponent` → `AudioSphereEnvironmentActor`
- Replace all `AudioBoxEnvironmentComponent` → `AudioBoxEnvironmentActor`

- [ ] **Step 3: Update AudioListenerEditor.cs**

- Replace all `AudioListenerComponent` → `AudioListenerActor`

- [ ] **Step 4: Update AudioTriggerEditor.cs**

- Replace all `AudioTriggerComponent` → `AudioTriggerScript`

- [ ] **Step 5: Update AudioRtpcEditor.cs**

- Replace all `AudioRtpcComponent` → `AudioRtpcScript`

- [ ] **Step 6: Commit**

```bash
git add Source/AudioSystem/CSharp/ Source/AudioSystemEditor/CSharp/
git commit -m "refactor: update C# references for Actor/Script rename"
```

---

### Task 6: Final verification — grep for stale references

- [ ] **Step 1: Search for any remaining old names**

```bash
cd /Users/na2axl/Projects/SparkyStudios/FlaxAudioSystemPlugin
grep -rn "AudioProxyComponent\|AudioListenerComponent\|AudioSphereEnvironmentComponent\|AudioBoxEnvironmentComponent\|AudioSystemComponent\|AudioTriggerComponent\|AudioRtpcComponent\|AudioSwitchStateComponent\|AudioAnimationComponent\|AudioSystemProxyDependentComponent" Source/ --include="*.h" --include="*.cpp" --include="*.cs"
```

Expected: **No matches** (or only in auto-generated .Gen files which will be regenerated).

- [ ] **Step 2: Search for stale Components/ include paths**

```bash
grep -rn "Components/" Source/ --include="*.h" --include="*.cpp"
```

Expected: **No matches**.

- [ ] **Step 3: Fix any remaining references found in Steps 1-2**

- [ ] **Step 4: Commit any fixes**

```bash
git add -A
git commit -m "refactor: fix remaining stale references from rename"
```
