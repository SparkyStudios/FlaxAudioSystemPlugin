#pragma once

#include <Engine/Core/Math/Vector3.h>
#include <Engine/Level/Actor.h>
#include <Engine/Scripting/ScriptingObjectReference.h>

#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioListenerActor
//
//  Registers a spatial listener with the audio middleware. Each active
//  listener receives its own unique ID (allocated from a static counter
//  starting at 2000, separate from entity IDs).
//
//  Lifecycle:
//    OnBeginPlay       — allocates a listener ID and sends RegisterListenerRequest.
//                        If IsDefault is true, registers as the scene default listener.
//                        Binds OnFrameUpdate to Scripting::Update.
//    OnFrameUpdate     — reads position/orientation from the override actors (or this
//                        Actor) and sends UpdateListenerTransformRequest if the
//                        transform changed. Polls every frame when override actors are
//                        active since OnTransformChanged only fires for THIS actor.
//    OnTransformChanged — marks the transform dirty so OnFrameUpdate will flush it.
//    OnEndPlay         — sends UnregisterListenerRequest, clears any default-listener
//                        registration, and unbinds OnFrameUpdate.
// ============================================================================

/// \brief An Actor that acts as an audio listener in the scene.
///
/// Place this actor in the scene tree (typically as a child of a Camera) to
/// register a spatial listener with the audio middleware. The listener tracks
/// the world-space transform of either this Actor or separate override actors
/// for position and orientation, and propagates the result to the middleware
/// each frame.
API_CLASS(Attributes="ActorContextMenu(\"New/Audio/Audio Listener\")")
class AUDIOSYSTEM_API AudioListenerActor : public Actor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioListenerActor);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// When true, registers this listener as the scene-wide default listener
    /// used by the obstruction/occlusion system.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Registers this as the scene default listener.\")")
    bool IsDefault = false;

    /// Optional actor to use as the source of position data.
    /// If null, this Actor's position is used instead.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Override actor for position data. Uses this Actor if null.\")")
    ScriptingObjectReference<Actor> PositionObject;

    /// Optional actor to use as the source of orientation data.
    /// If null, this Actor's orientation is used instead.
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Override actor for orientation data. Uses this Actor if null.\")")
    ScriptingObjectReference<Actor> OrientationObject;

    // ========================================================================
    //  Actor lifecycle overrides
    // ========================================================================

    /// Registers the viewport icon with the scene rendering system.
    void OnEnable() override;

    /// Unregisters the viewport icon from the scene rendering system.
    void OnDisable() override;

    /// Allocates a unique listener ID, registers this listener with the middleware,
    /// and binds the per-frame update delegate.
    /// If IsDefault is true, sets this as the scene default listener.
    void OnBeginPlay() override;

    /// Unbinds the per-frame update delegate, unregisters this listener from the
    /// middleware, and clears the default listener reference if applicable.
    void OnEndPlay() override;

    /// Marks the transform dirty so OnFrameUpdate will flush the change to the
    /// middleware on the next frame.
    void OnTransformChanged() override;

    // ========================================================================
    //  Listener ID
    // ========================================================================

    /// \return The unique listener ID assigned in OnBeginPlay.
    ///         Returns INVALID_AUDIO_SYSTEM_ID before OnBeginPlay has run.
    API_FUNCTION() uint64 GetListenerId() const;

private:
    // ========================================================================
    //  Helpers
    // ========================================================================

    /// Called every frame via Scripting::Update. Computes the current transform
    /// and submits an UpdateListenerTransform request if it has changed.
    void OnFrameUpdate();

    /// Build an AudioSystemTransform using the configured position/orientation sources.
    AudioSystemTransform ComputeCurrentTransform() const;

    // ========================================================================
    //  State
    // ========================================================================

    /// Unique listener ID allocated in OnBeginPlay; reset to INVALID_AUDIO_SYSTEM_ID on end-play.
    AudioSystemDataID _listenerId = INVALID_AUDIO_SYSTEM_ID;

    /// Transform captured at the end of the previous OnFrameUpdate call.
    AudioSystemTransform _lastTransform;

    /// Whether _lastTransform holds a valid previous-frame snapshot.
    bool _hasLastTransform = false;

    /// Set by OnTransformChanged; cleared after OnFrameUpdate flushes the change.
    bool _transformDirty = false;

    /// Whether this actor was registered as the default listener in OnBeginPlay.
    bool _registeredAsDefault = false;

    /// Source of monotonically increasing listener IDs. Listener IDs start at
    /// 2000 to avoid collisions with the entity ID space (starting at 1000).
    static AudioSystemDataID _nextListenerId;
};
