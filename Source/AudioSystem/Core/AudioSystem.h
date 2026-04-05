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

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Types/StringView.h>
#include <Engine/Platform/ConditionVariable.h>
#include <Engine/Platform/CriticalSection.h>

#include "../ATL/AudioTranslationLayer.h"
#include "AudioSystemRequests.h"
#include "AudioWorld.h"

// ============================================================================
//  Forward declarations
// ============================================================================

class AudioThread;
class AudioMiddleware;

// ============================================================================
//  AudioSystem
//
//  Singleton that owns the audio translation layer (ATL), request queues,
//  and the dedicated audio thread. All public methods are safe to call from
//  the main (game) thread.
//
//  Lifecycle: RegisterMiddleware() -> Startup() -> (per-frame UpdateSound())
//             -> Shutdown() -> UnregisterMiddleware()
// ============================================================================

/// <summary>
/// Core singleton that manages the audio request pipeline.
/// </summary>
class AUDIOSYSTEM_API AudioSystem
{
  public:
    /// <summary>
    /// Listener ID reserved for the editor preview listener (override mode).
    /// </summary>
    static constexpr AudioSystemDataID EDITOR_LISTENER_ID = 1;

    // ========================================================================
    //  Singleton
    // ========================================================================

    /// <returns>The global AudioSystem instance. Creates it on first call.</returns>
    static AudioSystem* Get();

    /// <summary>
    /// Destroy the global singleton. Must be called after Shutdown().
    /// </summary>
    static void Destroy();

    // ========================================================================
    //  Lifecycle
    // ========================================================================

    /// <summary>
    /// Initialise the ATL, start the audio thread. RegisterMiddleware() must
    /// have been called before this.
    /// </summary>
    /// <returns>true on success.</returns>
    bool Startup();

    /// <summary>
    /// Drain remaining requests, stop the audio thread, and shut down the ATL.
    /// </summary>
    void Shutdown();

    /// <returns>true after a successful Startup() and before Shutdown().</returns>
    bool IsInitialized() const;

    // ========================================================================
    //  Middleware binding
    // ========================================================================

    /// <summary>
    /// Set the middleware backend. Must be called before Startup().
    /// </summary>
    void RegisterMiddleware(AudioMiddleware* middleware);

    /// <summary>
    /// Clear the middleware binding (after Shutdown()).
    /// </summary>
    void UnregisterMiddleware();

    // ========================================================================
    //  Request submission (thread-safe)
    // ========================================================================

    /// <summary>
    /// Enqueue a request for asynchronous processing on the audio thread.
    /// </summary>
    void SendRequest(AudioRequest&& request);

    /// <summary>
    /// Enqueue a batch of requests for asynchronous processing.
    /// </summary>
    void SendRequests(Array<AudioRequest>& requests);

    /// <summary>
    /// Submit a request and block the calling thread until it is processed.
    /// Returns true when the audio thread reports a successful request result.
    /// </summary>
    bool SendRequestSync(AudioRequest&& request);

    // ========================================================================
    //  Control registration (synchronous, delegates to ATL)
    //
    //  These methods register control descriptors into the ATL maps so that
    //  subsequent audio requests (LoadTrigger, SetRtpc, etc.) can find them.
    //  The AudioSystem takes ownership of the data pointer on success.
    // ========================================================================

