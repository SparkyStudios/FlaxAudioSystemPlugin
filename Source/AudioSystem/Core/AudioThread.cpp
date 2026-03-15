#include <Engine/Core/Log.h>
#include <Engine/Platform/Thread.h>

#include "AudioThread.h"
#include "AudioSystem.h"

// ============================================================================
//  Constants
// ============================================================================

static const String AUDIO_THREAD_NAME = TEXT("AudioSystem");

// ============================================================================
//  Lifecycle
// ============================================================================

bool AudioThread::Start()
{
    if (_thread != nullptr)
    {
        LOG(Warning, "[AudioThread] Start called but thread is already running.");
        return true;
    }

    _keepRunning = true;
    _thread = Thread::Create(this, AUDIO_THREAD_NAME, ThreadPriority::AboveNormal);

    if (_thread == nullptr)
    {
        LOG(Error, "[AudioThread] Failed to create the audio thread.");
        return false;
    }

    LOG(Info, "[AudioThread] Audio thread started.");
    return true;
}

void AudioThread::RequestStop()
{
    if (_thread == nullptr)
        return;

    _keepRunning = false;

    // Wake the thread so it can observe _keepRunning == false and exit.
    if (_audioSystem != nullptr)
    {
        _audioSystem->_processingSignal.NotifyOne();
    }

    _thread->Join();
    Delete(_thread);
    _thread = nullptr;

    LOG(Info, "[AudioThread] Audio thread stopped.");
}

// ============================================================================
//  IRunnable
// ============================================================================

String AudioThread::ToString() const
{
    return AUDIO_THREAD_NAME;
}

int32 AudioThread::Run()
{
    if (_audioSystem == nullptr)
    {
        LOG(Error, "[AudioThread] Run called with null AudioSystem pointer.");
        return 1;
    }

    while (_keepRunning)
    {
        // Wait until the main thread signals that new requests are available.
        {
            _audioSystem->_processingMutex.Lock();
            _audioSystem->_processingSignal.Wait(_audioSystem->_processingMutex);
            _audioSystem->_processingMutex.Unlock();
        }

        if (!_keepRunning)
            break;

        _audioSystem->UpdateInternal();

        // Signal the main thread that blocking requests have been processed.
        _audioSystem->_mainSignal.NotifyOne();
    }

    return 0;
}

void AudioThread::Stop()
{
    _keepRunning = false;
}

void AudioThread::Exit()
{
}

void AudioThread::AfterWork(bool wasKilled)
{
}
