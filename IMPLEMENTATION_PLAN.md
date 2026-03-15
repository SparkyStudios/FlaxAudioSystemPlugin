# Flax AudioSystem Plugin — Implementation Plan

> **For AI agents:** This document is a complete, self-contained specification. Follow every phase in
> order. Read all referenced external sources before writing any code. Do not skip phases, do not
> merge phases, and do not begin a phase until all previous phases compile and pass any applicable
> tests. Mark each file as done before moving to the next.

---

## External References

Before writing any code, read these sources to understand the reference implementation and the
target platform APIs:

| Name | URL |
|---|---|
| Flax Engine source | https://github.com/FlaxEngine/FlaxEngine |
| Flax Engine — Audio module | https://github.com/FlaxEngine/FlaxEngine/tree/master/Source/Engine/Audio |
| Flax Engine — Plugin docs | https://docs.flaxengine.com/manual/scripting/plugins/index.html |
| Flax Engine — Scripting docs | https://docs.flaxengine.com/manual/scripting/index.html |
| FlaxFmod (reference plugin) | https://github.com/Tryibion/FlaxFmod |
| ezEngine runtime plugin | https://github.com/SparkyStudios/ezEngine/tree/sparky-studios/Code/EnginePlugins/AudioSystemPlugin |
| ezEngine editor plugin | https://github.com/SparkyStudios/ezEngine/tree/sparky-studios/Code/EditorPlugins/AudioSystem |

---

## Architecture Overview

```
┌──────────────────────────────────────────────────────────────────┐
│                     AudioSystemEditor module                     │
│  EditorPlugin entry · Toolbar actions · Preferences settings     │
│  Viewport icons · Asset proxies · Play-mode sync · Build hook    │
└───────────────────────────────┬──────────────────────────────────┘
                                │
┌───────────────────────────────▼──────────────────────────────────┐
│                       AudioSystem module                         │
│                                                                  │
│  ┌──────────────┐   ┌───────────────────────┐   ┌────────────┐  │
│  │  GamePlugin  │   │  AudioSystem singleton │   │AudioThread │  │
│  │  entry point │──▶│  request queues/mutex  │──▶│  ~100 Hz   │  │
│  └──────────────┘   └───────────┬───────────┘   └────────────┘  │
│                                 │                                │
│                    ┌────────────▼────────────┐                   │
│                    │ AudioTranslationLayer   │                   │
│                    │ entity/listener/control │                   │
│                    │ ID maps + ATL objects   │                   │
│                    └────────────┬────────────┘                   │
│                                 │                                │
│                    ┌────────────▼────────────┐                   │
│                    │   AudioMiddleware (ABC)  │                   │
│                    │  (implemented by a       │                   │
│                    │   separate middleware    │                   │
│                    │   plugin at runtime)     │                   │
│                    └─────────────────────────┘                   │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                      Components                          │   │
│  │  Proxy · Listener · Trigger · Rtpc · SwitchState        │   │
│  │  BoxEnvironment · SphereEnvironment · Animation          │   │
│  └──────────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────────┘
```

The plugin is **middleware-agnostic**. A concrete audio backend (Amplitude Audio, FMOD, Wwise, etc.)
is loaded at runtime by implementing `AudioMiddleware` in a separate plugin and registering it with
`AudioSystem` before `Startup()` is called.

---

## Target File Structure

```
Source/
├── AudioSystem/
│   ├── AudioSystem.Build.cs                   ← enable native C++
│   ├── AudioSystemPlugin.cpp
│   ├── AudioSystemPlugin.h
│   │
│   ├── Core/
│   │   ├── AudioSystemData.h                  ← IDs, enums, base data classes
│   │   ├── AudioSystemMessages.h              ← Flax message types
│   │   ├── AudioSystemRequests.h              ← 16 request variant types
│   │   ├── AudioMiddleware.h                  ← abstract backend interface
│   │   ├── AudioThread.cpp
│   │   ├── AudioThread.h
│   │   ├── AudioSystem.cpp
│   │   ├── AudioSystem.h
│   │   └── AudioWorldModule.cpp
│   │   └── AudioWorldModule.h
│   │
│   ├── ATL/
│   │   ├── AudioTranslationLayerData.h        ← ATL object types + ID maps
│   │   ├── AudioTranslationLayer.cpp
│   │   └── AudioTranslationLayer.h
│   │
│   └── Components/
│       ├── AudioSystemComponent.cpp
│       ├── AudioSystemComponent.h             ← abstract base + manager template
│       ├── AudioProxyComponent.cpp
│       ├── AudioProxyComponent.h
│       ├── AudioListenerComponent.cpp
│       ├── AudioListenerComponent.h
│       ├── AudioTriggerComponent.cpp
│       ├── AudioTriggerComponent.h
│       ├── AudioRtpcComponent.cpp
│       ├── AudioRtpcComponent.h
│       ├── AudioSwitchStateComponent.cpp
│       ├── AudioSwitchStateComponent.h
│       ├── AudioBoxEnvironmentComponent.cpp
│       ├── AudioBoxEnvironmentComponent.h
│       ├── AudioSphereEnvironmentComponent.cpp
│       ├── AudioSphereEnvironmentComponent.h
│       ├── AudioAnimationComponent.cpp
│       └── AudioAnimationComponent.h
│
└── AudioSystemEditor/
    ├── AudioSystemEditor.Build.cs             ← enable native C++
    ├── AudioSystemEditorPlugin.cpp
    ├── AudioSystemEditorPlugin.h
    ├── Actions/
    │   ├── AudioSystemActions.cpp
    │   └── AudioSystemActions.h
    ├── Preferences/
    │   ├── AudioSystemPreferences.cpp
    │   └── AudioSystemPreferences.h
    └── Icons/
        ├── AudioProxyComponent.png            ← 16×16 + 32×32 PNG
        ├── AudioListenerComponent.png
        ├── AudioTriggerComponent.png
        ├── AudioRtpcComponent.png
        ├── AudioSwitchStateComponent.png
        ├── AudioBoxEnvironmentComponent.png
        ├── AudioSphereEnvironmentComponent.png
        └── AudioAnimationComponent.png
```

