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
    ScopeLock lock(_requestsMutex);
    _requestsQueue.Add(MoveTemp(request));
}

void AudioSystem::SendRequests(Array<AudioRequest>& requests)
{
    ScopeLock lock(_requestsMutex);
    for (auto& req : requests)
    {
        _requestsQueue.Add(MoveTemp(req));
    }
}

void AudioSystem::SendRequestSync(AudioRequest&& request)
{
    {
        ScopeLock lock(_blockingRequestsMutex);
        _blockingRequestsQueue.Add(MoveTemp(request));
    }

    // Wake the audio thread to process the blocking request.
    _processingSignal.NotifyOne();

    // Block until the audio thread signals completion.
    _mainMutex.Lock();
    _mainSignal.Wait(_mainMutex);
    _mainMutex.Unlock();
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
    UpdateListenerTransformRequest req;
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

    // 3. Callbacks are invoked on the audio thread during processing.
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
        // Extract the callback before moving the request into ProcessRequest.
        Function<void(bool)> callback = MoveTemp(req.Callback);
        const bool success = _atl.ProcessRequest(MoveTemp(req), false);

        if (callback.IsBinded())
        {
            callback(success);
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

    for (auto& req : batch)
    {
        Function<void(bool)> callback = MoveTemp(req.Callback);
        const bool success = _atl.ProcessRequest(MoveTemp(req), true);

        if (callback.IsBinded())
        {
            callback(success);
        }
    }
}
