#include <Engine/Core/Log.h>

#include "AudioWorldModule.h"
#include "../Components/AudioProxyComponent.h"

// ============================================================================
//  Per-frame update
// ============================================================================

void AudioWorldModule::Update()
{
    // For each active proxy, query each environment for the send amount
    // and push it to the proxy (which dispatches to the audio system if changed).
    for (AudioProxyComponent* proxy : _proxies)
    {
        if (proxy == nullptr)
            continue;

        for (const AudioSystemEnvironmentActor* env : _environments)
        {
            if (env == nullptr)
                continue;

            const AudioSystemDataID envId = env->GetEnvironmentId();
            if (envId == INVALID_AUDIO_SYSTEM_ID)
                continue;

            const float amount = env->GetEnvironmentAmount(proxy);
            proxy->SetEnvironmentAmount(envId, amount);
        }
    }
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

// ============================================================================
//  Proxy registration
// ============================================================================

void AudioWorldModule::AddProxy(AudioProxyComponent* proxy)
{
    if (proxy == nullptr)
    {
        LOG(Warning, "[AudioWorldModule] AddProxy: null proxy pointer — ignoring.");
        return;
    }

    if (_proxies.Contains(proxy))
        return;

    _proxies.Add(proxy);
}

void AudioWorldModule::RemoveProxy(AudioProxyComponent* proxy)
{
    const int32 index = _proxies.Find(proxy);
    if (index != -1)
        _proxies.RemoveAt(index);
}
