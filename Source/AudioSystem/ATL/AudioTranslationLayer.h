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

#include <Engine/Core/Types/DateTime.h>
#include <Engine/Core/Types/StringView.h>

#include "../Core/AudioMiddleware.h"
#include "../Core/AudioSystemRequests.h"
#include "AudioTranslationLayerData.h"

/// <summary>
/// Bridges the high-level AudioSystem singleton and the low-level
/// AudioMiddleware backend.
///
/// Responsibilities:
///   - Maintain typed maps of all ATL control objects (entities, listeners,
///     triggers, events, RTPCs, switch-states, environments, banks).
///   - Translate typed AudioRequest structs into concrete AudioMiddleware calls.
///   - Provide O(1) ID lookups by hashed name for every control type.
///
/// Constraints:
///   - MUST NOT touch any Flax scene objects (Actors, Scripts, etc.).
///     All scene interaction is performed by the caller (AudioSystem).
///   - Thread-safety is guaranteed by the caller. AudioTranslationLayer
///     itself performs no locking.
/// </summary>
class AUDIOSYSTEM_API AudioTranslationLayer
{
  public:
    AudioTranslationLayer()  = default;
    ~AudioTranslationLayer() = default;

    // Non-copyable, non-movable — owned by AudioSystem as a single instance.
    AudioTranslationLayer(const AudioTranslationLayer&)            = delete;
    AudioTranslationLayer& operator=(const AudioTranslationLayer&) = delete;

    // ========================================================================
    //  Lifecycle
    // ========================================================================

    /// <summary>
    /// Initialise the ATL and create the global world entity via the middleware.
    /// Must be called after SetMiddleware().
    /// </summary>
    /// <returns>true on success; false if the middleware is null or reports failure.</returns>
    bool Startup();

    /// <summary>
    /// Destroy all ATL control objects, release all middleware data, and call
    /// Shutdown/Release on the middleware. Safe to call more than once.
    /// </summary>
    void Shutdown();

    /// <summary>
    /// Advance the ATL by one frame.
    ///
    /// Computes the delta time since the last Update call and forwards it to
    /// the middleware.
    /// </summary>
    void Update();

    // ========================================================================
    //  Middleware binding
    // ========================================================================

    /// <summary>
    /// Bind the middleware backend. Must be called before Startup().
    /// Passing nullptr is valid and will cause Startup() to fail gracefully.
    /// </summary>
    void SetMiddleware(AudioMiddleware* middleware);

    /// <returns>The currently bound middleware, or nullptr if none is set.</returns>
    AudioMiddleware* GetMiddleware() const
    {
        return _middleware;
    }

    // ========================================================================
    //  Request dispatch
    //
    //  Called from AudioSystem::UpdateInternal() on the audio thread.
    //  Dispatches a typed AudioRequest to the appropriate middleware method
    //  and updates the ATL maps accordingly.
    // ========================================================================

    /// <summary>
    /// Dispatch a typed audio request to the middleware.
    /// </summary>
    /// <param name="request">Typed request from the audio-request queue.</param>
    /// <param name="sync">true if the caller expects a synchronous result.</param>
    /// <returns>true if the request was handled successfully.</returns>
    bool ProcessRequest(AudioRequest&& request, bool sync);

    // ========================================================================
    //  Control registration
    //
    //  Called synchronously from the main thread to populate the ATL maps
    //  with control descriptors created by the middleware plugin.
    //  The ATL takes ownership of the data pointer after a successful call.
    // ========================================================================

