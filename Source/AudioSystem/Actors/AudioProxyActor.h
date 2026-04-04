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

#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Level/Actor.h>

#include "../Core/AudioSystemData.h"

/// <summary>
/// Actor that represents a spatial audio entity in the middleware.
///
/// Tracks this Actor's world-space transform each frame and propagates
/// changes to the audio middleware via the AudioSystem request queue.
/// Environment components update per-environment send amounts via
/// SetEnvironmentAmount / GetEnvironmentAmount.
/// </summary>
API_CLASS(Attributes = "ActorContextMenu(\"New/Audio/Audio Proxy\")")
class AUDIOSYSTEM_API AudioProxyActor : public Actor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioProxyActor);

  public:
    // ========================================================================
    //  Actor lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Registers the viewport icon with the scene rendering system.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// Unregisters the viewport icon from the scene rendering system.
    /// </summary>
    void OnDisable() override;

    /// <summary>
    /// Allocates a unique entity ID, registers this entity with the middleware,
    /// and binds the per-frame update delegate.
    /// </summary>
    void OnBeginPlay() override;

    /// <summary>
    /// Unbinds the per-frame update delegate and unregisters this entity.
    /// </summary>
    void OnEndPlay() override;

    /// <summary>
    /// Marks the transform as dirty so that OnFrameUpdate will submit a
    /// transform update on the next Scripting::Update tick.
    /// </summary>
    void OnTransformChanged() override;

    // ========================================================================
    //  Entity ID
    // ========================================================================

    /// <returns>The unique AudioSystemDataID assigned to this proxy. Returns INVALID_AUDIO_SYSTEM_ID before OnBeginPlay has run.</returns>
    API_FUNCTION()
    uint64 GetEntityId() const;

    // ========================================================================
    //  Environment amounts
    // ========================================================================

    /// <summary>
    /// Update the wet-send amount for one environment affecting this proxy.
    ///
    /// If the amount differs from the currently stored value the new value is
    /// cached and a SetEnvironmentAmount request is dispatched to the audio system.
    /// </summary>
    /// <param name="envId">ID of the environment (from AudioSystem::GetEnvironmentId).</param>
    /// <param name="amount">Send amount in [0, 1].</param>
    API_FUNCTION()
    void SetEnvironmentAmount(uint64 envId, float amount);

    /// <returns>The cached send amount for `envId`, or 0.0f if this proxy is not currently inside that environment.</returns>
    API_FUNCTION()
    float GetEnvironmentAmount(uint64 envId) const;

  private:
    // ========================================================================
    //  Helpers
    // ========================================================================

    /// <summary>
    /// Called each game frame via Scripting::Update. Submits a transform
    /// update request to the audio system if the transform has been marked dirty.
    /// </summary>
    void OnFrameUpdate();

    // ========================================================================
    //  State
    // ========================================================================

    /// <summary>
    /// Unique identifier allocated in OnBeginPlay; reset to INVALID_AUDIO_SYSTEM_ID on end play.
    /// </summary>
    AudioSystemDataID _entityId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// World-space position captured at the end of the previous OnFrameUpdate call.
    /// Used to derive velocity (delta-position / delta-time).
    /// </summary>
    Vector3 _lastPosition = Vector3::Zero;

    /// <summary>
    /// Set to true when OnTransformChanged fires; cleared after OnFrameUpdate sends the request.
    /// </summary>
    bool _transformDirty = false;

    /// <summary>
    /// Whether _lastPosition holds a valid previous-frame snapshot.
    /// </summary>
    bool _hasLastPosition = false;

    /// <summary>
    /// Per-environment wet-send amounts currently applied to this proxy.
    /// Environment components write into this dictionary each frame.
    /// </summary>
    Dictionary<AudioSystemDataID, float> _environmentAmounts;

    /// <summary>
    /// Source of monotonically increasing entity IDs. Incremented on the
    /// main thread only — no atomics required for a single-threaded caller.
    /// </summary>
    static AudioSystemDataID _nextEntityId;
};
