#pragma once

#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Scripting/Script.h>

#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioProxyComponent
//
//  Attaches an audio entity to an Actor so that the audio middleware tracks
//  its world-space transform every frame. Other audio scripts (trigger,
//  RTPC, switch-state, etc.) look up this component on the same Actor to
//  obtain the shared entity ID.
//
//  Lifecycle:
//    OnEnable  — allocates a unique AudioSystemDataID and sends
//                RegisterEntityRequest to the audio system.
//    OnUpdate  — computes world-space position, orientation and velocity;
//                sends UpdateEntityTransformRequest if the transform changed.
//    OnDisable — sends UnregisterEntityRequest.
// ============================================================================

/// \brief Represents a spatial audio entity attached to a Flax Actor.
///
/// Tracks the owner Actor's world-space transform each frame and propagates
/// changes to the audio middleware via the AudioSystem request queue.
/// Environment components update per-environment send amounts via
/// SetEnvironmentAmount / GetEnvironmentAmount.
API_CLASS() class AUDIOSYSTEM_API AudioProxyComponent : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioProxyComponent);

public:
    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// Allocates a unique entity ID and registers this entity with the middleware.
    void OnEnable() override;

    /// Computes and submits a transform update if the owner Actor has moved.
    void OnUpdate() override;

    /// Unregisters this entity from the middleware.
    void OnDisable() override;

    // ========================================================================
    //  Entity ID
    // ========================================================================

    /// \return The unique AudioSystemDataID assigned to this proxy.
    ///         Returns INVALID_AUDIO_SYSTEM_ID before OnEnable has run.
    API_FUNCTION() AudioSystemDataID GetEntityId() const;

    // ========================================================================
    //  Environment amounts
    // ========================================================================

    /// Update the wet-send amount for one environment affecting this proxy.
    ///
    /// If the amount differs from the currently stored value the new value is
    /// cached and a SetEnvironmentAmountRequest is dispatched to the audio system.
    ///
    /// \param envId   ID of the environment (from AudioSystem::GetEnvironmentId).
    /// \param amount  Send amount in [0, 1].
    API_FUNCTION() void SetEnvironmentAmount(AudioSystemDataID envId, float amount);

    /// \return The cached send amount for \p envId, or 0.0f if this proxy is
    ///         not currently inside that environment.
    API_FUNCTION() float GetEnvironmentAmount(AudioSystemDataID envId) const;

private:
    // ========================================================================
    //  Helpers
    // ========================================================================

    /// Build an AudioSystemTransform from the owner Actor's current world state.
    AudioSystemTransform ComputeCurrentTransform() const;

    // ========================================================================
    //  State
    // ========================================================================

    /// Unique identifier allocated in OnEnable; reset to INVALID_AUDIO_SYSTEM_ID on disable.
    AudioSystemDataID _entityId = INVALID_AUDIO_SYSTEM_ID;

    /// Transform captured at the end of the previous OnUpdate call.
    /// Used to derive velocity (delta-position / delta-time).
    AudioSystemTransform _lastTransform;

    /// Whether _lastTransform holds a valid previous-frame snapshot.
    bool _hasLastTransform = false;

    /// Per-environment wet-send amounts currently applied to this proxy.
    /// Environment components write into this dictionary each frame.
    Dictionary<AudioSystemDataID, float> _environmentAmounts;

    /// Source of monotonically increasing entity IDs. Incremented on the
    /// main thread only — no atomics required for a single-threaded caller.
    static AudioSystemDataID _nextEntityId;
};
