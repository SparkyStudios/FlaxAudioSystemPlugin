// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <Engine/Core/Math/Vector3.h>
#include <Engine/Core/Types/BaseTypes.h>
#include <Engine/Scripting/ScriptingObject.h>

// ============================================================================
//  Primitive ID types
// ============================================================================

/// <summary>
/// Unique identifier for any audio system data object (entity, trigger, RTPC, …).
/// </summary>
using AudioSystemDataID = uint64;

/// <summary>
/// Unique identifier for an audio control (parameter, switch, …) exposed to the middleware.
/// </summary>
using AudioSystemControlID = uint64;

/// <summary>
/// Sentinel value that means "no valid ID assigned".
/// </summary>
constexpr AudioSystemDataID INVALID_AUDIO_SYSTEM_ID = 0;

// ============================================================================
//  AudioSystemTransform
// ============================================================================

/// <summary>
/// Spatial description of an audio entity: position, velocity, and orientation.
///
/// Passed to the middleware every frame for every active listener and sound source
/// so that the middleware can compute effects such as 3-D panning, distance attenuation, and Doppler.
/// </summary>
API_STRUCT()
struct AUDIOSYSTEM_API AudioSystemTransform
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AudioSystemTransform);

    /// <summary>
    /// World-space position of the entity.
    /// </summary>
    API_FIELD()
    Vector3 Position = Vector3::Zero;

    /// <summary>
    /// World-space velocity of the entity (used for Doppler effect).
    /// </summary>
    API_FIELD()
    Vector3 Velocity = Vector3::Zero;

    /// <summary>
    /// Unit vector pointing in the forward direction of the entity.
    /// </summary>
    API_FIELD()
    Vector3 Forward = Vector3::Forward;

    /// <summary>
    /// Unit vector pointing in the up direction of the entity.
    /// </summary>
    API_FIELD()
    Vector3 Up = Vector3::Up;

    FORCE_INLINE bool operator==(const AudioSystemTransform& other) const
    {
        return Position == other.Position && Velocity == other.Velocity && Forward == other.Forward && Up == other.Up;
    }

    FORCE_INLINE bool operator!=(const AudioSystemTransform& other) const
    {
        return !(*this == other);
    }
};

// ============================================================================
//  AudioSystemSoundObstructionType
// ============================================================================

/// <summary>
/// Controls how many physics rays are cast to compute obstruction/occlusion.
///
/// A higher quality mode casts more rays per audio frame and therefore costs more
/// CPU time. Choose the mode that suits the scene geometry complexity.
/// </summary>
API_ENUM()
enum class AUDIOSYSTEM_API AudioSystemSoundObstructionType : uint8
{
    /// <summary>
    /// No ray casting is performed. Sound passes through all geometry.
    /// </summary>
    None = 0,

    /// <summary>
    /// One ray is cast from the listener to the sound source centre.
    /// Only occlusion (full blockage) is detected — no partial obstruction.
    /// </summary>
    SingleRay = 1,

    /// <summary>
    /// Five rays are cast in a cross pattern around the source.
    /// Both obstruction (partial blockage) and occlusion (full blockage) are detected.
    /// </summary>
    MultipleRay = 2,
};

// ============================================================================
//  AudioSystemTriggerState
// ============================================================================

/// <summary>
/// Lifecycle state of an audio trigger instance.
///
/// A trigger moves through these states as it is loaded, started, played back,
/// stopped and finally unloaded. Transitions are driven by the AudioSystem and
/// must only be advanced by the audio thread.
/// </summary>
API_ENUM()
enum class AUDIOSYSTEM_API AudioSystemTriggerState : uint8
{
    /// <summary>
    /// State is not initialised or has become invalid.
    /// </summary>
    Invalid = 0,

    /// <summary>
    /// The trigger has successfully played the audio.
    ///
    /// The audio may be still playing or have stopped playing naturally. This state only indicates that
    /// the last play request was successful.
    /// </summary>
    Played = 1,

    /// <summary>
    /// The trigger has been loaded and is ready to start.
    /// </summary>
    Ready = 2,

    /// <summary>
    /// The trigger's data is currently being loaded into memory.
    /// </summary>
    Loading = 3,

    /// <summary>
    /// The trigger's data is currently being unloaded from memory.
    /// </summary>
    Unloading = 4,

    /// <summary>
    /// A play request has been issued; the middleware is preparing to start.
    /// </summary>
    Playing = 5,

    /// <summary>
    /// A stop request has been issued; the middleware is fading out.
    /// </summary>
    Stopping = 6,

    /// <summary>
    /// Playback has fully stopped.
    /// </summary>
    Stopped = 7,
};

// ============================================================================
//  AudioSystemEventState
// ============================================================================

/// <summary>
/// Lifecycle state of an audio event.
/// </summary>
API_ENUM()
enum class AUDIOSYSTEM_API AudioSystemEventState : uint8
{
    /// <summary>
    /// State is not initialised or has become invalid.
    /// </summary>
    Invalid = 0,

    /// <summary>
    /// The event is actively playing.
    /// </summary>
    Playing = 1,

    /// <summary>
    /// The event's data is currently being loaded.
    /// </summary>
    Loading = 2,

    /// <summary>
    /// The event's data is currently being unloaded.
    /// </summary>
    Unloading = 3,
};

// ============================================================================
//  Opaque base data classes
//
//  Middleware back-ends subclass these to store their own fields.
//  They are abstract: the AudioSystem never instantiates them directly.
// ============================================================================

/// <summary>
/// Opaque base for a middleware audio entity (e.g. a Wwise/FMOD game object).
/// </summary>
class AUDIOSYSTEM_API AudioSystemEntityData
{
  public:
    virtual ~AudioSystemEntityData() = default;
};

/// <summary>
/// Opaque base for a middleware listener descriptor.
/// </summary>
class AUDIOSYSTEM_API AudioSystemListenerData
{
  public:
    virtual ~AudioSystemListenerData() = default;
};

/// <summary>
/// Opaque base for a middleware trigger descriptor (event template / cue).
/// </summary>
class AUDIOSYSTEM_API AudioSystemTriggerData
{
  public:
    virtual ~AudioSystemTriggerData() = default;
};

/// <summary>
/// Opaque base for a Real-Time Parameter Control (RTPC/parameter) descriptor.
/// </summary>
class AUDIOSYSTEM_API AudioSystemRtpcData
{
  public:
    virtual ~AudioSystemRtpcData() = default;
};

/// <summary>
/// Opaque base for a switch-state descriptor (a specific value of a switch/state).
/// </summary>
class AUDIOSYSTEM_API AudioSystemSwitchStateData
{
  public:
    virtual ~AudioSystemSwitchStateData() = default;
};

/// <summary>
/// Opaque base for an environment (aux bus / reverb send) descriptor.
/// </summary>
class AUDIOSYSTEM_API AudioSystemEnvironmentData
{
  public:
    virtual ~AudioSystemEnvironmentData() = default;
};

/// <summary>
/// Opaque base for a running event/trigger instance owned by the middleware.
/// </summary>
class AUDIOSYSTEM_API AudioSystemEventData
{
  public:
    virtual ~AudioSystemEventData() = default;
};

/// <summary>
/// Opaque base for an audio source (streaming source / voice) descriptor.
/// </summary>
class AUDIOSYSTEM_API AudioSystemSourceData
{
  public:
    virtual ~AudioSystemSourceData() = default;
};

/// <summary>
/// Opaque base for a sound bank descriptor.
/// </summary>
class AUDIOSYSTEM_API AudioSystemBankData
{
  public:
    virtual ~AudioSystemBankData() = default;
};
