#pragma once

#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Level/Actor.h>

#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioProxyComponent
//
//  An Actor that acts as a spatial audio entity. Other audio scripts
//  (trigger, RTPC, switch-state, etc.) are attached as children and obtain
//  the shared entity ID via GetEntityId().
//
//  Lifecycle:
//    OnBeginPlay       — allocates a unique AudioSystemDataID and sends
//                        RegisterEntity to the audio system. Binds
//                        OnFrameUpdate to Scripting::Update.
//    OnTransformChanged — marks the transform as dirty so that OnFrameUpdate
//                        will submit an UpdateEntityTransform request.
//    OnFrameUpdate     — (Scripting::Update delegate) computes world-space
//                        position, orientation and velocity; sends the
//                        UpdateEntityTransform request if the transform is dirty.
//    OnEndPlay         — unbinds OnFrameUpdate, sends UnregisterEntity.
// ============================================================================

/// \brief Actor that represents a spatial audio entity in the middleware.
///
/// Tracks this Actor's world-space transform each frame and propagates
/// changes to the audio middleware via the AudioSystem request queue.
/// Environment components update per-environment send amounts via
/// SetEnvironmentAmount / GetEnvironmentAmount.
API_CLASS(Attributes="ActorContextMenu(\"New/Audio/Audio Proxy\")")
class AUDIOSYSTEM_API AudioProxyComponent : public Actor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioProxyComponent);

public:
    // ========================================================================
    //  Actor lifecycle overrides
    // ========================================================================

    /// Registers the viewport icon with the scene rendering system.
    void OnEnable() override;

    /// Unregisters the viewport icon from the scene rendering system.
    void OnDisable() override;

    /// Allocates a unique entity ID, registers this entity with the middleware,
    /// and binds the per-frame update delegate.
    void OnBeginPlay() override;

    /// Unbinds the per-frame update delegate and unregisters this entity.
    void OnEndPlay() override;

    /// Marks the transform as dirty so that OnFrameUpdate will submit a
    /// transform update on the next Scripting::Update tick.
    void OnTransformChanged() override;

    // ========================================================================
    //  Entity ID
    // ========================================================================

    /// \return The unique AudioSystemDataID assigned to this proxy.
    ///         Returns INVALID_AUDIO_SYSTEM_ID before OnBeginPlay has run.
    API_FUNCTION() uint64 GetEntityId() const;

    // ========================================================================
    //  Environment amounts
    // ========================================================================

    /// Update the wet-send amount for one environment affecting this proxy.
    ///
    /// If the amount differs from the currently stored value the new value is
    /// cached and a SetEnvironmentAmount request is dispatched to the audio system.
    ///
    /// \param envId   ID of the environment (from AudioSystem::GetEnvironmentId).
    /// \param amount  Send amount in [0, 1].
    API_FUNCTION() void SetEnvironmentAmount(uint64 envId, float amount);

    /// \return The cached send amount for \p envId, or 0.0f if this proxy is
    ///         not currently inside that environment.
    API_FUNCTION() float GetEnvironmentAmount(uint64 envId) const;

private:
    // ========================================================================
    //  Helpers
    // ========================================================================

    /// Called each game frame via Scripting::Update. Submits a transform
    /// update request to the audio system if the transform has been marked dirty.
    void OnFrameUpdate();

    // ========================================================================
    //  State
    // ========================================================================

    /// Unique identifier allocated in OnBeginPlay; reset to INVALID_AUDIO_SYSTEM_ID on end play.
    AudioSystemDataID _entityId = INVALID_AUDIO_SYSTEM_ID;

    /// World-space position captured at the end of the previous OnFrameUpdate call.
    /// Used to derive velocity (delta-position / delta-time).
    Vector3 _lastPosition = Vector3::Zero;

    /// Set to true when OnTransformChanged fires; cleared after OnFrameUpdate sends the request.
    bool _transformDirty = false;

    /// Whether _lastPosition holds a valid previous-frame snapshot.
    bool _hasLastPosition = false;

    /// Per-environment wet-send amounts currently applied to this proxy.
    /// Environment components write into this dictionary each frame.
    Dictionary<AudioSystemDataID, float> _environmentAmounts;

    /// Source of monotonically increasing entity IDs. Incremented on the
    /// main thread only — no atomics required for a single-threaded caller.
    static AudioSystemDataID _nextEntityId;
};
