# Audio System Plugin Gaps Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close all critical, high, and medium-priority gaps identified in the ezEngine-to-Flax port review so that the audio system is functional end-to-end.

**Architecture:** The plugin follows a layered architecture: Components (scene-side Actors/Scripts) submit requests through AudioSystem (singleton + thread) which dispatches via AudioTranslationLayer (ATL) to the AudioMiddleware (abstract backend). We fix bottom-up: ATL first (ID lookups), then Core (guards, listener), then Components (bugs), then Editor (preferences, build hook).

**Tech Stack:** C++ (Flax Engine API), Flax Scripting (API_CLASS/API_FIELD macros), Flax Physics (raycasting), Flax Editor API (GameCooker)

---

## File Map

| File | Responsibility | Tasks |
|------|---------------|-------|
| `Source/AudioSystem/ATL/AudioTranslationLayer.h` | ATL header — add name-to-ID reverse lookup map | 1 |
| `Source/AudioSystem/ATL/AudioTranslationLayer.cpp` | ATL impl — implement Get*Id(), update Register*/Unregister* | 1 |
| `Source/AudioSystem/ATL/AudioTranslationLayerData.h` | ATL data — add Name field to ATLControl | 1 |
| `Source/AudioSystem/Core/AudioSystem.cpp` | Core — add _initialized guards, fix SetListener | 2, 3 |
| `Source/AudioSystem/Core/AudioSystem.h` | Core — add editor listener constant | 3 |
| `Source/AudioSystem/Core/AudioWorldModule.h` | WorldModule — add proxy tracking for environment update | 4 |
| `Source/AudioSystem/Core/AudioWorldModule.cpp` | WorldModule — implement Update() environment blending | 4 |
| `Source/AudioSystem/Components/AudioProxyComponent.h` | Proxy — register/unregister with world module | 4 |
| `Source/AudioSystem/Components/AudioProxyComponent.cpp` | Proxy — register/unregister with world module | 4 |
| `Source/AudioSystem/Components/AudioTriggerComponent.cpp` | Trigger — remove debug log | 5 |
| `Source/AudioSystem/Components/AudioSphereEnvironmentComponent.h` | Sphere env — fix semantics, add validation | 6 |
| `Source/AudioSystem/Components/AudioSphereEnvironmentComponent.cpp` | Sphere env — fix GetEnvironmentAmount and debug draw | 6 |
| `Source/AudioSystem/Components/AudioBoxEnvironmentComponent.h` | Box env — rename MaxDistance to MaxExtents, add validation | 6 |
| `Source/AudioSystem/Components/AudioBoxEnvironmentComponent.cpp` | Box env — fix GetEnvironmentAmount and debug draw | 6 |
| `Source/AudioSystemEditor/Preferences/AudioSystemPreferences.cpp` | Preferences — fix boolean checks, add cleanup | 7 |
| `Source/AudioSystemEditor/Preferences/AudioSystemPreferences.h` | Preferences — add Destroy() | 7 |
| `Source/AudioSystem/Core/AudioMiddleware.h` | Middleware — add DeployFiles pure virtual | 8 |
| `Source/AudioSystemEditor/Build/AudioSystemBuildHook.h` | Build hook — simplify to middleware delegate | 8 |
| `Source/AudioSystemEditor/Build/AudioSystemBuildHook.cpp` | Build hook — forward to AudioSystem::DeployFiles | 8 |
| `Source/AudioSystemEditor/AudioSystemEditorPlugin.cpp` | Editor plugin — call preferences cleanup | 7 |

---

### Task 1: Implement ATL Get*Id() name-based lookups

**Files:**
- Modify: `Source/AudioSystem/ATL/AudioTranslationLayerData.h:28-34`
- Modify: `Source/AudioSystem/ATL/AudioTranslationLayer.h:152-199`
- Modify: `Source/AudioSystem/ATL/AudioTranslationLayer.cpp:688-911`

The 5 `Get*Id()` methods currently return `INVALID_AUDIO_SYSTEM_ID` unconditionally. The approach: add a `Name` field to `ATLControl`, populate it during registration by storing the name alongside the ID, and add reverse-lookup maps (name hash -> ID) to the ATL.

However, the current registration API (`RegisterTrigger(AudioSystemDataID id, AudioSystemTriggerData* data)`) does not receive a name. The ID _is_ the name hash in ezEngine's design. So the lookup should iterate the map and match by ID — but that's O(N). Instead, we add a parallel `Dictionary<uint32, AudioSystemDataID>` that maps `StringView` hash -> control ID. The callers (components) already know the name when they call `AudioSystem::GetTriggerId("Play_Footstep")`. The ATL needs to store name->ID mappings populated when controls are registered.

The cleanest approach: add a `Name` string parameter to each `Register*()` method, and maintain a hash->ID dictionary per control type.

- [ ] **Step 1: Add Name parameter to ATL Register methods — header**

In `AudioTranslationLayer.h`, update the 5 register method signatures and add 5 name-to-ID lookup dictionaries:

```cpp
// In the public section, update signatures:
bool RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data);
bool RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data);
bool RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data);
bool RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data);
bool RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data);
```

In the private State section, add after `ATLSoundBankMap _banks;`:

```cpp
// Name-hash to control-ID reverse lookups.
// Populated by Register*(), cleared by Unregister*() and ClearAllMaps().
Dictionary<uint32, AudioSystemDataID> _triggerNameMap;
Dictionary<uint32, AudioSystemDataID> _rtpcNameMap;
Dictionary<uint32, AudioSystemDataID> _switchStateNameMap;
Dictionary<uint32, AudioSystemDataID> _environmentNameMap;
Dictionary<uint32, AudioSystemDataID> _bankNameMap;
```

Also add `#include <Engine/Core/Types/String.h>` if not already present (it is — `StringView.h` is already included).

- [ ] **Step 2: Implement the name-hash helper**

At the top of `AudioTranslationLayer.cpp`, add a static helper after the includes:

```cpp
#include <Engine/Core/Math/Math.h>

/// Hash a StringView name to a uint32 for use as a dictionary key.
/// Uses Flax's built-in GetHash on String since StringView doesn't have one directly.
static uint32 HashName(const StringView& name)
{
    return GetHash(String(name));
}
```

- [ ] **Step 3: Update RegisterTrigger to accept name and populate the name map**

Replace the existing `RegisterTrigger` implementation:

```cpp
bool AudioTranslationLayer::RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterTrigger: invalid id or null data.");
        return false;
    }

    if (_triggers.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterTrigger: trigger {0} already registered.", id);
        return false;
    }

    ATLTrigger* trigger = New<ATLTrigger>();
    trigger->Id = id;
    trigger->pData = data;
    _triggers.Add(id, trigger);

    if (name.HasChars())
    {
        const uint32 nameHash = HashName(name);
        _triggerNameMap[nameHash] = id;
    }

    return true;
}
```

- [ ] **Step 4: Update RegisterRtpc, RegisterSwitchState, RegisterEnvironment, RegisterSoundBank**

Apply the same pattern to the other 4 register methods. Each takes `const StringView& name` and populates the corresponding `_*NameMap`:

```cpp
bool AudioTranslationLayer::RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterRtpc: invalid id or null data.");
        return false;
    }

    if (_rtpcs.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterRtpc: RTPC {0} already registered.", id);
        return false;
    }

    ATLRtpc* rtpc = New<ATLRtpc>();
    rtpc->Id = id;
    rtpc->pData = data;
    _rtpcs.Add(id, rtpc);

    if (name.HasChars())
    {
        const uint32 nameHash = HashName(name);
        _rtpcNameMap[nameHash] = id;
    }

    return true;
}

bool AudioTranslationLayer::RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterSwitchState: invalid id or null data.");
        return false;
    }

    if (_switchStates.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterSwitchState: switch state {0} already registered.", id);
        return false;
    }

    ATLSwitchState* ss = New<ATLSwitchState>();
    ss->Id = id;
    ss->pData = data;
    _switchStates.Add(id, ss);

    if (name.HasChars())
    {
        const uint32 nameHash = HashName(name);
        _switchStateNameMap[nameHash] = id;
    }

    return true;
}

bool AudioTranslationLayer::RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterEnvironment: invalid id or null data.");
        return false;
    }

    if (_environments.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterEnvironment: environment {0} already registered.", id);
        return false;
    }

    ATLEnvironment* env = New<ATLEnvironment>();
    env->Id = id;
    env->pData = data;
    _environments.Add(id, env);

    if (name.HasChars())
    {
        const uint32 nameHash = HashName(name);
        _environmentNameMap[nameHash] = id;
    }

    return true;
}

bool AudioTranslationLayer::RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterSoundBank: invalid id or null data.");
        return false;
    }

    if (_banks.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterSoundBank: bank {0} already registered.", id);
        return false;
    }

    ATLSoundBank* bank = New<ATLSoundBank>();
    bank->Id = id;
    bank->pData = data;
    _banks.Add(id, bank);

    if (name.HasChars())
    {
        const uint32 nameHash = HashName(name);
        _bankNameMap[nameHash] = id;
    }

    return true;
}
```

- [ ] **Step 5: Update Unregister methods to clean up name maps**

Add name-map removal to each Unregister method. Since we don't store the name on the ATL object, we iterate the name map to find the entry pointing to the given ID. This is fine because unregistration is infrequent.

In each `Unregister*` method, before `Delete(...)`, add the cleanup. Example for `UnregisterTrigger`:

```cpp
bool AudioTranslationLayer::UnregisterTrigger(AudioSystemDataID id)
{
    ATLTrigger* trigger = nullptr;
    if (!_triggers.TryGet(id, trigger) || trigger == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterTrigger: trigger {0} not found.", id);
        return false;
    }

    if (trigger->pData != nullptr && _middleware != nullptr)
        _middleware->DestroyTriggerData(trigger->pData);

    // Remove the reverse name-map entry that points to this ID.
    for (auto& kv : _triggerNameMap)
    {
        if (kv.Value == id)
        {
            _triggerNameMap.Remove(kv.Key);
            break;
        }
    }

    _triggers.Remove(id);
    Delete(trigger);
    return true;
}
```

Repeat for `UnregisterRtpc` (using `_rtpcNameMap`), `UnregisterSwitchState` (`_switchStateNameMap`), `UnregisterEnvironment` (`_environmentNameMap`), `UnregisterSoundBank` (`_bankNameMap`).

- [ ] **Step 6: Implement the 5 Get*Id() methods**

Replace the stubs at the bottom of `AudioTranslationLayer.cpp`:

```cpp
AudioSystemDataID AudioTranslationLayer::GetTriggerId(StringView name) const
{
    const uint32 nameHash = HashName(name);
    const AudioSystemDataID* id = _triggerNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetRtpcId(StringView name) const
{
    const uint32 nameHash = HashName(name);
    const AudioSystemDataID* id = _rtpcNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetSwitchStateId(StringView name) const
{
    const uint32 nameHash = HashName(name);
    const AudioSystemDataID* id = _switchStateNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetEnvironmentId(StringView name) const
{
    const uint32 nameHash = HashName(name);
    const AudioSystemDataID* id = _environmentNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetBankId(StringView name) const
{
    const uint32 nameHash = HashName(name);
    const AudioSystemDataID* id = _bankNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}
```

- [ ] **Step 7: Update ClearAllMaps to also clear name maps**

In `AudioTranslationLayer::ClearAllMaps()`, add after the existing `.Clear()` calls:

```cpp
_triggerNameMap.Clear();
_rtpcNameMap.Clear();
_switchStateNameMap.Clear();
_environmentNameMap.Clear();
_bankNameMap.Clear();
```

- [ ] **Step 8: Update AudioSystem Register forwarding to pass names**

In `AudioSystem.h`, update the 5 register method signatures:

```cpp
bool RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data);
bool RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data);
bool RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data);
bool RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data);
bool RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data);
```

In `AudioSystem.cpp`, update the forwarding implementations:

```cpp
bool AudioSystem::RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data)
{
    return _atl.RegisterTrigger(id, name, data);
}

bool AudioSystem::RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data)
{
    return _atl.RegisterRtpc(id, name, data);
}

bool AudioSystem::RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data)
{
    return _atl.RegisterSwitchState(id, name, data);
}

bool AudioSystem::RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data)
{
    return _atl.RegisterEnvironment(id, name, data);
}

bool AudioSystem::RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data)
{
    return _atl.RegisterSoundBank(id, name, data);
}
```

- [ ] **Step 9: Verify build compiles**

Run: `<your build command>`
Expected: Clean compile. Any middleware plugin that calls `RegisterTrigger(id, data)` will need updating to `RegisterTrigger(id, name, data)` — this is an intentional API break that middleware authors must adapt to.

- [ ] **Step 10: Commit**

```bash
git add Source/AudioSystem/ATL/AudioTranslationLayer.h Source/AudioSystem/ATL/AudioTranslationLayer.cpp Source/AudioSystem/Core/AudioSystem.h Source/AudioSystem/Core/AudioSystem.cpp
git commit -m "feat: implement ATL name-based control ID lookups

Add name->ID reverse lookup dictionaries to the ATL. Register methods
now accept a StringView name parameter. The 5 Get*Id() methods hash the
name and perform an O(1) dictionary lookup instead of returning
INVALID_AUDIO_SYSTEM_ID."
```

---

### Task 2: Add _initialized guards to SendRequest/SendRequestSync

**Files:**
- Modify: `Source/AudioSystem/Core/AudioSystem.cpp:142-180`

Requests queued before Startup() or after Shutdown() could crash the audio thread.

- [ ] **Step 1: Add guard to SendRequest**

Replace the `SendRequest` method:

```cpp
void AudioSystem::SendRequest(AudioRequest&& request)
{
    if (!_initialized)
    {
        LOG(Warning, "[AudioSystem] SendRequest: system not initialized, dropping request.");
        return;
    }

    ScopeLock lock(_requestsMutex);
    _requestsQueue.Add(MoveTemp(request));
}
```

- [ ] **Step 2: Add guard to SendRequests**

Replace the `SendRequests` method:

```cpp
void AudioSystem::SendRequests(Array<AudioRequest>& requests)
{
    if (!_initialized)
    {
        LOG(Warning, "[AudioSystem] SendRequests: system not initialized, dropping {0} request(s).", requests.Count());
        return;
    }

    ScopeLock lock(_requestsMutex);
    for (auto& req : requests)
    {
        _requestsQueue.Add(MoveTemp(req));
    }
}
```

- [ ] **Step 3: Add guard to SendRequestSync**

Replace the `SendRequestSync` method:

```cpp
void AudioSystem::SendRequestSync(AudioRequest&& request)
{
    if (!_initialized)
    {
        LOG(Warning, "[AudioSystem] SendRequestSync: system not initialized, dropping request.");
        return;
    }

    {
        ScopeLock lock(_mainMutex);
        _blockingDone = false;
    }

    {
        ScopeLock lock(_blockingRequestsMutex);
        _blockingRequestsQueue.Add(MoveTemp(request));
    }

    // Wake the audio thread to process the blocking request.
    _processingSignal.NotifyOne();

    // Block until the audio thread signals completion, guarding against spurious wakeups.
    {
        ScopeLock lock(_mainMutex);
        while (!_blockingDone)
        {
            _mainSignal.Wait(_mainMutex);
        }
    }
}
```

- [ ] **Step 4: Commit**

```bash
git add Source/AudioSystem/Core/AudioSystem.cpp
git commit -m "fix: guard SendRequest methods against uninitialized state

Drop requests with a warning if AudioSystem is not initialized,
preventing potential crashes from processing against an uninitialized ATL."
```

---

### Task 3: Fix SetListener editor override mode

**Files:**
- Modify: `Source/AudioSystem/Core/AudioSystem.h:126`
- Modify: `Source/AudioSystem/Core/AudioSystem.cpp:312-329`

