#include <Engine/Core/Log.h>

#include "AudioWorldModule.h"

// ============================================================================
//  Per-frame update
// ============================================================================

void AudioWorldModule::Update()
{
    // TODO (Phase 7): For each component in _environments, query overlap with
    //                 active ATL audio objects and push environment amounts via
    //                 the AudioTranslationLayer so that the middleware can apply
    //                 reverb / obstruction blending per sound.
}

// ============================================================================
//  Environment registration
// ============================================================================

void AudioWorldModule::AddEnvironment(const AudioSystemEnvironmentActor* comp)
{
    if (comp == nullptr)
    {
        LOG(Warning, "[AudioWorldModule] AddEnvironment: null component pointer — ignoring.");
        return;
    }

    if (_environments.Contains(comp))
    {
        LOG(Warning, "[AudioWorldModule] AddEnvironment: component is already registered — ignoring duplicate.");
        return;
    }

    _environments.Add(comp);
}

void AudioWorldModule::RemoveEnvironment(const AudioSystemEnvironmentActor* comp)
{
    const int32 index = _environments.Find(comp);

    if (index == -1)
    {
        LOG(Warning, "[AudioWorldModule] RemoveEnvironment: component was not registered — ignoring.");
        return;
    }

    _environments.RemoveAt(index);
}

const Array<const AudioSystemEnvironmentActor*>& AudioWorldModule::GetEnvironments() const
{
    return _environments;
}

// ============================================================================
//  Default listener
// ============================================================================

void AudioWorldModule::SetDefaultListener(const AudioListenerComponent* listener)
{
    _defaultListener = listener;
}

const AudioListenerComponent* AudioWorldModule::GetDefaultListener() const
{
    return _defaultListener;
}
