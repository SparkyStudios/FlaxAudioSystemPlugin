#pragma once

#include <Engine/Core/Math/Vector3.h>
#include <Engine/Scripting/ScriptingObjectReference.h>

#include "AudioSystemComponent.h"
#include "../Core/AudioSystemData.h"

// Forward declarations
class Actor;

// ============================================================================
//  AudioListenerComponent
//
//  Registers a spatial listener with the audio middleware. Each active
//  listener receives its own unique ID (allocated from a static counter
//  starting at 2000, separate from entity IDs).
//
//  Lifecycle:
//    OnEnable  — allocates a listener ID and sends RegisterListenerRequest.
//                If IsDefault is true, registers as the scene default listener.
//    OnUpdate  — reads position/orientation from the override actors (or the
//                owner Actor) and sends UpdateListenerTransformRequest if the
//                transform changed.
//    OnDisable — sends UnregisterListenerRequest and clears any default-listener
//                registration.
// ============================================================================

/// \brief Attaches an audio listener to a Flax Actor.
///
/// The listener tracks the world-space transform of either the owner Actor or
/// separate override actors for position and orientation, and propagates the
/// result to the audio middleware each frame.
API_CLASS() class AUDIOSYSTEM_API AudioListenerComponent : public AudioSystemComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioListenerComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// When true, registers this listener as the scene-wide default listener
    /// used by the obstruction/occlusion system.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Registers this as the scene default listener.\")")
    bool IsDefault = false;

    /// Optional actor to use as the source of position data.
    /// If null, the owner Actor's position is used instead.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Override actor for position data. Uses the owner Actor if null.\")")
    ScriptingObjectReference<Actor> PositionObject;

    /// Optional actor to use as the source of orientation data.
    /// If null, the owner Actor's orientation is used instead.
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Override actor for orientation data. Uses the owner Actor if null.\")")
    ScriptingObjectReference<Actor> OrientationObject;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// Allocates a unique listener ID and registers this listener with the middleware.
    /// If IsDefault is true, sets this as the scene default listener.
    void OnEnable() override;

    /// Reads the current transform and submits an update if it has changed.
    void OnUpdate() override;

    /// Unregisters this listener from the middleware and clears the default
    /// listener reference if this component was registered as the default.
    void OnDisable() override;

    // ========================================================================
    //  Listener ID
    // ========================================================================

    /// \return The unique listener ID assigned in OnEnable.
    ///         Returns INVALID_AUDIO_SYSTEM_ID before OnEnable has run.
    API_FUNCTION() uint64 GetListenerId() const;

private:
    // ========================================================================
    //  Helpers
    // ========================================================================

    /// Build an AudioSystemTransform using the configured position/orientation sources.
    AudioSystemTransform ComputeCurrentTransform() const;

    // ========================================================================
    //  State
    // ========================================================================

    /// Unique listener ID allocated in OnEnable; reset to INVALID_AUDIO_SYSTEM_ID on disable.
    AudioSystemDataID _listenerId = INVALID_AUDIO_SYSTEM_ID;

    /// Transform captured at the end of the previous OnUpdate call.
    AudioSystemTransform _lastTransform;

    /// Whether _lastTransform holds a valid previous-frame snapshot.
    bool _hasLastTransform = false;

    /// Whether this component was registered as the default listener in OnEnable.
    bool _registeredAsDefault = false;

    /// Source of monotonically increasing listener IDs. Listener IDs start at
    /// 2000 to avoid collisions with the entity ID space (starting at 1000).
    static AudioSystemDataID _nextListenerId;
};
