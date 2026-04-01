#pragma once

#include <Engine/Core/Types/String.h>

#include "AudioSystemComponent.h"
#include "../Core/AudioSystemData.h"

// ============================================================================
//  Constants
// ============================================================================

/// Maximum number of rays used for multi-ray obstruction/occlusion sampling.
constexpr int32 k_MaxOcclusionRaysCount = 32;

/// Smoothing factor for lerping obstruction and occlusion toward target values.
constexpr float k_ObstructionSmoothingFactor = 0.1f;

/// Number of rays cast in multi-ray mode (centre + 4 perpendicular offsets).
constexpr int32 k_MultiRayCount = 5;

/// Angular offset radius (in world units) used for multi-ray side rays.
constexpr float k_MultiRayOffsetRadius = 0.25f;

// ============================================================================
//  AudioTriggerComponent
//
//  Fires named audio triggers on a sibling AudioProxyComponent's entity.
//  Manages a complete trigger lifecycle:
//
//    Invalid → Loading → Ready → Starting → Playing → Stopping → Stopped
//           → Unloading → Invalid
//
//  The state machine is driven by async callbacks from the audio thread.
//  Obstruction/occlusion rays are cast every frame while in the Playing state.
// ============================================================================

/// \brief Plays and stops named audio triggers on an Actor with obstruction/occlusion support.
///
/// Requires a sibling AudioProxyComponent on the same Actor.
/// The component manages loading, playing, and stopping trigger data, and
/// optionally performs physics ray casts each frame to compute the
/// obstruction and occlusion values sent to the audio middleware.
API_CLASS() class AUDIOSYSTEM_API AudioTriggerComponent : public AudioSystemProxyDependentComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioTriggerComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Name of the play trigger. Resolved to an ID in OnEnable.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Name of the play trigger.\")")
    String PlayTriggerName;

    /// Name of the optional stop trigger. When empty, StopEvent is used instead.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Name of the stop trigger. If empty, StopEvent is used.\")")
    String StopTriggerName;

    /// If true, trigger data is loaded in OnEnable so it is ready to play immediately.
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Pre-load trigger data on component enable.\")")
    bool LoadOnInit = true;

    /// If true, Play() is called automatically once loading completes.
    API_FIELD(Attributes="EditorOrder(3), Tooltip(\"Auto-play when loading completes.\")")
    bool PlayOnActivate = false;

    /// Controls how many rays are cast for obstruction/occlusion detection.
    API_FIELD(Attributes="EditorOrder(4), Tooltip(\"Ray casting mode for obstruction/occlusion detection.\")")
    AudioSystemSoundObstructionType ObstructionType = AudioSystemSoundObstructionType::SingleRay;

    /// Physics layer mask used when casting occlusion rays.
    API_FIELD(Attributes="EditorOrder(5), Tooltip(\"Physics layer for occlusion rays.\")")
    uint8 OcclusionCollisionLayer = 0;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// Resolves the proxy, caches trigger IDs, and optionally starts loading.
    void OnEnable() override;

protected:
    /// Per-frame update: submits transform-delta obstruction/occlusion rays
    /// while the trigger is in the Playing state.
    void OnUpdate() override;

public:
    /// Stops any active playback and unloads trigger data.
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// Load trigger data (if not already loaded) then start playback.
    /// \param sync  When true, the load request blocks the calling thread.
    API_FUNCTION() void Play(bool sync = false);

    /// Stop playback. Uses the stop trigger if configured, otherwise StopEvent.
    /// \param sync  When true, the stop request blocks the calling thread.
    API_FUNCTION() void Stop(bool sync = false);

    // ========================================================================
    //  State queries
    // ========================================================================

    /// \return The current trigger lifecycle state.
    API_FUNCTION() AudioSystemTriggerState GetTriggerState() const;

    API_FUNCTION() bool IsLoading()  const;
    API_FUNCTION() bool IsReady()    const;
    API_FUNCTION() bool IsPlaying()  const;
    API_FUNCTION() bool IsStopping() const;
    API_FUNCTION() bool IsStopped()  const;

private:
    // ========================================================================
    //  Helpers — state transitions
    // ========================================================================

    /// Send a LoadTriggerRequest; on callback transition to Ready (or Playing if
    /// PlayOnActivate).
    void RequestLoad(bool sync);

    /// Send an ActivateTriggerRequest; on callback transition to Playing.
    void RequestPlay(bool sync);

    /// Send a stop request (stop-trigger or StopEvent); on callback go to Stopping.
    void RequestStop(bool sync);

    /// Send an UnloadTriggerRequest; on callback transition to Invalid.
    void RequestUnload(bool sync);

    // ========================================================================
    //  Helpers — obstruction / occlusion
    // ========================================================================

    /// Update obstruction/occlusion each frame while Playing.
    void UpdateObstructionOcclusion();

    /// Cast rays and compute new target obstruction and occlusion values.
    void ComputeObstructionOcclusion(float& outObstruction, float& outOcclusion) const;

    /// Cast a single ray from \p start in \p direction for \p distance.
    /// \return true if the ray hit something on \p layerMask.
    bool CastRay(const Vector3& start, const Vector3& direction, float distance) const;

    // ========================================================================
    //  State
    // ========================================================================

    /// Resolved ID for PlayTriggerName. INVALID_AUDIO_SYSTEM_ID if not found.
    AudioSystemDataID _playTriggerId = INVALID_AUDIO_SYSTEM_ID;

    /// Resolved ID for StopTriggerName. INVALID_AUDIO_SYSTEM_ID if not defined.
    AudioSystemDataID _stopTriggerId = INVALID_AUDIO_SYSTEM_ID;

    /// Current lifecycle state of the trigger.
    AudioSystemTriggerState _state = AudioSystemTriggerState::Invalid;

    // -- Obstruction / occlusion smoothed values ----------------------------

    float _currentOcclusion   = 0.0f;
    float _targetOcclusion    = 0.0f;
    float _currentObstruction = 0.0f;
    float _targetObstruction  = 0.0f;
};
