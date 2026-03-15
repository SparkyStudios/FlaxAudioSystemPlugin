#include <Engine/Core/Log.h>
#include <Engine/Level/Actor.h>

#include "AudioSystemComponent.h"
#include "AudioProxyComponent.h"
#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioSystemComponent — scripting type registration
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE_NO_SPAWN(AudioSystemComponent, Script,
    "AudioSystem.AudioSystemComponent", nullptr, nullptr);

// ============================================================================
//  AudioSystemProxyDependentComponent — scripting type registration
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE_NO_SPAWN(AudioSystemProxyDependentComponent, AudioSystemComponent,
    "AudioSystem.AudioSystemProxyDependentComponent", nullptr, nullptr);

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

    _proxy = owner->GetScript<AudioProxyComponent>();
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioSystemProxyDependentComponent] OnEnable: no sibling AudioProxyComponent found on Actor '{0}'. Component will be disabled.",
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
