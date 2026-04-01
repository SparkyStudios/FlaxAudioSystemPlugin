#include <Engine/Core/Log.h>
#include <Engine/Threading/Threading.h>

#include "AudioSystem.h"
#include "AudioThread.h"

// ============================================================================
//  Singleton
// ============================================================================

AudioSystem* AudioSystem::_instance = nullptr;

AudioSystem* AudioSystem::Get()
{
    if (_instance == nullptr)
    {
        _instance = New<AudioSystem>();
    }
    return _instance;
}

void AudioSystem::Destroy()
{
    if (_instance != nullptr)
    {
        Delete(_instance);
        _instance = nullptr;
    }
}

// ============================================================================
//  Construction / Destruction
// ============================================================================

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem()
{
    if (_initialized)
    {
        Shutdown();
    }
}

// ============================================================================
//  Lifecycle
// ============================================================================

bool AudioSystem::Startup()
{
    if (_initialized)
    {
        LOG(Warning, "[AudioSystem] Startup called but system is already initialized.");
        return true;
    }

    LOG(Info, "[AudioSystem] Starting up...");

    if (!_atl.Startup())
    {
        LOG(Error, "[AudioSystem] ATL startup failed.");
        return false;
    }

    StartAudioThread();
    _initialized = true;

    LOG(Info, "[AudioSystem] Startup complete.");
    return true;
}

void AudioSystem::Shutdown()
{
    if (!_initialized)
    {
        LOG(Warning, "[AudioSystem] Shutdown called but system is not initialized.");
        return;
    }

    LOG(Info, "[AudioSystem] Shutting down...");

    _initialized = false;
    StopAudioThread();
    _atl.Shutdown();

    // Discard any requests left in the queues.
    {
        ScopeLock lock(_requestsMutex);
        _requestsQueue.Clear();
    }
    {
        ScopeLock lock(_pendingRequestsMutex);
        _pendingRequestsQueue.Clear();
    }
    {
        ScopeLock lock(_blockingRequestsMutex);
        _blockingRequestsQueue.Clear();
    }
    {
        ScopeLock lock(_pendingCallbacksMutex);
        _pendingCallbacksQueue.Clear();
    }
    {
        ScopeLock lock(_blockingCallbacksMutex);
        _blockingCallbacksQueue.Clear();
    }

    LOG(Info, "[AudioSystem] Shutdown complete.");
}

bool AudioSystem::IsInitialized() const
{
    return _initialized;
}

// ============================================================================
//  Middleware binding
// ============================================================================

void AudioSystem::RegisterMiddleware(AudioMiddleware* middleware)
{
    if (middleware == nullptr)
    {
        LOG(Warning, "[AudioSystem] RegisterMiddleware called with null pointer.");
        return;
    }

    _atl.SetMiddleware(middleware);
    LOG(Info, "[AudioSystem] Middleware registered.");
}

void AudioSystem::UnregisterMiddleware()
{
    _atl.SetMiddleware(nullptr);
    LOG(Info, "[AudioSystem] Middleware unregistered.");
}

// ============================================================================
//  Request submission
// ============================================================================

void AudioSystem::SendRequest(AudioRequest&& request)
{
    if (!_initialized)
    {
        LOG(Warning, "[AudioSystem] SendRequest: system not initialized, dropping request.");
        return;
    }

    ScopeLock lock(_requestsMutex);
    _requestsQueue.Add(MoveTemp(request));
}

void AudioSystem::SendRequests(Array<AudioRequest>& requests)
{
    if (!_initialized)
    {
        LOG(Warning, "[AudioSystem] SendRequests: system not initialized, dropping {0} request(s).", requests.Count());
        return;
    }

    ScopeLock lock(_requestsMutex);
    for (auto& req : requests)
    {
        _requestsQueue.Add(MoveTemp(req));
    }
}

void AudioSystem::SendRequestSync(AudioRequest&& request)
{
    if (!_initialized)
    {
        LOG(Warning, "[AudioSystem] SendRequestSync: system not initialized, dropping request.");
        return;
    }

    {
        ScopeLock lock(_mainMutex);
        _blockingDone = false;
    }

    {
        ScopeLock lock(_blockingRequestsMutex);
        _blockingRequestsQueue.Add(MoveTemp(request));
    }

    // Wake the audio thread to process the blocking request.
    _processingSignal.NotifyOne();

    // Block until the audio thread signals completion, guarding against spurious wakeups.
    {
        ScopeLock lock(_mainMutex);
        while (!_blockingDone)
        {
            _mainSignal.Wait(_mainMutex);
        }
    }
}