When `_listenerOverrideMode` is true and index is -1, `SetListener` produces `listenerId = 0` which is `INVALID_AUDIO_SYSTEM_ID`. Port the ezEngine logic: map index -1 to a dedicated editor listener ID.

- [ ] **Step 1: Add editor listener constant to header**

In `AudioSystem.h`, add after the class opening brace (before the Singleton section):

```cpp
    /// Listener ID reserved for the editor preview listener (override mode).
    static constexpr AudioSystemDataID EDITOR_LISTENER_ID = 1;
```

- [ ] **Step 2: Fix SetListener to use override mode**

Replace the `SetListener` method in `AudioSystem.cpp`:

```cpp
void AudioSystem::SetListener(int32 index, Vector3 position, Vector3 forward, Vector3 up, Vector3 velocity)
{
    AudioSystemDataID listenerId;

    if (_listenerOverrideMode && index == -1)
    {
        listenerId = EDITOR_LISTENER_ID;
    }
    else
    {
        // Listener indices are 0-based; IDs are 1-based (0 is INVALID).
        listenerId = static_cast<AudioSystemDataID>(index + 1);
    }

    if (listenerId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSystem] SetListener: computed listener ID is invalid (index={0}).", index);
        return;
    }

    AudioRequest req;
    req.Type = AudioRequestType::UpdateListenerTransform;
    req.EntityId = listenerId;
    req.Transform.Position = position;
    req.Transform.Forward = forward;
    req.Transform.Up = up;
    req.Transform.Velocity = velocity;

    if (_listenerOverrideMode)
        SendRequestSync(MoveTemp(req));
    else
        SendRequest(MoveTemp(req));
}
```

- [ ] **Step 3: Commit**

```bash
git add Source/AudioSystem/Core/AudioSystem.h Source/AudioSystem/Core/AudioSystem.cpp
git commit -m "fix: handle editor listener override mode in SetListener

Map index -1 to EDITOR_LISTENER_ID when listener override mode is
active. Send sync request in override mode to match ezEngine behavior."
```

---

### Task 4: Implement AudioWorldModule::Update() environment blending

**Files:**
- Modify: `Source/AudioSystem/Core/AudioWorldModule.h:21-62`
- Modify: `Source/AudioSystem/Core/AudioWorldModule.cpp:9-15`
- Modify: `Source/AudioSystem/Components/AudioProxyComponent.h:53-56`
- Modify: `Source/AudioSystem/Components/AudioProxyComponent.cpp:49-66`

The AudioWorldModule::Update() stub needs to iterate all environments and all active proxies, compute environment amounts, and push them to each proxy. The proxy must register/unregister itself with the world module so the module knows which proxies are active.

- [ ] **Step 1: Add proxy tracking to AudioWorldModule header**

In `AudioWorldModule.h`, add a new section after the Default listener section and before `private:`:

```cpp
    // ========================================================================
    //  Proxy registration
    // ========================================================================

    /// Register an active audio proxy for environment updates.
    void AddProxy(AudioProxyComponent* proxy);

    /// Unregister a proxy (e.g., on EndPlay).
    void RemoveProxy(AudioProxyComponent* proxy);
```

In the private section, add:

```cpp
    Array<AudioProxyComponent*> _proxies;
```

Also add the forward declaration at the top of the file (replace the existing one or add alongside):

```cpp
class AudioProxyComponent;
```

And add `#include "../Components/AudioProxyComponent.h"` — but to avoid circular includes, use a forward declaration instead and include in the .cpp.

- [ ] **Step 2: Implement AddProxy, RemoveProxy, and Update**

In `AudioWorldModule.cpp`, add the include at the top:

```cpp
#include "../Components/AudioProxyComponent.h"
```

Add the proxy registration methods:

```cpp
void AudioWorldModule::AddProxy(AudioProxyComponent* proxy)
{
    if (proxy == nullptr)
    {
        LOG(Warning, "[AudioWorldModule] AddProxy: null proxy pointer — ignoring.");
        return;
    }

    if (_proxies.Contains(proxy))
        return;

    _proxies.Add(proxy);
}

void AudioWorldModule::RemoveProxy(AudioProxyComponent* proxy)
{
    const int32 index = _proxies.Find(proxy);
    if (index != -1)
        _proxies.RemoveAt(index);
}
```

Replace the `Update()` stub:

```cpp
void AudioWorldModule::Update()
{
    // For each active proxy, query each environment for the send amount
    // and push it to the proxy (which dispatches to the audio system if changed).
    for (AudioProxyComponent* proxy : _proxies)
    {
        if (proxy == nullptr)
            continue;

        for (const AudioSystemEnvironmentActor* env : _environments)
        {
            if (env == nullptr)
                continue;

            const AudioSystemDataID envId = env->GetEnvironmentId();
            if (envId == INVALID_AUDIO_SYSTEM_ID)
                continue;

            const float amount = env->GetEnvironmentAmount(proxy);
            proxy->SetEnvironmentAmount(envId, amount);
        }
    }
}
```

- [ ] **Step 3: Register/unregister proxy with world module**

In `AudioProxyComponent.cpp`, in `OnBeginPlay()`, add after the `Scripting::Update.Bind` line:

```cpp
    AudioSystem::Get()->GetWorldModule().AddProxy(this);
```

In `OnEndPlay()`, add before the `Scripting::Update.Unbind` line:

```cpp
    AudioSystem::Get()->GetWorldModule().RemoveProxy(this);
```

- [ ] **Step 4: Commit**

