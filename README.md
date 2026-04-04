# Flax Audio System Plugin

Middleware-agnostic audio system for [Flax Engine](https://flaxengine.com/).

This repository contains two modules:

- `AudioSystem` (runtime): request pipeline, scene actors, and gameplay scripts
- `AudioSystemEditor` (editor): settings UI, toolbar integration, and build deploy hook

Repository: <https://github.com/SparkyStudios/FlaxAudioSystemPlugin>

## Features

- Middleware abstraction via [`AudioMiddleware`](Source/AudioSystem/Core/AudioMiddleware.h) (implement your own backend as a native plugin)
- Threaded request processing (`AudioSystem` + `AudioTranslationLayer`) with main-thread callbacks
- Scene actors:
  - `AudioProxyActor`: spatial audio entity that tracks transform
  - `AudioListenerActor`: spatial listener (supports default listener + override actors)
  - `AudioBoxEnvironmentActor` / `AudioSphereEnvironmentActor`: environment zones with falloff
- Gameplay scripts:
  - `AudioTriggerScript`: play/stop triggers, optional obstruction/occlusion raycasts
  - `AudioRtpcScript`: set/reset RTPC values
  - `AudioSwitchStateScript`: set switch states
  - `AudioAnimationScript` + `AudioAnimEvent`: trigger audio from animation events
- Editor integration:
  - “Audio System” settings with master gain + mute
  - game cooking deploy hook that forwards to middleware (`DeployFiles`)

## Requirements

- Flax Engine `>= 0.0.6167` (see [AudioSystem.flaxproj](AudioSystem.flaxproj))

## Installation

Add this project as a reference to your Flax game project, then rebuild.

1. Clone this repository somewhere on disk (commonly next to your game project).
2. Edit your game `.flaxproj` and add a reference to `AudioSystem.flaxproj`:

```json
{
  "References": [
    { "Name": "path/to/FlaxAudioSystemPlugin/AudioSystem.flaxproj" }
  ]
}
```

1. Regenerate project files and build (or open the project in Flax Editor and let it compile).

## Quick Start (Runtime)

This repository provides the audio system, but does not ship with an actual middleware backend. To get sound, you need to load a backend plugin that implements [`AudioMiddleware`](Source/AudioSystem/Core/AudioMiddleware.h) and registers it with the system.

### 1) Bind a middleware backend

In your backend plugin (or game code), register the middleware and start the audio system:

```cpp
#include <AudioSystem/Core/AudioSystem.h>
#include <AudioSystem/Core/AudioMiddleware.h>

void BindAudioMiddleware(AudioMiddleware* middleware)
{
    auto* audioSystem = AudioSystem::Get();
    audioSystem->RegisterMiddleware(middleware);
    audioSystem->Startup();
}
```

If your backend uses a JSON configuration asset, forward it to the system:

```cpp
AudioSystem::Get()->LoadConfiguration(TEXT("Content/Audio/MiddlewareConfig.json"));
```

### 2) Add a listener

- Place an `AudioListenerActor` in the scene (often parented to the camera).
- Set `IsDefault = true` to make it the scene-wide default listener (used by obstruction/occlusion).

### 3) Add an emitter

- Place an `AudioProxyActor` where you want a spatial emitter.
- Add `AudioTriggerScript` on the same actor and set `PlayTriggerName` to a trigger registered by your middleware.
- Call `Play()` from code or enable `PlayOnActivate`.

### 4) Optional: environments and parameters

- Add an `AudioBoxEnvironmentActor` or `AudioSphereEnvironmentActor` and set `EnvironmentName` to drive environment sends for proxies inside the zone.
- Add `AudioRtpcScript` for runtime parameters.
- Add `AudioSwitchStateScript` for switch states.

## C# Convenience API

For common operations from gameplay scripts, use [`AudioManager`](Source/AudioSystem/CSharp/AudioManager.cs):

```csharp
using AudioSystem;
using FlaxEngine;

AudioManager.Play(actor, "Footstep");
AudioManager.SetRtpc(actor, "Speed", 0.75f);
AudioManager.SetSwitchState(actor, "Surface_Stone");
```

## License

Apache License 2.0 (<https://www.apache.org/licenses/LICENSE-2.0>).
