#include <Engine/Core/Log.h>

#include "AudioSystemEnvironmentComponent.h"
#include "../Core/AudioSystem.h"

// ============================================================================
//  Scripting type registration
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE_NO_SPAWN(AudioSystemEnvironmentComponent, AudioSystemComponent,
    "AudioSystem.AudioSystemEnvironmentComponent", nullptr, nullptr);

// ============================================================================
//  OnEnable
// ============================================================================

void AudioSystemEnvironmentComponent::OnEnable()
{
    if (!EnvironmentName.HasChars())
    {
        LOG(Warning, "[AudioSystemEnvironmentComponent] OnEnable: EnvironmentName is empty. No environment will be applied.");
        _environmentId = INVALID_AUDIO_SYSTEM_ID;
        return;
    }

    _environmentId = AudioSystem::Get()->GetEnvironmentId(EnvironmentName);

    if (_environmentId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSystemEnvironmentComponent] OnEnable: Environment '{0}' could not be resolved.", EnvironmentName);
    }

    AudioSystem::Get()->GetWorldModule().AddEnvironment(this);
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioSystemEnvironmentComponent::OnDisable()
{
    AudioSystem::Get()->GetWorldModule().RemoveEnvironment(this);
    _environmentId = INVALID_AUDIO_SYSTEM_ID;
}

// ============================================================================
//  GetEnvironmentId
// ============================================================================

AudioSystemDataID AudioSystemEnvironmentComponent::GetEnvironmentId() const
{
    return _environmentId;
}