```bash
git add Source/AudioSystem/Core/AudioWorldModule.h Source/AudioSystem/Core/AudioWorldModule.cpp Source/AudioSystem/Components/AudioProxyComponent.cpp
git commit -m "feat: implement environment blending in AudioWorldModule::Update

WorldModule now tracks active proxies and iterates all environment/proxy
pairs each frame, pushing computed send amounts to each proxy."
```

---

### Task 5: Remove debug LOG(Warning) from AudioTriggerComponent::OnEnable

**Files:**
- Modify: `Source/AudioSystem/Components/AudioTriggerComponent.cpp:45-46`

- [ ] **Step 1: Remove the debug log**

Remove lines 45-46 from `AudioTriggerComponent.cpp`:

```cpp
    LOG(Warning, "[AudioTriggerComponent] OnEnable: Playing {0} with ID {1}.",
        PlayTriggerName, _playTriggerId);
```

- [ ] **Step 2: Commit**

```bash
git add Source/AudioSystem/Components/AudioTriggerComponent.cpp
git commit -m "fix: remove debug LOG(Warning) from AudioTriggerComponent::OnEnable"
```

---

### Task 6: Fix environment zone semantics for both Sphere and Box shapes

**Files:**
- Modify: `Source/AudioSystem/Components/AudioSphereEnvironmentComponent.h`
- Modify: `Source/AudioSystem/Components/AudioSphereEnvironmentComponent.cpp`
- Modify: `Source/AudioSystem/Components/AudioBoxEnvironmentComponent.h`
- Modify: `Source/AudioSystem/Components/AudioBoxEnvironmentComponent.cpp`

Both environment shapes follow the same semantic pattern:

| Zone | Sphere | Box | Send |
|------|--------|-----|------|
| Inner (full power) | `[0, Radius]` | Inside `HalfExtents` box | 1.0 |
| Falloff | `[Radius, MaxDistance]` | Between `HalfExtents` and `MaxExtents` boxes | 1.0 → 0.0 |
| Outside | `> MaxDistance` | Outside `MaxExtents` box | 0.0 |

Invariants: `MaxDistance > Radius` (sphere), `MaxExtents > HalfExtents` per-axis (box).

The current sphere code is **inverted** — it treats `Radius` as the outer boundary and `MaxDistance` as falloff width. The box uses `MaxDistance` as falloff distance from the box surface, which needs renaming to `MaxExtents` (a `Vector3`) to match the box-shaped outer boundary.

#### Part A: Fix Sphere

- [ ] **Step 1: Update sphere header comments and add validation tooltip**

In `AudioSphereEnvironmentComponent.h`, update the field comments:

```cpp
    /// Inner radius of the sphere zone (full wet-send inside this distance).
    API_FIELD(Attributes="EditorOrder(1), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Inner radius: full environment send inside this distance.\")")
    float Radius = 2.0f;

    /// Outer radius of the sphere zone (zero send beyond this distance).
    /// Must be greater than Radius. The falloff shell lies between Radius and MaxDistance.
    API_FIELD(Attributes="EditorOrder(2), Limit(0), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Outer radius: zero send beyond this distance. Must be greater than Radius.\")")
    float MaxDistance = 5.0f;
```

- [ ] **Step 2: Rewrite sphere GetEnvironmentAmount**

Replace the entire `GetEnvironmentAmount` method in `AudioSphereEnvironmentComponent.cpp`:

```cpp
float AudioSphereEnvironmentComponent::GetEnvironmentAmount(const AudioProxyComponent* proxy) const
{
    if (proxy == nullptr)
        return 0.0f;

    const Vector3 ownerPos = GetPosition();
    const Vector3 proxyPos = proxy->GetPosition();
    const float   dist     = Vector3::Distance(ownerPos, proxyPos);

    // Clamp MaxDistance so the falloff width is never negative.
    const float outerRadius = Math::Max(MaxDistance, Radius);

    // Beyond the outer sphere: no send.
    if (outerRadius <= 0.0f || dist >= outerRadius)
        return 0.0f;

    // Inside the inner sphere: full send.
    if (dist <= Radius)
        return 1.0f;

    // Falloff shell [Radius, outerRadius]: linear fade from 1 to 0.
    const float falloffWidth = outerRadius - Radius;
    if (falloffWidth <= 0.0f)
        return 0.0f;

    return 1.0f - ((dist - Radius) / falloffWidth);
}
```

- [ ] **Step 3: Fix sphere debug draw**

The debug draw should show the inner sphere at `Radius` and the outer sphere at `MaxDistance`. Replace both debug draw methods:

```cpp
void AudioSphereEnvironmentComponent::OnDebugDraw()
{
    const Color dimColor = EnvironmentColor.AlphaMultiplied(WiresDimAlpha);
    const Vector3 center = GetPosition();

    // Inner sphere (full send boundary).
    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, Radius), dimColor, 0, true);
    // Outer sphere (zero send boundary).
    const float outerRadius = Math::Max(MaxDistance, Radius);
    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, outerRadius), dimColor, 0, true);

    Actor::OnDebugDraw();
}

void AudioSphereEnvironmentComponent::OnDebugDrawSelected()
{
    const Vector3 center = GetPosition();

    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, Radius), EnvironmentColor, 0, false);
    const float outerRadius = Math::Max(MaxDistance, Radius);
    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, outerRadius), EnvironmentColor, 0, false);

    Actor::OnDebugDrawSelected();
}
```

