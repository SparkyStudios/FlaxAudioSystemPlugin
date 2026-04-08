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

#include <Engine/Core/Types/String.h>

#include "../Core/AudioSystemData.h"
#include "AudioSystemScript.h"

// ============================================================================
//  Constants
// ============================================================================

/// <summary>
/// Maximum number of rays used for multi-ray obstruction/occlusion sampling.
/// </summary>
constexpr int32 k_MaxOcclusionRaysCount = 32;

/// <summary>
/// Smoothing factor for lerping obstruction and occlusion toward target values.
/// </summary>
constexpr float k_ObstructionSmoothingFactor = 0.1f;

/// <summary>
/// Number of rays cast in multi-ray mode (centre + 4 perpendicular offsets).
/// </summary>
constexpr int32 k_MultiRayCount = 5;

/// <summary>
/// Angular offset radius (in world units) used for multi-ray side rays.
/// </summary>
constexpr float k_MultiRayOffsetRadius = 0.25f;

/// <summary>
/// Plays and stops named audio triggers on an Actor with obstruction/occlusion support.
///
/// Requires a sibling AudioProxyActor on the same Actor.
/// The component manages loading, playing, and stopping trigger data, and
/// optionally performs physics ray casts each frame to compute the
/// obstruction and occlusion values sent to the audio middleware.
/// </summary>
API_CLASS(Attributes = "FlaxEngine.Category(\"Audio System\")")
class AUDIOSYSTEM_API AudioTriggerScript : public AudioProxyDependentScript
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioTriggerScript);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// Name of the play trigger. Resolved to an ID in OnEnable.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"Name of the play trigger.\")")
    String PlayTriggerName;

    /// <summary>
    /// Name of the optional stop trigger. When empty, StopEvent is used instead.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1), Tooltip(\"Name of the stop trigger. If empty, a StopEvent is sent using the play trigger name, which may not work on all middleware.\")")
    String StopTriggerName;

    /// <summary>
    /// If true, trigger data is loaded in OnEnable so it is ready to play immediately.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(2), Tooltip(\"Pre-load trigger data on component enable.\")")
    bool LoadOnInit = true;

    /// <summary>
    /// If true, Play() is called automatically once loading completes.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(3), Tooltip(\"Auto-play when loading completes.\")")
    bool PlayOnActivate = false;

    /// <summary>
    /// Controls how many rays are cast for obstruction/occlusion detection.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(4), Tooltip(\"Ray casting mode for obstruction/occlusion detection.\")")
    AudioSystemSoundObstructionType ObstructionType = AudioSystemSoundObstructionType::SingleRay;

    /// <summary>
    /// Physics layer mask used when casting occlusion rays.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(5), Tooltip(\"Physics layer for occlusion rays.\")")
    uint8 OcclusionCollisionLayer = 0;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Resolves the proxy, caches trigger IDs, and optionally starts loading.
    /// </summary>
    void OnEnable() override;

  protected:
    /// <summary>
    /// Per-frame update: submits transform-delta obstruction/occlusion rays
    /// while the trigger is in the Playing state.
    /// </summary>
    void OnUpdate() override;

  public:
    /// <summary>
    /// Stops any active playback and unloads trigger data.
    /// </summary>
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// <summary>
    /// Loads trigger data (if not already loaded) then start playback.
    /// </summary>
    /// <param name="sync">When true, the play request blocks the calling thread until done.</param>
    API_FUNCTION()
    void Play(bool sync = false);

    /// <summary>
    /// Stops playback. Uses the stop trigger if configured, otherwise StopEvent.
    /// </summary>
    /// <param name="sync">When true, the stop request blocks the calling thread until done.</param>
    API_FUNCTION()
    void Stop(bool sync = false);

    // ========================================================================
    //  State queries
    // ========================================================================

    /// <returns>The current trigger lifecycle state.</returns>
    API_FUNCTION()
    AudioSystemTriggerState GetTriggerState() const;

    /// <returns>true if the trigger is loading.</returns>
    API_FUNCTION()
    bool IsLoading() const;

    /// <returns>true if the trigger is ready to play.</returns>
    API_FUNCTION()
    bool IsReady() const;

    /// <returns>true if the trigger is playing.</returns>
    API_FUNCTION()
    bool IsPlaying() const;

    /// <returns>true if the trigger is stopping.</returns>
    API_FUNCTION()
    bool IsStopping() const;

    /// <returns>true if the trigger is stopped.</returns>
    API_FUNCTION()
    bool IsStopped() const;

    /// <returns>The current occlusion value.</returns>
    API_FUNCTION()
    float GetOcclusion() const;

    /// <returns>The current obstruction value.</returns>
    API_FUNCTION()
    float GetObstruction() const;

  private:
    // ========================================================================
    //  Helpers — state transitions
    // ========================================================================

    /// <summary>
    /// Send a LoadTriggerRequest; on callback transition to Ready (or Playing if
    /// PlayOnActivate).
    /// </summary>
    void RequestLoad(bool sync);

    /// <summary>
    /// Send an ActivateTriggerRequest; on callback transition to Playing.
    /// </summary>
    void RequestPlay(bool sync);

    /// <summary>
    /// Send a stop request (stop-trigger or StopEvent); on callback go to Stopping.
    /// </summary>
    void RequestStop(bool sync);

    /// <summary>
    /// Send an UnloadTriggerRequest; on callback transition to Invalid.
    /// </summary>
    void RequestUnload(bool sync);

    // ========================================================================
    //  Helpers — obstruction / occlusion
    // ========================================================================

    /// <summary>
    /// Update obstruction/occlusion each frame while Playing.
    /// </summary>
    void UpdateObstructionOcclusion();

    /// <summary>
    /// Cast rays and compute new target obstruction and occlusion values.
    /// </summary>
    void ComputeObstructionOcclusion(float& outObstruction, float& outOcclusion) const;

    /// <summary>
    /// Cast a single ray from start in direction for distance.
    /// </summary>
    /// <returns>true if the ray hit something on `layerMask`.</returns>
    bool CastRay(const Vector3& start, const Vector3& direction, float distance) const;

    // ========================================================================
    //  State
    // ========================================================================

    /// <summary>
    /// Resolved ID for PlayTriggerName. INVALID_AUDIO_SYSTEM_ID if not found.
    /// </summary>
    AudioSystemDataID _playTriggerId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// Resolved ID for StopTriggerName. INVALID_AUDIO_SYSTEM_ID if not defined.
    /// </summary>
    AudioSystemDataID _stopTriggerId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// Current lifecycle state of the trigger.
    /// </summary>
    AudioSystemTriggerState _state = AudioSystemTriggerState::Invalid;

    // -- Obstruction / occlusion smoothed values ----------------------------

    float _currentOcclusion   = 0.0f;
    float _targetOcclusion    = 0.0f;
    float _currentObstruction = 0.0f;
    float _targetObstruction  = 0.0f;
};