---

## Flax ↔ ezEngine API Mapping

| ezEngine concept | Flax C++ equivalent |
|---|---|
| `ezComponent` | `Script` (C# managed) or custom C++ `ScriptingObject` |
| `ezComponentManager<T>` | No direct equivalent; use Flax `SceneObject` update tick |
| `ezWorldModule` | Custom C++ class updated from `AudioSystem::UpdateSound()` |
| `ezWorld` / `ezGameObject` | `Scene` / `Actor` |
| `ezVec3` / `ezQuat` / `ezTransform` | `Vector3` / `Quaternion` / `Transform` |
| `ezPhysicsWorldModuleInterface::RayCast` | `Physics::RayCast()` |
| `ezThread` | `Thread` (Flax threading) |
| `ezMutex` | `CriticalSection` |
| `ezSemaphore` | `Semaphore` |
| `ezSoundInterface` singleton | `IPlugin` service registered via `GamePlugin` |
| `ezDeque<ezVariant>` request queue | `Array<Variant>` or typed deque |
| `EZ_DECLARE_SINGLETON_OF_INTERFACE` | Manual singleton via `static AudioSystem* s_pInstance` |
| `ezPreferences` (editor) | `CustomEditorPresenter` + JSON settings asset |
| `ezButtonAction` / `ezSliderAction` | `ToolStripButton` / custom `ToolStripSlider` in `FlaxEditor.GUI` |
| Qt `.qrc` icon resources | Plain PNG files embedded as `FlaxEngine` content assets |

---

## Phase 1 — Build Scaffolding

**Goal:** Make both modules compile native C++. Delete the placeholder C# file.

### Steps

1. **Delete** `Source/AudioSystem/ExamplePlugin.cs`.

2. **Rewrite** `Source/AudioSystem/AudioSystem.Build.cs`:
   ```csharp
   using Flax.Build;
   using Flax.Build.NativeCpp;

   public class AudioSystem : GameModule
   {
       public override void Setup(BuildOptions options)
       {
           base.Setup(options);
           BuildNativeCode = true;
           options.PublicDependencies.Add("Core");    // Actor, Script, Thread, CriticalSection, Semaphore
           options.PublicDependencies.Add("Engine");  // Engine update hook, scripting types
           options.PublicDependencies.Add("Physics"); // Physics::RayCast for obstruction/occlusion rays
       }
   }
   ```

3. **Rewrite** `Source/AudioSystemEditor/AudioSystemEditor.Build.cs`:
   ```csharp
   using Flax.Build;
   using Flax.Build.NativeCpp;

   public class AudioSystemEditor : GameEditorModule
   {
       public override void Setup(BuildOptions options)
       {
           base.Setup(options);
           BuildNativeCode = true;
           options.PublicDependencies.Add("AudioSystem");
           options.PublicDependencies.Add("Editor");
           options.PublicDependencies.Add("EditorFramework");
       }
   }
   ```

4. Create stub `Source/AudioSystem/AudioSystemPlugin.h`:
   ```cpp
   #pragma once
   #include <Engine/Plugins/GamePlugin.h>

   /// \brief Entry point for the AudioSystem game plugin.
   API_CLASS() class AUDIOSYSTEM_API AudioSystemPlugin : public GamePlugin
   {
       API_AUTO_SERIALIZATION();
       DECLARE_SCRIPTING_TYPE(AudioSystemPlugin);
   public:
       void Initialize() override;
       void Deinitialize() override;
   };
   ```

5. Create stub `Source/AudioSystem/AudioSystemPlugin.cpp` that calls
   `AudioSystem::Startup()` in `Initialize()` and `AudioSystem::Shutdown()` in `Deinitialize()`.
   Leave method bodies empty for now — they will be filled in Phase 5.

6. Create stub `Source/AudioSystemEditor/AudioSystemEditorPlugin.h` / `.cpp` deriving from
   `EditorPlugin` with empty `InitializeEditor()` / `DeinitializeEditor()`.

7. Verify both targets compile with `flaxc` or by opening the project in Flax Editor.

---

## Phase 2 — Core Data Types (`Core/AudioSystemData.h`)

**Goal:** Define all primitive types, enums, and opaque base data classes that flow through the
entire system. Port directly from
[`AudioSystemData.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Core/AudioSystemData.h).

### Declarations required

```cpp
// ID types
using AudioSystemDataID    = uint64;
using AudioSystemControlID = uint64;
constexpr AudioSystemDataID INVALID_AUDIO_SYSTEM_ID = 0;

// Transform (position + velocity + orientation)
struct AUDIOSYSTEM_API AudioSystemTransform
{
    Vector3 Position  = Vector3::Zero;
    Vector3 Velocity  = Vector3::Zero;
    Vector3 Forward   = Vector3::Forward;
    Vector3 Up        = Vector3::Up;
    bool operator==(const AudioSystemTransform&) const;
    bool operator!=(const AudioSystemTransform&) const;
};

// Obstruction ray mode
enum class AUDIOSYSTEM_API AudioSystemSoundObstructionType : uint8
{
    None      = 0,  // no ray casting
    SingleRay = 1,  // one center ray, occlusion only
    MultipleRay = 2 // 5-ray pattern, obstruction + occlusion
};

// Trigger lifecycle state machine
enum class AUDIOSYSTEM_API AudioSystemTriggerState : uint8
{
    Invalid  = 0,
    Playing  = 1,
    Ready    = 2,
    Loading  = 3,
    Unloading = 4,
    Starting = 5,
    Stopping = 6,
    Stopped  = 7
};

// Audio event state
enum class AUDIOSYSTEM_API AudioSystemEventState : uint8
{
    Invalid  = 0,
    Playing  = 1,
    Loading  = 2,
    Unloading = 3
};

// Opaque base types — middleware fills these in by subclassing
class AUDIOSYSTEM_API AudioSystemEntityData      : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemListenerData    : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemTriggerData     : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemRtpcData        : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemSwitchStateData : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemEnvironmentData : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemEventData       : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemSourceData      : public ScriptingObject { ... };
class AUDIOSYSTEM_API AudioSystemBankData        : public ScriptingObject { ... };
```

All base data classes have only a virtual destructor. Concrete middleware plugins subclass them
and store their own handle/ID fields.

Expose `AudioSystemTransform` and the enum types to Flax reflection with `API_STRUCT()` /
`API_ENUM()` so they are accessible from C# if needed.

---

## Phase 3 — Middleware Interface (`Core/AudioMiddleware.h`)

**Goal:** Define the pure-virtual backend contract. Port from
[`AudioMiddleware.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Core/AudioMiddleware.h).

No implementation file is needed — this is a pure interface header.

### Required pure-virtual methods (grouped)

| Group | Methods |
|---|---|
| Lifecycle | `LoadConfiguration`, `Startup`, `Update(float dt)`, `Shutdown`, `Release`, `StopAllSounds` |
| Entity | `CreateWorldEntity`, `CreateEntityData`, `DestroyEntityData`, `AddEntity`, `ResetEntity`, `UpdateEntity`, `RemoveEntity`, `SetEntityTransform` |
| Listener | `CreateListenerData`, `DestroyListenerData`, `AddListener`, `ResetListener`, `RemoveListener`, `SetListenerTransform` |
| Event | `CreateEventData`, `ResetEventData`, `DestroyEventData` |
| Trigger | `LoadTrigger`, `ActivateTrigger`, `UnloadTrigger`, `StopEvent`, `StopAllEvents` |
| RTPC | `SetRtpc`, `ResetRtpc` |
| Switch | `SetSwitchState` |
| Obstruction | `SetObstructionAndOcclusion` |
| Environment | `SetEnvironmentAmount` |
| Bank | `LoadBank`, `UnloadBank`, `DestroyBank` |
| Control destroy | `DestroyTriggerData`, `DestroyRtpcData`, `DestroySwitchStateData`, `DestroyEnvironmentData` |
| Global | `SetLanguage`, `OnMasterGainChange(float)`, `OnMuteChange(bool)`, `OnLoseFocus`, `OnGainFocus` |
| Query | `GetMiddlewareName() const`, `GetMasterGain() const`, `GetMute() const` |

Configuration is loaded from a Flax JSON asset (replacing the ezEngine DDL format). Pass the asset
path as a `StringView` to `LoadConfiguration`.

---

## Phase 4 — Audio Translation Layer (`ATL/`)

**Goal:** Bridge between the high-level `AudioSystem` and the low-level `AudioMiddleware`.
Port from
[`AudioTranslationLayerData.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/ATL/AudioTranslationLayerData.h)
and
[`AudioTranslationLayer.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/ATL/AudioTranslationLayer.h).

### `ATL/AudioTranslationLayerData.h`

Define ATL wrapper objects. Each wraps an opaque middleware data pointer and an ID:

```cpp
struct ATLControl
{
    AudioSystemDataID Id = INVALID_AUDIO_SYSTEM_ID;
    virtual AudioSystemDataID GetId() const { return Id; }
    virtual ~ATLControl() = default;
};

struct ATLEntity       final : ATLControl { AudioSystemEntityData*      pData = nullptr; };
struct ATLListener           : ATLControl { AudioSystemListenerData*    pData = nullptr; };
struct ATLTrigger      final : ATLControl
{
    AudioSystemTriggerData* pData = nullptr;
    // Tracks active events spawned by this trigger
    Dictionary<AudioSystemDataID, ATLEvent*> Events;
};
struct ATLEvent        final : ATLControl { AudioSystemEventData*       pData = nullptr; };
struct ATLRtpc         final : ATLControl { AudioSystemRtpcData*        pData = nullptr; };
struct ATLSwitchState  final : ATLControl { AudioSystemSwitchStateData* pData = nullptr; };
struct ATLEnvironment  final : ATLControl { AudioSystemEnvironmentData* pData = nullptr; };
struct ATLSoundBank    final : ATLControl { AudioSystemBankData*        pData = nullptr; };
```

Define typed lookup maps using Flax's `Dictionary<AudioSystemDataID, T*>` for each ATL object
type.

### `ATL/AudioTranslationLayer.h` / `.cpp`

```cpp
class AUDIOSYSTEM_API AudioTranslationLayer
{
public:
    Result Startup();
    void Shutdown();
    void Update();

    // Control ID lookup by name (hashed at load time)
    AudioSystemDataID GetTriggerId(StringView name) const;
    AudioSystemDataID GetRtpcId(StringView name) const;
    AudioSystemDataID GetSwitchStateId(StringView name) const;
    AudioSystemDataID GetEnvironmentId(StringView name) const;
    AudioSystemDataID GetBankId(StringView name) const;

    // Request dispatcher — called from AudioSystem::UpdateInternal()
    bool ProcessRequest(Variant&& request, bool sync);

private:
    void OnMasterGainChange(/* CVar event */);
    void OnMuteChange(/* CVar event */);

    AudioMiddleware* _middleware = nullptr;

    Dictionary<AudioSystemDataID, ATLEntity*>      _entities;
    Dictionary<AudioSystemDataID, ATLListener*>    _listeners;
    Dictionary<AudioSystemDataID, ATLTrigger*>     _triggers;
    Dictionary<AudioSystemDataID, ATLEvent*>       _events;
    Dictionary<AudioSystemDataID, ATLRtpc*>        _rtpcs;
    Dictionary<AudioSystemDataID, ATLSwitchState*> _switchStates;
    Dictionary<AudioSystemDataID, ATLEnvironment*> _environments;
    Dictionary<AudioSystemDataID, ATLSoundBank*>   _banks;

    DateTime _lastUpdateTime;
    DateTime _lastFrameTime;
};
```

`ProcessRequest` contains a large `if/else if` dispatch on the request variant type tag, calling
the corresponding `_middleware->*` method and updating the ATL maps accordingly.

**Important:** `AudioTranslationLayer` must never touch Flax scene objects directly. It only
communicates with `AudioMiddleware`. Thread-safety is guaranteed by the caller (`AudioSystem`).

---

## Phase 5 — AudioSystem Singleton + Thread + Requests

**Goal:** Implement the core singleton with request queuing and a dedicated audio thread. Port
from
[`AudioSystem.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Core/AudioSystem.h)
and
[`AudioThread.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Core/AudioThread.h).

### `Core/AudioSystemRequests.h`

Define a tagged-union (or a simple struct hierarchy stored as `Variant`) for each of the 16
request types that `AudioSystem` can enqueue. Mirror the ezEngine request types exactly:

| Request struct | Purpose |
|---|---|
| `RegisterEntityRequest` | Create ATL entity + add to middleware |
| `UnregisterEntityRequest` | Remove entity from ATL + middleware |
| `UpdateEntityTransformRequest` | Push new world transform for an entity |
| `RegisterListenerRequest` | Create ATL listener |
| `UnregisterListenerRequest` | Remove listener |
| `UpdateListenerTransformRequest` | Push new world transform for a listener |
| `LoadTriggerRequest` | Load trigger data into ATL |
| `ActivateTriggerRequest` | Fire a trigger on an entity |
| `StopEventRequest` | Stop a specific event on an entity |
| `StopAllEventsRequest` | Stop all events on an entity |
| `UnloadTriggerRequest` | Unload trigger data from ATL |
| `SetRtpcValueRequest` | Set an RTPC value on an entity |
| `ResetRtpcValueRequest` | Reset an RTPC to default |
| `SetSwitchStateRequest` | Set a switch state on an entity |
| `SetObstructionOcclusionRequest` | Update obstruction/occlusion floats |
| `SetEnvironmentAmountRequest` | Set environment effect amount on entity |
| `LoadBankRequest` | Load a sound bank |
| `UnloadBankRequest` | Unload a sound bank |
| `ShutdownRequest` | Signal the audio thread to stop |

Each request struct has:
- `AudioSystemDataID EntityId`
- `AudioSystemDataID ListenerId` (where applicable)
- `AudioSystemDataID ObjectId` (trigger/rtpc/etc. ID)
- `RequestStatus Status` (pending, success, failure)
- An optional `Function<void(RequestStatus)> Callback` invoked on the **main thread** after
  processing.

### `Core/AudioThread.h` / `.cpp`

```cpp
class AUDIOSYSTEM_API AudioThread : public Thread
{
public:
    AudioThread();
private:
    friend class AudioSystem;
    uint32 Run() override;           // calls AudioSystem::UpdateInternal() in a loop
    AudioSystem* _audioSystem = nullptr;
    volatile bool _keepRunning = true;
};
```

The `Run()` loop:
1. Waits on `_processingEvent` semaphore (released by `AudioSystem` once per frame).
2. Calls `_audioSystem->UpdateInternal()`.
3. Signals `_mainEvent` semaphore so blocking requests can unblock the main thread.
4. Loops while `_keepRunning == true`.

### `Core/AudioSystem.h` / `.cpp`

```cpp
class AUDIOSYSTEM_API AudioSystem
{
public:
    static AudioSystem* Get();       // singleton accessor

    // Called by AudioSystemPlugin::Initialize / Deinitialize
    bool Startup();
    void Shutdown();
    bool IsInitialized() const;

    // Register the middleware BEFORE calling Startup()
    void RegisterMiddleware(AudioMiddleware* middleware);
    void UnregisterMiddleware();

    // Request submission (thread-safe)
    void SendRequest(Variant&& request);           // async
    void SendRequests(Array<Variant>& requests);   // async batch
    void SendRequestSync(Variant&& request);       // blocks until processed

    // Named control ID lookups
    AudioSystemDataID GetTriggerId(StringView name) const;
    AudioSystemDataID GetRtpcId(StringView name) const;
    AudioSystemDataID GetSwitchStateId(StringView name) const;
    AudioSystemDataID GetEnvironmentId(StringView name) const;
    AudioSystemDataID GetBankId(StringView name) const;

    // Control registration (called by ATL after middleware creates data)
    void RegisterTrigger(AudioSystemDataID id, AudioSystemTriggerData* data);
    void RegisterRtpc(AudioSystemDataID id, AudioSystemRtpcData* data);
    void RegisterSwitchState(AudioSystemDataID id, AudioSystemSwitchStateData* data);
    void RegisterEnvironment(AudioSystemDataID id, AudioSystemEnvironmentData* data);
    void RegisterSoundBank(AudioSystemDataID id, AudioSystemBankData* data);

    // Unregistration
    void UnregisterEntity(AudioSystemDataID id);
    void UnregisterListener(AudioSystemDataID id);
    void UnregisterTrigger(AudioSystemDataID id);
    // ... (one per type)

    // Sound interface (called by engine / game code)
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    void SetMasterMute(bool mute);
    bool GetMasterMute() const;
    void SetMasterPaused(bool paused);
    void SetListenerOverrideMode(bool enabled);
    void SetListener(int32 index, Vector3 pos, Vector3 fwd, Vector3 up, Vector3 vel);

    // Called once per frame from AudioSystemPlugin::Update() on the main thread
    void UpdateSound();

    // Config
    void LoadConfiguration(StringView file);

private:
    friend class AudioThread;
    friend class AudioTranslationLayer;

    void UpdateInternal();           // runs on audio thread
    void StartAudioThread();
    void StopAudioThread();
    void QueueRequestCallback(Variant&& request, bool sync);

    static AudioSystem* _instance;

    AudioThread*          _audioThread = nullptr;
    AudioTranslationLayer _atl;

    // Five separate queues to avoid stalls between async/sync/callback paths
    Array<Variant> _requestsQueue;
    Array<Variant> _pendingRequestsQueue;
    Array<Variant> _blockingRequestsQueue;
    Array<Variant> _pendingCallbacksQueue;
    Array<Variant> _blockingCallbacksQueue;

    CriticalSection _requestsMutex;
    CriticalSection _pendingRequestsMutex;
    CriticalSection _blockingRequestsMutex;
    CriticalSection _pendingCallbacksMutex;
    CriticalSection _blockingCallbacksMutex;

    Semaphore _mainEvent;
    Semaphore _processingEvent;

    bool _initialized = false;
    bool _listenerOverrideMode = false;
};
```

`UpdateSound()` (main thread):
1. Swap `_requestsQueue` into `_pendingRequestsQueue` under mutex.
2. Signal `_processingEvent` semaphore to wake the audio thread.
3. Drain `_pendingCallbacksQueue` and invoke each callback on the main thread.

`UpdateInternal()` (audio thread):
1. Lock `_pendingRequestsMutex`, process each request through `_atl.ProcessRequest()`.
2. Move callback entries to `_pendingCallbacksQueue`.
3. Process blocking requests and signal `_mainEvent`.

**Integrate with Flax engine update**: Override `GamePlugin` virtual (or hook into
`Engine::Update` delegate) to call `AudioSystem::Get()->UpdateSound()` once per frame.

---

## Phase 6 — AudioWorldModule (`Core/AudioWorldModule.h` / `.cpp`)

**Goal:** Scene-level state tracker. Owns the list of active environment components and the
default listener. Port from
[`AudioWorldModule.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Core/AudioWorldModule.h).

```cpp
class AUDIOSYSTEM_API AudioWorldModule
{
public:
    // Called from AudioSystem once per frame after ATL update
    void Update();

    // Environment registration (called by environment components on activate/deactivate)
    void AddEnvironment(const AudioSystemEnvironmentComponent* comp);
    void RemoveEnvironment(const AudioSystemEnvironmentComponent* comp);
    const Array<const AudioSystemEnvironmentComponent*>& GetEnvironments() const;

    // Default listener (used for occlusion ray casting)
    void SetDefaultListener(const AudioListenerComponent* listener);
    const AudioListenerComponent* GetDefaultListener() const;

private:
    Array<const AudioSystemEnvironmentComponent*> _environments;
    const AudioListenerComponent* _defaultListener = nullptr;
};
```

`AudioWorldModule` is owned as a member of `AudioSystem` (not a separate allocation). It is
**not** a Flax scene module — it is updated explicitly from `AudioSystem::UpdateSound()`.

---

## Phase 7 — Component System

All components are C++ `Script` subclasses (Flax's `Script` inherits from `SceneObject` and is
attached to `Actor` nodes, equivalent to ezEngine's component/game-object relationship).

Expose all serialised properties to Flax reflection (`API_FIELD()`) so they appear in the
Editor Properties panel.

### 7a. `AudioSystemComponent` (abstract base)

Port from
[`AudioSystemComponent.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Components/AudioSystemComponent.h).

```cpp
// Abstract — never instantiate directly.
// All audio scripts derive from this.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemComponent : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemComponent);
protected:
    virtual void OnUpdate() = 0;
};
```

Define the **ProxyDependent** intermediate base:

```cpp
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemProxyDependentComponent
    : public AudioSystemComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemProxyDependentComponent);
public:
    void OnEnable() override;
    void OnDisable() override;
protected:
    AudioSystemDataID GetEntityId() const;
    AudioProxyComponent* _proxy = nullptr;  // resolved at OnEnable
};
```

`OnEnable` walks the owner `Actor`'s scripts to find a sibling `AudioProxyComponent`. If none
exists, logs a warning and disables the component.

---

### 7b. `AudioProxyComponent`

Port from
[`AudioProxyComponent.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Components/AudioProxyComponent.h).

**Behaviour:**
- `OnEnable`: Allocates a new `AudioSystemDataID` (incrementing counter), sends
  `RegisterEntityRequest` async.
- `OnUpdate`: Computes world-space position, forward, up, velocity (delta position / delta time
  since `_lastTransform`). If the transform changed, sends `UpdateEntityTransformRequest` async.
- `OnDisable`: Sends `UnregisterEntityRequest`. If no other proxy-dependent components reference
  this proxy, unregisters immediately.
- `GetEntityId()`: Returns the assigned ID.

**Serialised properties:** none (transform is derived from owner Actor).

**Environment tracking:** Maintains a `Dictionary<AudioSystemDataID, float>` of
`nextAmount`/`previousAmount` pairs for each environment currently affecting this proxy. Updated
each frame by the environment components.

---

### 7c. `AudioListenerComponent`

Port from
[`AudioListenerComponent.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Components/AudioListenerComponent.h).

**Serialised properties:**

| Field | Type | Default | Description |
|---|---|---|---|
| `IsDefault` | `bool` | `false` | Registers this as the scene default listener |
| `PositionObject` | `Actor*` | `null` | Override actor for position data |
| `OrientationObject` | `Actor*` | `null` | Override actor for orientation data |

**Behaviour:**
- `OnEnable`: Allocates listener ID, sends `RegisterListenerRequest`. If `IsDefault`, calls
  `AudioWorldModule::SetDefaultListener(this)`.
- `OnUpdate`: Reads position from `PositionObject ?? Owner`, orientation from
  `OrientationObject ?? Owner`. Computes velocity. If transform changed sends
  `UpdateListenerTransformRequest` async.
- `OnDisable`: Sends `UnregisterListenerRequest`. If it was the default listener, clears
  `AudioWorldModule::SetDefaultListener(nullptr)`.

---

### 7d. `AudioTriggerComponent`

Port from
[`AudioTriggerComponent.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Components/AudioTriggerComponent.h).

This is the most complex component. Read the full ezEngine header before implementing.

**Serialised properties:**

| Field | Type | Default | Description |
|---|---|---|---|
| `PlayTriggerName` | `String` | `""` | Name of the play trigger as defined in the control collection |
| `StopTriggerName` | `String` | `""` | Name of the stop trigger (optional) |
| `LoadOnInit` | `bool` | `true` | Pre-load the trigger data on component enable |
| `PlayOnActivate` | `bool` | `false` | Auto-play when component becomes active |
| `ObstructionType` | `AudioSystemSoundObstructionType` | `SingleRay` | Ray casting mode |
| `OcclusionCollisionLayer` | `uint8` | `0` | Physics layer for occlusion rays |

**State machine** (`AudioSystemTriggerState`):
```
Invalid → Loading → Ready → Starting → Playing → Stopping → Stopped → Unloading → Invalid
```
- State transitions are driven by request callbacks from the audio thread.
- `IsLoading()`, `IsReady()`, `IsPlaying()`, etc. are convenience getters on the current state.

**Public API:**
- `Play(bool sync = false)` — Loads trigger if not loaded, then activates it.
- `Stop(bool sync = false)` — Activates stop trigger if defined; otherwise sends `StopEventRequest`.

**Obstruction/Occlusion system** (managed by a companion manager object or inline in the
component update):

The `AudioTriggerComponentManager` equivalent in Flax is a static helper updated from
`OnUpdate`. Keep the obstruction state (target, current, per-ray values) as private members.

- `SingleRay`: cast one ray from the entity position toward the default listener. If blocked →
  occlusion = 1, else 0. Smooth toward target using lerp each frame.
- `MultipleRay`: cast 5 rays (center + 4 offset directions perpendicular to the center ray).
  Average the hit results for occlusion; compare center vs edge hits for obstruction.
- After computing values, send `SetObstructionOcclusionRequest` each frame while playing.

Maximum 32 ray values stored per trigger component (`k_MaxOcclusionRaysCount = 32`). Use
`Physics::RayCast()` for each ray.

---

### 7e. `AudioRtpcComponent`

Port from the ezEngine `AudioRtpcComponent.h` (reviewed during research).

**Serialised properties:**

| Field | Type | Default | Description |
|---|---|---|---|
| `RtpcName` | `String` | `""` | Name of the RTPC as defined in the control collection |
| `InitialValue` | `float` | `0.0f` | Value applied on component enable |

**Public API:**
- `SetValue(float value, bool sync = false)` — Clamps to middleware-defined range, sends
  `SetRtpcValueRequest`.
- `ResetValue(bool sync = false)` — Sends `ResetRtpcValueRequest`.
- `GetValue() const` — Returns the last set value.

**Messages:**
- Handles incoming `AudioSystemSetRtpcValueMsg` (so C# scripts can drive the RTPC via message).
- Broadcasts `AudioSystemRtpcValueChangedMsg` when the value changes.

---

### 7f. `AudioSwitchStateComponent`

Port from
[`AudioSwitchStateComponent.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Components/AudioSwitchStateComponent.h).

**Serialised properties:**

| Field | Type | Default | Description |
|---|---|---|---|
| `SwitchStateName` | `String` | `""` | The switch state name to activate on init |

**Public API:**
- `SetState(StringView stateName, bool sync = false)` — Sends `SetSwitchStateRequest`.
- `GetState() const` — Returns the last active state name.

**Messages:**
- Handles `AudioSystemSetSwitchStateMsg`.
- Broadcasts `AudioSystemSwitchStateChangedMsg`.

---

### 7g. `AudioBoxEnvironmentComponent`

Port from
[`AudioBoxEnvironmentComponent.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Components/AudioBoxEnvironmentComponent.h).

Inherits from `AudioSystemEnvironmentComponent` (abstract, handles `OnActivated`/`OnDeactivated`
by calling `AudioWorldModule::AddEnvironment` / `RemoveEnvironment`).

**Serialised properties:**

| Field | Type | Default | Description |
|---|---|---|---|
| `EnvironmentName` | `String` | `""` | Name of the environment effect in the control collection |
| `HalfExtents` | `Vector3` | `(1,1,1)` | Half-size of the box in local space |
| `MaxDistance` | `float` | `1.0f` | Fade-out distance beyond the box surface |

**`GetEnvironmentAmount(AudioProxyComponent* proxy) const`:**
1. Transform proxy world position into local box space.
2. If fully inside box → return `1.0f`.
3. If distance to box surface ≤ `MaxDistance` → return lerp `(1 → 0)`.
4. If outside and beyond `MaxDistance` → return `0.0f`.

Each frame, for every active proxy, the `AudioWorldModule::Update()` loop calls
`GetEnvironmentAmount()` per environment and sends `SetEnvironmentAmountRequest` if the value
changed.

---

### 7h. `AudioSphereEnvironmentComponent`

Same interface as 7g but uses sphere distance instead of AABB:

```
amount = 1.0          if dist < (radius - maxDistance)
amount = lerp(1→0)    if dist in [radius - maxDistance, radius]
amount = 0.0          if dist > radius
```

**Serialised properties:**

| Field | Type | Default | Description |
|---|---|---|---|
| `EnvironmentName` | `String` | `""` | Same as box |
| `Radius` | `float` | `5.0f` | Sphere radius |
| `MaxDistance` | `float` | `1.0f` | Fade distance |

---

### 7i. `AudioAnimationComponent`

Port from
[`AudioAnimationComponent.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EnginePlugins/AudioSystemPlugin/Components/AudioAnimationComponent.h).

**Behaviour:** Listens to Flax animation state machine events (via `AnimationController` on the
owner Actor). When a named animation event fires, looks up a matching `AudioTriggerComponent`
sibling and calls `Play()`.

**Serialised properties:**

| Field | Type | Description |
|---|---|---|
| `EventTriggerMap` | `Dictionary<String, String>` | Maps animation event name → play trigger name |

On each matching animation event, look up the corresponding `AudioTriggerComponent` by trigger
name among siblings and call `Play()`.

---

## Phase 8 — Editor Module

### 8a. `AudioSystemEditorPlugin` (entry point)

Port from
[`EditorPluginAudioSystem.cpp`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EditorPlugins/AudioSystem/EditorPluginAudioSystem/EditorPluginAudioSystem.cpp).

```cpp
class AUDIOSYSTEMEDITOR_API AudioSystemEditorPlugin : public EditorPlugin
{
    DECLARE_SCRIPTING_TYPE(AudioSystemEditorPlugin);
public:
    void InitializeEditor() override;
    void DeinitializeEditor() override;
};
```

`InitializeEditor()` must, **in this order**:

1. Load or create `AudioSystemPreferences` (JSON asset at `<ProjectFolder>/Settings/AudioSystem.json`).
2. Register the 8 component viewport icons (see §8d).
3. Register asset proxy types (see §8e).
4. Add toolbar actions (see §8c).
5. Subscribe to `Editor.PlayModeBegin` → call `AudioSystemPreferences::SyncSettings()`.
6. Subscribe to `Editor.PlayModeEnd` → optionally reset to editor audio state.
7. Subscribe to `GameCooker.DeployFiles` → copy bank files (see §8f).
8. Subscribe to project open/close events → call `SyncSettings()` on project open.

`DeinitializeEditor()` reverses all registrations.

---

### 8b. `AudioSystemPreferences` (settings)

Port from
[`AudioSystemPreferences.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EditorPlugins/AudioSystem/EditorPluginAudioSystem/Preferences/AudioSystemPreferences.h)
and its `.cpp`.

Stored as a Flax JSON settings asset at `<ProjectFolder>/Settings/AudioSystem.json`.

```cpp
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioSystemPreferences : public ScriptingObject
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSystemPreferences);
public:
    API_FIELD() float MasterGain = 1.0f;    // range [0, 1]
    API_FIELD() bool  MuteAudio  = false;

    /// Push current values into the running AudioSystem instance.
    API_FUNCTION() void SyncSettings();

    static AudioSystemPreferences* Get();

private:
    static AudioSystemPreferences* _instance;
};
```

`SyncSettings()`:
1. Call `AudioSystem::Get()->SetMasterVolume(MasterGain)`.
2. Call `AudioSystem::Get()->SetMasterMute(MuteAudio)`.

On engine process restart (or play mode begin), call `SyncSettings()` automatically to restore
the preference values into the fresh runtime.

---

### 8c. Toolbar Actions

Port from
[`AudioSystemActions.h`](https://github.com/SparkyStudios/ezEngine/blob/sparky-studios/Code/EditorPlugins/AudioSystem/EditorPluginAudioSystem/Actions/AudioSystemActions.h)
and its `.cpp`.

Add two controls to `Editor.UI.ToolStrip` (main editor toolbar):

#### Mute toggle button (`AudioSystemMuteAction`)

- Icon: `Icons/SoundOn.png` / `Icons/SoundOff.png` (swap on state change).
- On click: `prefs->MuteAudio = !prefs->MuteAudio; prefs->SyncSettings();`
- Tooltip: `"Mute/Unmute Audio System"`
- Stays in sync with `AudioSystemPreferences::MuteAudio` at all times.

#### Master volume slider (`AudioSystemVolumeSliderAction`)

- Range: `0` to `100` (maps to `0.0f`–`1.0f` gain internally).
- On value change: `prefs->MasterGain = value / 100.0f; prefs->SyncSettings();`
- Tooltip: `"Audio Master Volume: X%"`
- Initial position set from `prefs->MasterGain * 100`.

Both actions subscribe to a preference-changed event so they update their visual state if
preferences are modified from outside (e.g., project load).

---

### 8d. Viewport Icons

Register a 32×32 (and optionally 16×16) PNG icon for each component so it appears in the Scene
Graph and the viewport object picker.

Icons stored at `Source/AudioSystemEditor/Icons/`:

- `AudioProxyComponent.png`
- `AudioListenerComponent.png`
- `AudioTriggerComponent.png`
- `AudioRtpcComponent.png`
- `AudioSwitchStateComponent.png`
- `AudioBoxEnvironmentComponent.png`
- `AudioSphereEnvironmentComponent.png`
- `AudioAnimationComponent.png`

Registration in Flax follows the same pattern as `FmodEditorSystem.cs` from the FlaxFmod
reference plugin:

```csharp
// C# equivalent (use C++ Editor API if available):
Editor.Instance.Icons.AddIcon(typeof(AudioProxyComponent),
    Content.LoadAsync<Texture>(iconPath));