// ============================================================================
//  Control registration
// ============================================================================

bool AudioSystem::RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data)
{
    return _atl.RegisterTrigger(id, name, data);
}

bool AudioSystem::RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data)
{
    return _atl.RegisterRtpc(id, name, data);
}

bool AudioSystem::RegisterSwitchState(AudioSystemDataID id, const StringView& name, AudioSystemSwitchStateData* data)
{
    return _atl.RegisterSwitchState(id, name, data);
}

bool AudioSystem::RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data)
{
    return _atl.RegisterEnvironment(id, name, data);
}

bool AudioSystem::RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data)
{
    return _atl.RegisterSoundBank(id, name, data);
}

// ============================================================================
//  Control unregistration
// ============================================================================

bool AudioSystem::UnregisterTrigger(AudioSystemDataID id)
{
    return _atl.UnregisterTrigger(id);
}

bool AudioSystem::UnregisterRtpc(AudioSystemDataID id)
{
    return _atl.UnregisterRtpc(id);
}

bool AudioSystem::UnregisterSwitchState(AudioSystemDataID id)
{
    return _atl.UnregisterSwitchState(id);
}

bool AudioSystem::UnregisterEnvironment(AudioSystemDataID id)
{
    return _atl.UnregisterEnvironment(id);
}

bool AudioSystem::UnregisterSoundBank(AudioSystemDataID id)
{
    return _atl.UnregisterSoundBank(id);
}

// ============================================================================
//  Named control ID lookups
// ============================================================================

AudioSystemDataID AudioSystem::GetTriggerId(StringView name) const
{
    return _atl.GetTriggerId(name);
}

AudioSystemDataID AudioSystem::GetRtpcId(StringView name) const
{
    return _atl.GetRtpcId(name);
}

AudioSystemDataID AudioSystem::GetSwitchStateId(StringView name) const
{
    return _atl.GetSwitchStateId(name);
}

AudioSystemDataID AudioSystem::GetEnvironmentId(StringView name) const
{
    return _atl.GetEnvironmentId(name);
}

AudioSystemDataID AudioSystem::GetBankId(StringView name) const
{
    return _atl.GetBankId(name);
}

// ============================================================================
//  Global audio controls
// ============================================================================

void AudioSystem::SetMasterVolume(float volume)
{
    AudioMiddleware* mw = _atl.GetMiddleware();
    if (mw == nullptr)
    {
        LOG(Warning, "[AudioSystem] SetMasterVolume: no middleware registered.");
        return;
    }
    mw->OnMasterGainChange(volume);
}

float AudioSystem::GetMasterVolume() const
{
    const AudioMiddleware* mw = _atl.GetMiddleware();
    return mw != nullptr ? mw->GetMasterGain() : 0.0f;
}

void AudioSystem::SetMasterMute(bool mute)
{
    AudioMiddleware* mw = _atl.GetMiddleware();
    if (mw == nullptr)
    {
        LOG(Warning, "[AudioSystem] SetMasterMute: no middleware registered.");
        return;
    }
    mw->OnMuteChange(mute);
}

bool AudioSystem::GetMasterMute() const
{
    const AudioMiddleware* mw = _atl.GetMiddleware();
    return mw != nullptr ? mw->GetMute() : false;
}

void AudioSystem::SetMasterPaused(bool paused)
{
    _masterPaused = paused;
}

void AudioSystem::SetListenerOverrideMode(bool enabled)
{
    _listenerOverrideMode = enabled;
}

void AudioSystem::SetListener(int32 index, Vector3 position, Vector3 forward, Vector3 up, Vector3 velocity)
{
    // Build a transform request for the listener at the given index.
    const AudioSystemDataID listenerId = static_cast<AudioSystemDataID>(index + 1);
    AudioRequest req;
    req.Type = AudioRequestType::UpdateListenerTransform;
    req.EntityId = listenerId;
    req.Transform.Position = position;
    req.Transform.Forward = forward;
    req.Transform.Up = up;
    req.Transform.Velocity = velocity;
    SendRequest(MoveTemp(req));
}

// ============================================================================
//  Per-frame update (main thread)
// ============================================================================

