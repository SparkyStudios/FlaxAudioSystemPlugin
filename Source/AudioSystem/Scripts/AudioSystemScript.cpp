#include <Engine/Core/Log.h>
#include <Engine/Level/Actor.h>

#include "AudioSystemScript.h"
#include "../Actors/AudioProxyActor.h"
#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioProxyDependentScript — OnEnable
// ============================================================================

void AudioProxyDependentScript::OnEnable()
{
    Actor* owner = GetActor();
    if (owner == nullptr)
    {
        LOG(Warning, "[AudioProxyDependentScript] OnEnable: owner Actor is null. Component will be disabled.");
        SetEnabled(false);
        return;
    }

    _proxy = Cast<AudioProxyActor>(owner);
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioProxyDependentScript] OnEnable: owner Actor '{0}' is not an AudioProxyActor. Attach this script to an AudioProxyActor actor.",
            owner->GetName());
        SetEnabled(false);
        return;
    }
}

// ============================================================================
//  AudioProxyDependentScript — OnDisable
// ============================================================================

void AudioProxyDependentScript::OnDisable()
{
    _proxy = nullptr;
}

// ============================================================================
//  AudioProxyDependentScript — GetEntityId
// ============================================================================

AudioSystemDataID AudioProxyDependentScript::GetEntityId() const
{
    if (_proxy == nullptr)
        return INVALID_AUDIO_SYSTEM_ID;

    return _proxy->GetEntityId();
}
