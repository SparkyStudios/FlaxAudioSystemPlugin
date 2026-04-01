#include <Engine/Core/Log.h>
#include <Engine/Level/Actor.h>

#include "AudioSystemComponent.h"
#include "AudioProxyComponent.h"
#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioSystemProxyDependentComponent — OnEnable
// ============================================================================

void AudioSystemProxyDependentComponent::OnEnable()
{
    Actor* owner = GetActor();
    if (owner == nullptr)
    {
        LOG(Warning, "[AudioSystemProxyDependentComponent] OnEnable: owner Actor is null. Component will be disabled.");
        SetEnabled(false);
        return;
    }

    _proxy = Cast<AudioProxyComponent>(owner);
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioSystemProxyDependentComponent] OnEnable: owner Actor '{0}' is not an AudioProxyComponent. Attach this script to an AudioProxyComponent actor.",
            owner->GetName());
        SetEnabled(false);
        return;
    }
}

// ============================================================================
//  AudioSystemProxyDependentComponent — OnDisable
// ============================================================================

void AudioSystemProxyDependentComponent::OnDisable()
{
    _proxy = nullptr;
}

// ============================================================================
//  AudioSystemProxyDependentComponent — GetEntityId
// ============================================================================

AudioSystemDataID AudioSystemProxyDependentComponent::GetEntityId() const
{
    if (_proxy == nullptr)
        return INVALID_AUDIO_SYSTEM_ID;

    return _proxy->GetEntityId();
}