- [ ] **Step 4: Commit sphere changes**

```bash
git add Source/AudioSystem/Components/AudioSphereEnvironmentComponent.h Source/AudioSystem/Components/AudioSphereEnvironmentComponent.cpp
git commit -m "fix: correct sphere environment zone semantics

Radius is the inner boundary (full send), MaxDistance is the outer
boundary (zero send). Falloff is linear between the two. Previously
Radius was treated as the outer boundary which inverted the behavior.
Debug draw now correctly shows both boundaries."
```

#### Part B: Fix Box — rename MaxDistance to MaxExtents

- [ ] **Step 5: Update box header with MaxExtents**

In `AudioBoxEnvironmentComponent.h`, replace the `MaxDistance` field:

```cpp
    /// Half-extents of the inner box in the Actor's local space (full wet-send inside).
    API_FIELD(Attributes="EditorOrder(1), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Inner half-extents: full environment send inside this box.\")")
    Vector3 HalfExtents = Vector3(1.0f, 1.0f, 1.0f);

    /// Half-extents of the outer box in the Actor's local space (zero send outside).
    /// Each axis must be >= the corresponding HalfExtents axis. The falloff zone
    /// lies between HalfExtents and MaxExtents.
    API_FIELD(Attributes="EditorOrder(2), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Outer half-extents: zero send outside this box. Must be >= HalfExtents per axis.\")")
    Vector3 MaxExtents = Vector3(2.0f, 2.0f, 2.0f);
```

- [ ] **Step 6: Rewrite box GetEnvironmentAmount**

Replace the entire method in `AudioBoxEnvironmentComponent.cpp`:

```cpp
float AudioBoxEnvironmentComponent::GetEnvironmentAmount(const AudioProxyComponent* proxy) const
{
    if (proxy == nullptr)
        return 0.0f;

    const Vector3 proxyWorldPos = proxy->GetPosition();

    // Transform the proxy world position into this Actor's local space so
    // the box test is always axis-aligned regardless of Actor rotation/scale.
    const Vector3 localPos = GetTransform().WorldToLocal(proxyWorldPos);
    const Vector3 absPos = Vector3(Math::Abs(localPos.X), Math::Abs(localPos.Y), Math::Abs(localPos.Z));

    // Clamp MaxExtents so each axis is at least as large as HalfExtents.
    const Vector3 outerExtents = Vector3::Max(MaxExtents, HalfExtents);

    // Outside the outer box entirely: no send.
    if (absPos.X >= outerExtents.X || absPos.Y >= outerExtents.Y || absPos.Z >= outerExtents.Z)
        return 0.0f;

    // Inside the inner box: full send.
    if (absPos.X <= HalfExtents.X && absPos.Y <= HalfExtents.Y && absPos.Z <= HalfExtents.Z)
        return 1.0f;

    // Falloff zone: compute per-axis interpolation factor and take the minimum.
    // For each axis, the factor is 1.0 at HalfExtents and 0.0 at outerExtents.
    float minFactor = 1.0f;

    for (int32 i = 0; i < 3; ++i)
    {
        const float inner = (&HalfExtents.X)[i];
        const float outer = (&outerExtents.X)[i];
        const float pos   = (&absPos.X)[i];

        if (pos > inner)
        {
            const float range = outer - inner;
            if (range <= 0.0f)
            {
                minFactor = 0.0f;
                break;
            }
            const float factor = 1.0f - ((pos - inner) / range);
            minFactor = Math::Min(minFactor, factor);
        }
    }

    return Math::Max(minFactor, 0.0f);
}
```

- [ ] **Step 7: Update box debug draw to show both inner and outer boxes**

Replace the debug draw methods in `AudioBoxEnvironmentComponent.cpp`:

```cpp
void AudioBoxEnvironmentComponent::OnDebugDraw()
{
    const Color dimColor = EnvironmentColor.AlphaMultiplied(WiresDimAlpha);
    const Vector3 outerExtents = Vector3::Max(MaxExtents, HalfExtents);

    // Inner box (full send boundary).
    OrientedBoundingBox innerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, HalfExtents * 2.0f, innerBox);
    innerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(innerBox, dimColor, 0, true);

    // Outer box (zero send boundary).
    OrientedBoundingBox outerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, outerExtents * 2.0f, outerBox);
    outerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(outerBox, dimColor, 0, true);

    Actor::OnDebugDraw();
}

void AudioBoxEnvironmentComponent::OnDebugDrawSelected()
{
    const Vector3 outerExtents = Vector3::Max(MaxExtents, HalfExtents);

    OrientedBoundingBox innerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, HalfExtents * 2.0f, innerBox);
    innerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(innerBox, EnvironmentColor, 0, false);

    OrientedBoundingBox outerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, outerExtents * 2.0f, outerBox);
    outerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(outerBox, EnvironmentColor, 0, false);

    Actor::OnDebugDrawSelected();
}
```

- [ ] **Step 8: Commit box changes**

```bash
git add Source/AudioSystem/Components/AudioBoxEnvironmentComponent.h Source/AudioSystem/Components/AudioBoxEnvironmentComponent.cpp
git commit -m "fix: rename MaxDistance to MaxExtents for box environment zones

Box environments now use HalfExtents (inner, full send) and MaxExtents
(outer, zero send) with per-axis linear falloff between the two boxes.
Debug draw shows both inner and outer wireframe boxes.
Replaces the old MaxDistance (scalar falloff from surface) with a
proper Vector3 outer boundary matching the sphere's Radius/MaxDistance
pattern."
```