    /// <summary>
    /// Register a trigger control. The ATL takes ownership of data.
    /// </summary>
    /// <returns>true if registration succeeded; false if `id` is invalid or already registered.</returns>
    bool RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data);

    /// <summary>
    /// Register an RTPC control. The ATL takes ownership of data.
    /// </summary>
    bool RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data);

    /// <summary>
    /// Register a switch control. The ATL takes ownership of data.
    /// </summary>
    bool RegisterSwitch(AudioSystemDataID id, const StringView& name, AudioSystemSwitchData* data);

    /// <summary>
    /// Register a switch-state control. The ATL takes ownership of data.
    /// </summary>
    bool RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data);

    /// <summary>
    /// Register an environment control. The ATL takes ownership of data.
    /// </summary>
    bool RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data);

    /// <summary>
    /// Register a sound bank control. The ATL takes ownership of data.
    /// </summary>
    bool RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data);

    // ========================================================================
    //  Control unregistration
    //
    //  Removes the control from the ATL map and calls the appropriate
    //  middleware Destroy*Data() method to free the data pointer.
    // ========================================================================

    /// <summary>
    /// Unregister a trigger and destroy its middleware data.
    /// </summary>
    bool UnregisterTrigger(AudioSystemDataID id);

    /// <summary>
    /// Unregister an RTPC and destroy its middleware data.
    /// </summary>
    bool UnregisterRtpc(AudioSystemDataID id);

    /// <summary>
    /// Unregister a switch and destroy its data.
    /// </summary>
    bool UnregisterSwitch(AudioSystemDataID id);

    /// <summary>
    /// Unregister a switch-state and destroy its middleware data.
    /// </summary>
    bool UnregisterSwitchState(AudioSystemDataID id);

    /// <summary>
    /// Unregister an environment and destroy its middleware data.
    /// </summary>
    bool UnregisterEnvironment(AudioSystemDataID id);

    /// <summary>
    /// Unregister a sound bank and destroy its middleware data.
    /// </summary>
    bool UnregisterSoundBank(AudioSystemDataID id);

    // ========================================================================
    //  Control ID lookup by hashed name
    //
    //  Names are hashed at asset-load time and stored as AudioSystemDataID.
    //  These methods perform a map lookup and return INVALID_AUDIO_SYSTEM_ID
    //  when the control is not yet registered.
    // ========================================================================

    /// <returns>The ID of the named trigger, or INVALID_AUDIO_SYSTEM_ID.</returns>
    AudioSystemDataID GetTriggerId(StringView name) const;

    /// <returns>The ID of the named RTPC, or INVALID_AUDIO_SYSTEM_ID.</returns>
    AudioSystemDataID GetRtpcId(StringView name) const;

    /// <returns>The ID of the named switch, or INVALID_AUDIO_SYSTEM_ID.</returns>
    AudioSystemDataID GetSwitchId(StringView name) const;

    /// <returns>The ID of the named switch-state, or INVALID_AUDIO_SYSTEM_ID.</returns>
    AudioSystemDataID GetSwitchStateId(StringView name) const;

    /// <returns>The ID of the named switch-state within the specified switch, or INVALID_AUDIO_SYSTEM_ID.</returns>
    AudioSystemDataID GetSwitchStateId(StringView switchName, StringView stateName) const;

    /// <returns>The ID of the named environment, or INVALID_AUDIO_SYSTEM_ID.</returns>
    AudioSystemDataID GetEnvironmentId(StringView name) const;

    /// <returns>The ID of the named sound bank, or INVALID_AUDIO_SYSTEM_ID.</returns>
    AudioSystemDataID GetBankId(StringView name) const;

  private:
    // ========================================================================
    //  Internal helpers
    // ========================================================================

    /// <summary>
    /// Destroy and free every object in every ATL map. Does not call any
    /// middleware teardown — use Shutdown() for the full teardown sequence.
    /// </summary>
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

    /// <summary>
    /// Monotonically increasing event instance ID counter.
    /// </summary>
    AudioSystemDataID _nextEventId = 1;

    ATLEntityMap      _entities;
    ATLListenerMap    _listeners;
    ATLTriggerMap     _triggers;
    ATLEventMap       _events;
    ATLRtpcMap        _rtpcs;
    ATLSwitchMap      _switches;
    ATLSwitchStateMap _switchStates;
    ATLEnvironmentMap _environments;
    ATLSoundBankMap   _banks;

    /// <summary>
    /// Reverse lookup maps: hashed name -> control ID.
    /// </summary>
    Dictionary<uint32, AudioSystemDataID> _triggerNameMap;
    Dictionary<uint32, AudioSystemDataID> _rtpcNameMap;
    Dictionary<uint32, AudioSystemDataID> _switchNameMap;
    Dictionary<uint32, AudioSystemDataID> _switchStateNameMap;
    Dictionary<uint32, AudioSystemDataID> _environmentNameMap;
    Dictionary<uint32, AudioSystemDataID> _bankNameMap;

    DateTime _lastUpdateTime;
};