void AudioSystem::UpdateSound()
{
    if (!_initialized)
        return;

    // 1. Swap the public request queue into the pending queue for the audio thread.
    {
        ScopeLock lock(_requestsMutex);
        ScopeLock lockPending(_pendingRequestsMutex);
        _pendingRequestsQueue.Add(_requestsQueue);
        _requestsQueue.Clear();
    }

    // 2. Wake the audio thread.
    _processingSignal.NotifyOne();

    // 3. Drain callback queues on the main thread.
    {
        Array<AudioRequest> pendingCallbacks;
        {
            ScopeLock lock(_pendingCallbacksMutex);
            pendingCallbacks = MoveTemp(_pendingCallbacksQueue);
        }
        for (auto& cb : pendingCallbacks)
        {
            if (cb.Callback.IsBinded())
                cb.Callback(cb.Success);
        }
    }
    {
        Array<AudioRequest> blockingCallbacks;
        {
            ScopeLock lock(_blockingCallbacksMutex);
            blockingCallbacks = MoveTemp(_blockingCallbacksQueue);
        }
        for (auto& cb : blockingCallbacks)
        {
            if (cb.Callback.IsBinded())
                cb.Callback(cb.Success);
        }
    }

    // 4. Update scene-level audio world state (environments, listener) after ATL.
    _worldModule.Update();
}

// ============================================================================
//  World module
// ============================================================================

AudioWorldModule& AudioSystem::GetWorldModule()
{
    return _worldModule;
}

// ============================================================================
//  Configuration
// ============================================================================

void AudioSystem::LoadConfiguration(StringView file)
{
    AudioMiddleware* mw = _atl.GetMiddleware();
    if (mw == nullptr)
    {
        LOG(Error, "[AudioSystem] LoadConfiguration failed: no middleware registered.");
        return;
    }

    if (!mw->LoadConfiguration(file))
    {
        LOG(Error, "[AudioSystem] Middleware rejected configuration file.");
    }
}

// ============================================================================
//  Audio thread management
// ============================================================================

void AudioSystem::StartAudioThread()
{
    if (_audioThread != nullptr)
        return;

    _audioThread = New<AudioThread>();
    _audioThread->_audioSystem = this;

    if (!_audioThread->Start())
    {
        LOG(Error, "[AudioSystem] Failed to start audio thread.");
        Delete(_audioThread);
        _audioThread = nullptr;
    }
}

void AudioSystem::StopAudioThread()
{
    if (_audioThread == nullptr)
        return;

    _audioThread->RequestStop();
    Delete(_audioThread);
    _audioThread = nullptr;
}

// ============================================================================
//  UpdateInternal — runs on the audio thread
// ============================================================================

void AudioSystem::UpdateInternal()
{
    ProcessPendingRequests();
    ProcessBlockingRequests();
    _atl.Update();
}

void AudioSystem::ProcessPendingRequests()
{
    // Snapshot pending requests under lock, then process without holding it.
    Array<AudioRequest> batch;
    {
        ScopeLock lock(_pendingRequestsMutex);
        batch = MoveTemp(_pendingRequestsQueue);
    }

    for (auto& req : batch)
    {
        // Extract the callback before processing so it survives the move.
        Function<void(bool)> callback = MoveTemp(req.Callback);
        const bool success = _atl.ProcessRequest(MoveTemp(req), false);

        if (callback.IsBinded())
        {
            // Queue the callback for main-thread invocation instead of calling it here.
            AudioRequest cbEntry;
            cbEntry.Type = AudioRequestType::Shutdown; // Type is irrelevant for callbacks.
            cbEntry.Success = success;
            cbEntry.Callback = MoveTemp(callback);

            ScopeLock lock(_pendingCallbacksMutex);
            _pendingCallbacksQueue.Add(MoveTemp(cbEntry));
        }
    }
}

void AudioSystem::ProcessBlockingRequests()
{
    Array<AudioRequest> batch;
    {
        ScopeLock lock(_blockingRequestsMutex);
        batch = MoveTemp(_blockingRequestsQueue);
    }

    if (batch.IsEmpty())
        return;

    for (auto& req : batch)
    {
        Function<void(bool)> callback = MoveTemp(req.Callback);
        const bool success = _atl.ProcessRequest(MoveTemp(req), true);

        if (callback.IsBinded())
        {
            // Queue the callback for main-thread invocation instead of calling it here.
            AudioRequest cbEntry;
            cbEntry.Type = AudioRequestType::Shutdown; // Type is irrelevant for callbacks.
            cbEntry.Success = success;
            cbEntry.Callback = MoveTemp(callback);

            ScopeLock lock(_blockingCallbacksMutex);
            _blockingCallbacksQueue.Add(MoveTemp(cbEntry));
        }
    }

    // Signal the main thread that blocking processing is complete.
    {
        ScopeLock lock(_mainMutex);
        _blockingDone = true;
    }
    _mainSignal.NotifyOne();
}