```

---

### 8e. Asset Proxies

Register `CustomEditorPresenter` or `AssetProxy` entries so that the following asset types can
be created via right-click → New in the Content Browser:

| Asset class | Created file | Description |
|---|---|---|
| `AudioBankAsset` | `*.audiobankref` | Reference to a middleware bank file path |
| `AudioTriggerAsset` | `*.audiotrigger` | Named trigger reference |
| `AudioRtpcAsset` | `*.audiortpc` | Named RTPC reference + value range |
| `AudioSwitchStateAsset` | `*.audioswitchstate` | Switch group + state name pair |
| `AudioEnvironmentAsset` | `*.audioenvironment` | Named environment reference |

Each asset is a simple JSON file containing only the control name string (and range metadata
for RTPC). These are **not** compiled binary assets — they are JSON content assets that
`AudioSystem` reads at runtime to populate its name→ID registry.

---

### 8f. Build Deployment Hook

Subscribe to `GameCooker.DeployFiles` in `InitializeEditor()`:

```csharp
GameCooker.DeployFiles += OnDeployFiles;
```

`OnDeployFiles`:
1. Read the list of bank file paths from `AudioSystemPreferences` (or a dedicated project
   settings field `BankFolders: String[]`).
2. Copy each bank directory recursively to the cooked output folder alongside game binaries.

This ensures audio data is bundled with the shipping build without manual steps.

---

## Phase 9 — `.gitignore` Update

Append to the project `.gitignore` (create if missing):

```
# Flax Engine output
/Cache/
/Binaries/
/Logs/
/.vs/
/.idea/
/Output/
```

---

## Coding Rules for the Implementing Agent

1. **Never mutate** shared state without holding the relevant `CriticalSection`. All five request
   queues have their own lock.

2. **Never call** Flax scene/actor APIs from the audio thread. The audio thread only calls
   `AudioMiddleware` methods and operates on ATL data.

3. **File size limit:** keep every `.cpp` and `.h` file under 800 lines. If a file exceeds this,
   extract helpers into a new file.

4. **Function size limit:** keep every function under 50 lines. Extract helpers freely.

5. **No magic numbers.** Use named constants (e.g., `k_MaxOcclusionRaysCount = 32`,
   `k_AudioThreadHz = 100`).

6. **Immutability:** prefer returning new values over mutating in place. All request structs are
   value types passed by move.

7. **Error handling:** every method that can fail must return `bool` or a `Result`-equivalent.
   Log failures via `LOG_WARNING` / `LOG_ERROR`. Never silently swallow errors.

8. **Validate at boundaries:** validate string names (non-empty), IDs (non-zero), and pointer
   arguments (non-null) at every public API entry point before doing anything else.

9. **No backwards-compat shims:** this is a new codebase. Delete unused code, do not add
   `// removed` comments.