    bool RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data);
    bool RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data);
    bool RegisterSwitch(AudioSystemDataID id, const StringView& name, AudioSystemSwitchData* data);
    bool RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data);
    bool RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data);
    bool RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data);

    // ========================================================================
    //  Control unregistration (synchronous, delegates to ATL)
    // ========================================================================

    bool UnregisterTrigger(AudioSystemDataID id);
    bool UnregisterRtpc(AudioSystemDataID id);
    bool UnregisterSwitch(AudioSystemDataID id);
    bool UnregisterSwitchState(AudioSystemDataID id);
    bool UnregisterEnvironment(AudioSystemDataID id);
    bool UnregisterSoundBank(AudioSystemDataID id);

    // ========================================================================
    //  Named control ID lookups (delegates to ATL)
    // ========================================================================

    AudioSystemDataID GetTriggerId(StringView name) const;
    AudioSystemDataID GetRtpcId(StringView name) const;
    AudioSystemDataID GetSwitchId(StringView name) const;
    AudioSystemDataID GetSwitchStateId(StringView switchName, StringView stateName) const;
    AudioSystemDataID GetEnvironmentId(StringView name) const;
    AudioSystemDataID GetBankId(StringView name) const;

    // ========================================================================
    //  Global audio controls
    // ========================================================================

    void  SetMasterVolume(float volume);
    float GetMasterVolume() const;
    void  SetMasterMute(bool mute);
    bool  GetMasterMute() const;
    void  SetMasterPaused(bool paused);
    void  SetListenerOverrideMode(bool enabled);
    void  SetListener(int32 index, Vector3 position, Vector3 forward, Vector3 up, Vector3 velocity);

    // ========================================================================
    //  Per-frame hook (called from AudioSystemPlugin on the main thread)
    // ========================================================================

    /// <summary>
    /// Swap request queues and wake the audio thread.
    /// </summary>
    void UpdateSound();

    // ========================================================================
    //  World module
    // ========================================================================

    /// <returns>The scene-level audio world module (environment list, default listener).</returns>
    AudioWorld& GetWorld();

    // ========================================================================
    //  Configuration
    // ========================================================================

    void LoadConfiguration(StringView file);

#if USE_EDITOR
    /// <summary>
    /// Forward a deploy-files request to the loaded middleware.
    /// Called from the editor build hook during game cooking.
    /// </summary>
    /// <param name="outputPath">Absolute path to the cooked output root folder.</param>
    /// <returns>true if the middleware deployed successfully, false on error or no middleware.</returns>
    bool DeployFiles(const StringView& outputPath);
#endif

    AudioSystem();
    ~AudioSystem();

  private:
    friend class AudioThread;
    friend class AudioTranslationLayer;

    // Non-copyable, non-movable.
    AudioSystem(const AudioSystem&)            = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // ========================================================================
    //  Internal — audio thread
    // ========================================================================

    /// <summary>
    /// Process pending requests on the audio thread.
    /// </summary>
    void UpdateInternal();

    void StartAudioThread();
    void StopAudioThread();

    // -- Helpers for UpdateInternal ------------------------------------------
    void ProcessPendingRequests();
    void ProcessBlockingRequests();

    // ========================================================================
    //  State
    // ========================================================================

    static AudioSystem* _instance;

    AudioThread*          _audioThread = nullptr;
    AudioTranslationLayer _atl;
    AudioWorld            _world;

    // Request queues: async, pending (swapped per-frame), and blocking (sync).
    Array<AudioRequest> _requestsQueue;
    Array<AudioRequest> _pendingRequestsQueue;
    Array<AudioRequest> _blockingRequestsQueue;

    CriticalSection _requestsMutex;
    CriticalSection _pendingRequestsMutex;
    CriticalSection _blockingRequestsMutex;

    // Callback queues: filled by the audio thread, drained on the main thread.
    Array<AudioRequest> _pendingCallbacksQueue;
    Array<AudioRequest> _blockingCallbacksQueue;

    CriticalSection _pendingCallbacksMutex;
    CriticalSection _blockingCallbacksMutex;

    // Signaling between the main thread and the audio thread.
    CriticalSection   _mainMutex;
    ConditionVariable _mainSignal;
    CriticalSection   _processingMutex;
    ConditionVariable _processingSignal;

    /// <summary>
    /// Flag set by the audio thread after processing a blocking request.
    /// Used together with _mainSignal to guard against spurious wakeups.
    /// </summary>
    bool _blockingDone = false;
    bool _blockingSuccess = false;

    bool _initialized          = false;
    bool _masterPaused         = false;
    bool _listenerOverrideMode = false;
};