---

### Task 7: Fix AudioSystemPreferences boolean checks and memory leak

**Files:**
- Modify: `Source/AudioSystemEditor/Preferences/AudioSystemPreferences.h:56-70`
- Modify: `Source/AudioSystemEditor/Preferences/AudioSystemPreferences.cpp:65-126`
- Modify: `Source/AudioSystemEditor/AudioSystemEditorPlugin.cpp` (add cleanup call)

Flax's `File::ReadAllBytes` and `File::WriteAllBytes` return `true` on **error** (this is the Flax convention). The current code's boolean checks are actually correct for this convention. However, the `_instance` is never freed — that's a real leak.

- [ ] **Step 1: Add static Destroy method to header**

In `AudioSystemPreferences.h`, add after the `Get()` declaration:

```cpp
    /// Destroy the singleton instance and release memory.
    /// Safe to call even if LoadOrCreate() was never called.
    static void Destroy();
```

- [ ] **Step 2: Implement Destroy**

In `AudioSystemPreferences.cpp`, add after the `Get()` method:

```cpp
void AudioSystemPreferences::Destroy()
{
    if (_instance != nullptr)
    {
        Delete(_instance);
        _instance = nullptr;
    }
}
```

- [ ] **Step 3: Call Destroy from editor plugin shutdown**

In `AudioSystemEditorPlugin.cpp`, in the `Deinitialize()` method, after the existing `prefs->Save()` call (or at the end of `Deinitialize()`), add:

```cpp
    AudioSystemPreferences::Destroy();
```

- [ ] **Step 4: Remove const_cast smell in Save()**

In `AudioSystemPreferences.h`, change `Save()` from `const`:

```cpp
    void Save();
```

In `AudioSystemPreferences.cpp`, update the implementation:

```cpp
void AudioSystemPreferences::Save()
{
    const String settingsPath = GetSettingsPath();

    const String settingsDir = StringUtils::GetDirectoryName(settingsPath);
    if (!FileSystem::DirectoryExists(settingsDir))
    {
        FileSystem::CreateDirectory(settingsDir);
    }

    const Array<byte> data = JsonSerializer::SaveToBytes(this);

    if (File::WriteAllBytes(settingsPath, data))
    {
        LOG(Warning, "[AudioSystemPreferences] Failed to write settings to: {0}", settingsPath);
        return;
    }

    LOG(Info, "[AudioSystemPreferences] Settings saved to: {0}", settingsPath);
}
```

- [ ] **Step 5: Commit**

```bash
git add Source/AudioSystemEditor/Preferences/AudioSystemPreferences.h Source/AudioSystemEditor/Preferences/AudioSystemPreferences.cpp Source/AudioSystemEditor/AudioSystemEditorPlugin.cpp
git commit -m "fix: add preferences cleanup and remove const_cast

Add AudioSystemPreferences::Destroy() called from editor plugin
Deinitialize to prevent memory leak. Remove const from Save() to
eliminate the const_cast workaround."
```

---

### Task 8: Delegate build deployment to middleware via new DeployFiles interface

**Files:**
- Modify: `Source/AudioSystem/Core/AudioMiddleware.h` — add `DeployFiles` pure virtual
- Modify: `Source/AudioSystem/Core/AudioSystem.h` — add `DeployFiles` forwarding method
- Modify: `Source/AudioSystem/Core/AudioSystem.cpp` — implement forwarding
- Rewrite: `Source/AudioSystemEditor/Build/AudioSystemBuildHook.h` — simplify to just delegate
- Rewrite: `Source/AudioSystemEditor/Build/AudioSystemBuildHook.cpp` — remove file-copy logic, call middleware

The audio system is middleware-agnostic. It has no knowledge of bank directories, file layouts, or asset formats. The build hook should not own file-copy logic or read `BankDirectories` from preferences. Instead, it should ask the loaded middleware to deploy whatever files it needs to the output path. Each middleware implementation (Wwise, FMOD, etc.) knows its own assets and deployment strategy.

- [ ] **Step 1: Add DeployFiles to AudioMiddleware interface (editor-only)**

In `AudioMiddleware.h`, add a new method in the Lifecycle section, after `StopAllSounds()`, wrapped in `USE_EDITOR`:

```cpp
#if USE_EDITOR
    /// Deploy middleware-specific files (sound banks, config, etc.) to the
    /// cooked build output directory. Called by the editor build hook during
    /// the GameCooker deploy phase.
    ///
    /// The middleware implementation is responsible for knowing which files
    /// to copy and where they should go within the output tree.
    ///
    /// \param outputPath  Absolute path to the cooked output root folder.
    /// \return true if deployment succeeded; false on error.
    virtual bool DeployFiles(const StringView& outputPath) = 0;
#endif
```

- [ ] **Step 2: Add DeployFiles forwarding to AudioSystem (editor-only)**

In `AudioSystem.h`, add in the Configuration section (after `LoadConfiguration`), wrapped in `USE_EDITOR`:

```cpp
#if USE_EDITOR
    /// Forward a deploy-files request to the loaded middleware.
    /// Called from the editor build hook during game cooking.
    /// \param outputPath  Absolute path to the cooked output root folder.
    /// \return true if the middleware deployed successfully, false on error or no middleware.
    bool DeployFiles(const StringView& outputPath);
#endif
```