10. **Follow the Flax naming convention** observed in FlaxEngine source: `PascalCase` for types
    and methods, `_camelCase` for private members, `camelCase` for local variables, `ALL_CAPS`
    for macros and constants.

---

## Implementation Order Checklist

```
[ ] Phase 1  — Build scaffolding compiles without errors
[ ] Phase 2  — AudioSystemData.h compiles; types visible in reflection
[ ] Phase 3  — AudioMiddleware.h compiles; no concrete implementation yet
[ ] Phase 4  — AudioTranslationLayer compiles with stub ProcessRequest
[ ] Phase 5  — AudioSystem singleton starts/stops audio thread cleanly
[ ] Phase 6  — AudioWorldModule integrates with AudioSystem::UpdateSound
[ ] Phase 7a — AudioSystemComponent abstract base visible in editor
[ ] Phase 7b — AudioProxyComponent registers entity; transform updates seen in log
[ ] Phase 7c — AudioListenerComponent registers listener
[ ] Phase 7d — AudioTriggerComponent: state machine + occlusion rays
[ ] Phase 7e — AudioRtpcComponent: value set/reset
[ ] Phase 7f — AudioSwitchStateComponent: state set
[ ] Phase 7g — AudioBoxEnvironmentComponent: amount computed correctly
[ ] Phase 7h — AudioSphereEnvironmentComponent: amount computed correctly
[ ] Phase 7i — AudioAnimationComponent: maps anim events to triggers
[ ] Phase 8a — EditorPlugin initialises without crash
[ ] Phase 8b — AudioSystemPreferences persisted and loaded correctly
[ ] Phase 8c — Toolbar mute button and volume slider visible and functional
[ ] Phase 8d — Component icons visible in Scene Graph panel
[ ] Phase 8e — Asset proxies appear in Content Browser right-click menu
[ ] Phase 8f — Bank files copied to output on GameCooker build
[ ] Phase 9  — .gitignore excludes Cache/, Binaries/, Logs/
```
