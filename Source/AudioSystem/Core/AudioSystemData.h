#pragma once

#include <Engine/Core/Types/BaseTypes.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Scripting/ScriptingObject.h>

// ============================================================================
//  Primitive ID types
// ============================================================================

/// Unique identifier for any audio system data object (entity, trigger, RTPC, …).
using AudioSystemDataID = uint64;

/// Unique identifier for an audio control (parameter, switch, …) exposed to the middleware.
using AudioSystemControlID = uint64;

/// Sentinel value that means "no valid ID assigned".
constexpr AudioSystemDataID INVALID_AUDIO_SYSTEM_ID = 0;

// ============================================================================
//  AudioSystemTransform
// ============================================================================

/// \brief Spatial description of an audio entity: position, velocity, and orientation.
///
/// Passed to the middleware every frame for every active listener and sound source
/// so that the middleware can compute 3-D panning, distance attenuation, and Doppler.
API_STRUCT() struct AUDIOSYSTEM_API AudioSystemTransform
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AudioSystemTransform);

    /// World-space position of the entity.
    API_FIELD() Vector3 Position = Vector3::Zero;

    /// World-space velocity of the entity (used for Doppler effect).
    API_FIELD() Vector3 Velocity = Vector3::Zero;

    /// Unit vector pointing in the forward direction of the entity.
    API_FIELD() Vector3 Forward = Vector3::Forward;

    /// Unit vector pointing in the up direction of the entity.
    API_FIELD() Vector3 Up = Vector3::Up;

    FORCE_INLINE bool operator==(const AudioSystemTransform& other) const
    {
        return Position == other.Position
            && Velocity == other.Velocity
            && Forward  == other.Forward
            && Up       == other.Up;
    }

    FORCE_INLINE bool operator!=(const AudioSystemTransform& other) const
    {
        return !(*this == other);
    }
};

// ============================================================================
//  AudioSystemSoundObstructionType
// ============================================================================

/// \brief Controls how many physics rays are cast to compute obstruction/occlusion.
///
/// A higher quality mode casts more rays per audio frame and therefore costs more
/// CPU time. Choose the mode that suits the scene geometry complexity.
API_ENUM() enum class AUDIOSYSTEM_API AudioSystemSoundObstructionType : uint8
{
    /// No ray casting is performed. Sound passes through all geometry.
    None = 0,

    /// One ray is cast from the listener to the sound source centre.
    /// Only occlusion (full blockage) is detected — no partial obstruction.
    SingleRay = 1,

    /// Five rays are cast in a cross pattern around the source.
    /// Both obstruction (partial blockage) and occlusion (full blockage) are detected.
    MultipleRay = 2,
};

// ============================================================================
//  AudioSystemTriggerState
// ============================================================================

/// \brief Lifecycle state of an audio trigger instance.
///
/// A trigger moves through these states as it is loaded, started, played back,
/// stopped and finally unloaded. Transitions are driven by the AudioSystem and
/// must only be advanced by the audio thread.
API_ENUM() enum class AUDIOSYSTEM_API AudioSystemTriggerState : uint8
{
    /// State is not initialised or has become invalid.
    Invalid = 0,

    /// The trigger is actively playing audio.
    Playing = 1,

    /// The trigger has been loaded and is ready to start.
    Ready = 2,

    /// The trigger's data is currently being loaded into memory.
    Loading = 3,

    /// The trigger's data is currently being unloaded from memory.
    Unloading = 4,

    /// A play request has been issued; the middleware is preparing to start.
    Starting = 5,

    /// A stop request has been issued; the middleware is fading out.
    Stopping = 6,

    /// Playback has fully stopped.
    Stopped = 7,
};

// ============================================================================
//  AudioSystemEventState
// ============================================================================

/// \brief Lifecycle state of an audio event.
API_ENUM() enum class AUDIOSYSTEM_API AudioSystemEventState : uint8
{
    /// State is not initialised or has become invalid.
    Invalid = 0,

    /// The event is actively playing.
    Playing = 1,

    /// The event's data is currently being loaded.
    Loading = 2,

    /// The event's data is currently being unloaded.
    Unloading = 3,
};

// ============================================================================
//  Opaque base data classes
//
//  Middleware back-ends subclass these to store their own handle/ID fields.
//  They are abstract: the AudioSystem never instantiates them directly.
//  DECLARE_SCRIPTING_TYPE_NO_SPAWN prevents Flax from exposing a C# constructor.
// ============================================================================

/// \brief Opaque base for a middleware audio entity (e.g. a Wwise/FMOD game object).
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemEntityData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemEntityData);
public:
    virtual ~AudioSystemEntityData() = default;
};

/// \brief Opaque base for a middleware listener descriptor.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemListenerData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemListenerData);
public:
    virtual ~AudioSystemListenerData() = default;
};

/// \brief Opaque base for a middleware trigger descriptor (event template / cue).
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemTriggerData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemTriggerData);
public:
    virtual ~AudioSystemTriggerData() = default;
};

/// \brief Opaque base for a Real-Time Parameter Control (RTPC/parameter) descriptor.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemRtpcData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemRtpcData);
public:
    virtual ~AudioSystemRtpcData() = default;
};

/// \brief Opaque base for a switch-state descriptor (a specific value of a switch/state).
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemSwitchStateData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemSwitchStateData);
public:
    virtual ~AudioSystemSwitchStateData() = default;
};

/// \brief Opaque base for an environment (aux bus / reverb send) descriptor.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemEnvironmentData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemEnvironmentData);
public:
    virtual ~AudioSystemEnvironmentData() = default;
};

/// \brief Opaque base for a running event/trigger instance owned by the middleware.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemEventData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemEventData);
public:
    virtual ~AudioSystemEventData() = default;
};

/// \brief Opaque base for an audio source (streaming source / voice) descriptor.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemSourceData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemSourceData);
public:
    virtual ~AudioSystemSourceData() = default;
};

/// \brief Opaque base for a sound bank descriptor.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemBankData : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemBankData);
public:
    virtual ~AudioSystemBankData() = default;
};
