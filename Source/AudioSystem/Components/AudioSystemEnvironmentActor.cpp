#include <Engine/Core/Log.h>

#include "AudioSystemEnvironmentActor.h"
#include "../Core/AudioSystem.h"

// ============================================================================
//  OnBeginPlay
// ============================================================================

void AudioSystemEnvironmentActor::OnBeginPlay()
{
    Actor::OnBeginPlay();

    if (!EnvironmentName.HasChars())
    {
        LOG(Warning, "[AudioSystemEnvironmentActor] OnBeginPlay: EnvironmentName is empty. No environment will be applied.");
        _environmentId = INVALID_AUDIO_SYSTEM_ID;
        return;
    }

    _environmentId = AudioSystem::Get()->GetEnvironmentId(EnvironmentName);

    if (_environmentId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSystemEnvironmentActor] OnBeginPlay: Environment '{0}' could not be resolved.", EnvironmentName);
    }

    AudioSystem::Get()->GetWorldModule().AddEnvironment(this);
}

// ============================================================================
//  OnEndPlay
// ============================================================================

void AudioSystemEnvironmentActor::OnEndPlay()
{
    AudioSystem::Get()->GetWorldModule().RemoveEnvironment(this);
    _environmentId = INVALID_AUDIO_SYSTEM_ID;

    Actor::OnEndPlay();
}

// ============================================================================
//  GetEnvironmentId
// ============================================================================

AudioSystemDataID AudioSystemEnvironmentActor::GetEnvironmentId() const
{
    return _environmentId;
}

// ============================================================================
//  Debug draw (editor only)
// ============================================================================

#if USE_EDITOR

void AudioSystemEnvironmentActor::OnDebugDraw()
{
    Actor::OnDebugDraw();
}

void AudioSystemEnvironmentActor::OnDebugDrawSelected()
{
    Actor::OnDebugDrawSelected();
}

#endif
