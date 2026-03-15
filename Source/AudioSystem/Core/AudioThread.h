#pragma once

#include <Engine/Threading/IRunnable.h>

// ============================================================================
//  AudioThread
//
//  A dedicated thread that drives the audio system's request processing loop.
//  Implements the Flax IRunnable interface and is owned by AudioSystem.
//
//  The run loop:
//    1. Wait on _processingSignal (released by AudioSystem once per frame).
//    2. Call AudioSystem::UpdateInternal().
//    3. Notify _mainSignal so blocking callers can unblock.
//    4. Repeat while _keepRunning is true.
// ============================================================================

class AudioSystem;
class Thread;

/// \brief Runnable that drives the audio request processing loop on a
///        dedicated thread.
class AUDIOSYSTEM_API AudioThread : public IRunnable
{
public:
    AudioThread() = default;

    /// Start the background thread.
    /// \return true on success; false if the thread could not be created.
    bool Start();

    /// Signal the thread to stop and block until it exits.
    void RequestStop();

    // -- IRunnable -----------------------------------------------------------
    String ToString() const override;
    int32 Run() override;
    void Stop() override;
    void Exit() override;
    void AfterWork(bool wasKilled) override;

private:
    friend class AudioSystem;

    AudioSystem* _audioSystem = nullptr;
    Thread* _thread = nullptr;
    volatile bool _keepRunning = true;
};