In `AudioSystem.cpp`, add the implementation after `LoadConfiguration`, wrapped in `USE_EDITOR`:

```cpp
#if USE_EDITOR
bool AudioSystem::DeployFiles(const StringView& outputPath)
{
    AudioMiddleware* mw = _atl.GetMiddleware();
    if (mw == nullptr)
    {
        LOG(Warning, "[AudioSystem] DeployFiles: no middleware registered — nothing to deploy.");
        return false;
    }

    return mw->DeployFiles(outputPath);
}
#endif
```

- [ ] **Step 3: Simplify AudioSystemBuildHook header**

Replace the entire `AudioSystemBuildHook.h`:

```cpp
#pragma once

// Forward declaration to avoid pulling the full cooker header into every TU.
struct CookingData;

// ============================================================================
//  AudioSystemBuildHook
//
//  Subscribes to the GameCooker deployment event and delegates file
//  deployment to the loaded audio middleware. The hook itself has no
//  knowledge of middleware-specific assets or file layouts.
//
//  Lifecycle:
//    Register()   — called from AudioSystemEditorPlugin::Initialize()
//    Unregister() — called from AudioSystemEditorPlugin::Deinitialize()
// ============================================================================

class AUDIOSYSTEMEDITOR_API AudioSystemBuildHook
{
public:
    /// Subscribe to GameCooker.DeployFiles.
    static void Register();

    /// Unsubscribe from GameCooker.DeployFiles.
    static void Unregister();

private:
    AudioSystemBuildHook() = delete;
    ~AudioSystemBuildHook() = delete;

    /// Called by GameCooker during the deploy phase.
    /// Forwards to AudioSystem::DeployFiles().
    static void OnDeployFiles(CookingData& data);
};
```

- [ ] **Step 4: Rewrite AudioSystemBuildHook implementation**

Replace the entire `AudioSystemBuildHook.cpp`:

```cpp
#include "AudioSystemBuildHook.h"

#include <Editor/Cooker/GameCooker.h>
#include <Engine/Core/Log.h>

#include "../../AudioSystem/Core/AudioSystem.h"

// ============================================================================
//  Lifecycle
// ============================================================================

void AudioSystemBuildHook::Register()
{
    GameCooker::DeployFiles.Bind<&AudioSystemBuildHook::OnDeployFiles>();
    LOG(Info, "[AudioSystemBuildHook] Build deployment hook registered.");
}

void AudioSystemBuildHook::Unregister()
{
    GameCooker::DeployFiles.Unbind<&AudioSystemBuildHook::OnDeployFiles>();
    LOG(Info, "[AudioSystemBuildHook] Build deployment hook unregistered.");
}

// ============================================================================
//  DeployFiles handler
// ============================================================================

void AudioSystemBuildHook::OnDeployFiles(CookingData& data)
{
    AudioSystem* system = AudioSystem::Get();
    if (system == nullptr)
    {
        LOG(Warning, "[AudioSystemBuildHook] OnDeployFiles: AudioSystem not available.");
        return;
    }

    const String outputPath = data.DataOutputPath;
    if (!system->DeployFiles(outputPath))
    {
        LOG(Error, "[AudioSystemBuildHook] Middleware file deployment failed.");
    }
}
```

Note: The exact field name on `CookingData` for the output path may be `DataOutputPath`, `OutputPath`, or `OriginalOutputPath`. Check the Flax `Editor/Cooker/GameCooker.h` header to confirm. Adapt accordingly.

- [ ] **Step 5: Remove BankDirectories from AudioSystemPreferences**

In `AudioSystemPreferences.h`, remove the `BankDirectories` field since it is no longer used by the build hook. Bank directory configuration is now the middleware's responsibility:

Remove:
```cpp
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Bank directories to include in the cooked build.\")")
    Array<String> BankDirectories;
```

- [ ] **Step 6: Commit**

```bash
git add Source/AudioSystem/Core/AudioMiddleware.h Source/AudioSystem/Core/AudioSystem.h Source/AudioSystem/Core/AudioSystem.cpp Source/AudioSystemEditor/Build/AudioSystemBuildHook.h Source/AudioSystemEditor/Build/AudioSystemBuildHook.cpp Source/AudioSystemEditor/Preferences/AudioSystemPreferences.h
git commit -m "refactor: delegate build deployment to middleware

The build hook no longer owns file-copy logic or bank directory config.
Instead, AudioMiddleware gains a DeployFiles() pure virtual that each
backend implements with its own asset deployment strategy. The build
hook simply forwards GameCooker::DeployFiles to AudioSystem::DeployFiles
which delegates to the loaded middleware.

Remove BankDirectories from AudioSystemPreferences as it is now the
middleware's responsibility."
```

---

## Task Priority Summary

| Priority | Task | Impact |
|----------|------|--------|
| CRITICAL | 1. ATL Get*Id() lookups | Unblocks all name-based audio resolution |
| CRITICAL | 2. _initialized guards | Prevents crashes from uninitialized state |
| HIGH | 3. SetListener override mode | Fixes editor audio preview |
| HIGH | 4. WorldModule environment blending | Enables reverb/environment effects |
| MEDIUM | 5. Remove debug log | Cleans up console spam |
| MEDIUM | 6. Sphere debug draw fix | Correct editor visualization |
| MEDIUM | 7. Preferences cleanup | Fixes memory leak, code smell |
| LOW | 8. Build hook wiring | Enables bank deployment in builds |
