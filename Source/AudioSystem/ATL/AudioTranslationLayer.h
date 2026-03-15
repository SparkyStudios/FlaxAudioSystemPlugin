#pragma once

#include <Engine/Core/Types/DateTime.h>
#include <Engine/Core/Types/StringView.h>

#include "../Core/AudioMiddleware.h"
#include "../Core/AudioSystemRequests.h"
#include "AudioTranslationLayerData.h"

// ============================================================================
//  AudioTranslationLayer
//
//  Bridges the high-level AudioSystem singleton and the low-level
//  AudioMiddleware backend.
//
//  Responsibilities:
//    - Maintain typed maps of all ATL control objects (entities, listeners,
//      triggers, events, RTPCs, switch-states, environments, banks).
//    - Translate typed AudioRequest structs into concrete AudioMiddleware calls.
//    - Provide O(1) ID lookups by hashed name for every control type.
//
//  Constraints:
//    - MUST NOT touch any Flax scene objects (Actors, Scripts, etc.).
//      All scene interaction is performed by the caller (AudioSystem).
//    - Thread-safety is guaranteed by the caller. AudioTranslationLayer
//      itself performs no locking.
// ============================================================================

/// \brief Translates high-level audio requests into middleware calls.
class AUDIOSYSTEM_API AudioTranslationLayer
{
public:
    AudioTranslationLayer() = default;
    ~AudioTranslationLayer() = default;

    // Non-copyable, non-movable — owned by AudioSystem as a single instance.
    AudioTranslationLayer(const AudioTranslationLayer&) = delete;
    AudioTranslationLayer& operator=(const AudioTranslationLayer&) = delete;

    // ========================================================================
    //  Lifecycle
    // ========================================================================

    /// Initialise the ATL and create the global world entity via the middleware.
    /// Must be called after SetMiddleware().
    /// \return true on success; false if the middleware is null or reports failure.
    bool Startup();

    /// Destroy all ATL control objects, release all middleware data, and call
    /// Shutdown/Release on the middleware. Safe to call more than once.
    void Shutdown();

    /// Advance the ATL by one frame.
    /// Computes the delta time since the last Update call and forwards it to
    /// the middleware.
    void Update();

    // ========================================================================
    //  Middleware binding
    // ========================================================================

    /// Bind the middleware backend. Must be called before Startup().
    /// Passing nullptr is valid and will cause Startup() to fail gracefully.
    void SetMiddleware(AudioMiddleware* middleware);

    /// \return The currently bound middleware, or nullptr if none is set.
    AudioMiddleware* GetMiddleware() const { return _middleware; }

    // ========================================================================
    //  Request dispatch
    //
    //  Called from AudioSystem::UpdateInternal() on the audio thread.
    //  Dispatches a typed AudioRequest to the appropriate middleware method
    //  and updates the ATL maps accordingly.
    // ========================================================================

    /// Dispatch a typed audio request to the middleware.
    /// \param request  Typed request from the audio-request queue.
    /// \param sync     true if the caller expects a synchronous result.
    /// \return true if the request was handled successfully.
    bool ProcessRequest(AudioRequest&& request, bool sync);

    // ========================================================================
    //  Control ID lookup by hashed name
    //
    //  Names are hashed at asset-load time and stored as AudioSystemDataID.
    //  These methods perform a map lookup and return INVALID_AUDIO_SYSTEM_ID
    //  when the control is not yet registered.
    //
    //  NOTE: The maps will be populated in Phase 5 when the asset-loading
    //  pipeline and control registration path are implemented.
    // ========================================================================

    /// \return The ID of the named trigger, or INVALID_AUDIO_SYSTEM_ID.
    AudioSystemDataID GetTriggerId(StringView name) const;

    /// \return The ID of the named RTPC, or INVALID_AUDIO_SYSTEM_ID.
    AudioSystemDataID GetRtpcId(StringView name) const;

    /// \return The ID of the named switch-state, or INVALID_AUDIO_SYSTEM_ID.
    AudioSystemDataID GetSwitchStateId(StringView name) const;

    /// \return The ID of the named environment, or INVALID_AUDIO_SYSTEM_ID.
    AudioSystemDataID GetEnvironmentId(StringView name) const;

    /// \return The ID of the named sound bank, or INVALID_AUDIO_SYSTEM_ID.
    AudioSystemDataID GetBankId(StringView name) const;

private:
    // ========================================================================
    //  Internal helpers
    // ========================================================================

    /// Destroy and free every object in every ATL map. Does not call any
    /// middleware teardown — use Shutdown() for the full teardown sequence.
    void ClearAllMaps();

    // -- Request handlers (one per AudioRequestType) -------------------------
    bool HandleRegisterEntity(const AudioRequest& request);
    bool HandleUnregisterEntity(const AudioRequest& request);
    bool HandleUpdateEntityTransform(const AudioRequest& request);
    bool HandleRegisterListener(const AudioRequest& request);
    bool HandleUnregisterListener(const AudioRequest& request);
    bool HandleUpdateListenerTransform(const AudioRequest& request);
    bool HandleLoadTrigger(const AudioRequest& request);
    bool HandleActivateTrigger(const AudioRequest& request);
    bool HandleStopEvent(const AudioRequest& request);
    bool HandleStopAllEvents(const AudioRequest& request);
    bool HandleUnloadTrigger(const AudioRequest& request);
    bool HandleSetRtpcValue(const AudioRequest& request);
    bool HandleResetRtpcValue(const AudioRequest& request);
    bool HandleSetSwitchState(const AudioRequest& request);
    bool HandleSetObstructionOcclusion(const AudioRequest& request);
    bool HandleSetEnvironmentAmount(const AudioRequest& request);
    bool HandleLoadBank(const AudioRequest& request);
    bool HandleUnloadBank(const AudioRequest& request);

    // ========================================================================
    //  State
    // ========================================================================

    AudioMiddleware* _middleware = nullptr;

    ATLEntityMap      _entities;
    ATLListenerMap    _listeners;
    ATLTriggerMap     _triggers;
    ATLEventMap       _events;
    ATLRtpcMap        _rtpcs;
    ATLSwitchStateMap _switchStates;
    ATLEnvironmentMap _environments;
    ATLSoundBankMap   _banks;

    DateTime _lastUpdateTime;
};
