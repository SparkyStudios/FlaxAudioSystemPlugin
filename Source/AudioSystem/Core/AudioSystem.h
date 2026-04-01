#pragma once

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Types/StringView.h>
#include <Engine/Platform/CriticalSection.h>
#include <Engine/Platform/ConditionVariable.h>

#include "../ATL/AudioTranslationLayer.h"
#include "AudioSystemRequests.h"
#include "AudioWorldModule.h"

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

/// \brief Core singleton that manages the audio request pipeline.
class AUDIOSYSTEM_API AudioSystem
{
public:
    /// Listener ID reserved for the editor preview listener (override mode).
    static constexpr AudioSystemDataID EDITOR_LISTENER_ID = 1;

    // ========================================================================
    //  Singleton
    // ========================================================================

    /// \return The global AudioSystem instance. Creates it on first call.
    static AudioSystem* Get();

    /// Destroy the global singleton. Must be called after Shutdown().
    static void Destroy();

    // ========================================================================
    //  Lifecycle
    // ========================================================================

    /// Initialise the ATL, start the audio thread. RegisterMiddleware() must
    /// have been called before this.
    /// \return true on success.
    bool Startup();

    /// Drain remaining requests, stop the audio thread, and shut down the ATL.
    void Shutdown();

    /// \return true after a successful Startup() and before Shutdown().
    bool IsInitialized() const;

    // ========================================================================
    //  Middleware binding
    // ========================================================================

    /// Set the middleware backend. Must be called before Startup().
    void RegisterMiddleware(AudioMiddleware* middleware);

    /// Clear the middleware binding (after Shutdown()).
    void UnregisterMiddleware();

    // ========================================================================
    //  Request submission (thread-safe)
    // ========================================================================

    /// Enqueue a request for asynchronous processing on the audio thread.
    void SendRequest(AudioRequest&& request);

    /// Enqueue a batch of requests for asynchronous processing.
    void SendRequests(Array<AudioRequest>& requests);

    /// Submit a request and block the calling thread until it is processed.
    void SendRequestSync(AudioRequest&& request);

    // ========================================================================
    //  Control registration (synchronous, delegates to ATL)
    //
    //  These methods register control descriptors into the ATL maps so that
    //  subsequent audio requests (LoadTrigger, SetRtpc, etc.) can find them.
    //  The AudioSystem takes ownership of the data pointer on success.
    // ========================================================================

    bool RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data);
    bool RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data);
    bool RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data);
    bool RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data);
    bool RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data);

    // ========================================================================
    //  Control unregistration (synchronous, delegates to ATL)
    // ========================================================================

    bool UnregisterTrigger(AudioSystemDataID id);
    bool UnregisterRtpc(AudioSystemDataID id);
    bool UnregisterSwitchState(AudioSystemDataID id);
    bool UnregisterEnvironment(AudioSystemDataID id);
    bool UnregisterSoundBank(AudioSystemDataID id);

    // ========================================================================
    //  Named control ID lookups (delegates to ATL)
    // ========================================================================

    AudioSystemDataID GetTriggerId(StringView name) const;
    AudioSystemDataID GetRtpcId(StringView name) const;
    AudioSystemDataID GetSwitchStateId(StringView name) const;
    AudioSystemDataID GetEnvironmentId(StringView name) const;
    AudioSystemDataID GetBankId(StringView name) const;

    // ========================================================================
    //  Global audio controls
    // ========================================================================

    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    void SetMasterMute(bool mute);
    bool GetMasterMute() const;
    void SetMasterPaused(bool paused);
    void SetListenerOverrideMode(bool enabled);
    void SetListener(int32 index, Vector3 position, Vector3 forward, Vector3 up, Vector3 velocity);

    // ========================================================================
    //  Per-frame hook (called from AudioSystemPlugin on the main thread)
    // ========================================================================

    /// Swap request queues and wake the audio thread.
    void UpdateSound();

    // ========================================================================
    //  World module
    // ========================================================================

    /// \return The scene-level audio world module (environment list, default listener).
    AudioWorldModule& GetWorldModule();

    // ========================================================================
    //  Configuration
    // ========================================================================

    void LoadConfiguration(StringView file);

#if USE_EDITOR
    /// Forward a deploy-files request to the loaded middleware.
    /// Called from the editor build hook during game cooking.
    /// \param outputPath  Absolute path to the cooked output root folder.
    /// \return true if the middleware deployed successfully, false on error or no middleware.
    bool DeployFiles(const StringView& outputPath);
#endif

    AudioSystem();
    ~AudioSystem();

private:
    friend class AudioThread;
    friend class AudioTranslationLayer;

    // Non-copyable, non-movable.
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // ========================================================================
    //  Internal — audio thread
    // ========================================================================

    /// Process pending requests on the audio thread.
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
    AudioWorldModule      _worldModule;

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
    CriticalSection    _mainMutex;
    ConditionVariable  _mainSignal;
    CriticalSection    _processingMutex;
    ConditionVariable  _processingSignal;

    /// Flag set by the audio thread after processing a blocking request.
    /// Used together with _mainSignal to guard against spurious wakeups.
    bool _blockingDone = false;

    bool _initialized = false;
    bool _masterPaused = false;
    bool _listenerOverrideMode = false;
};
