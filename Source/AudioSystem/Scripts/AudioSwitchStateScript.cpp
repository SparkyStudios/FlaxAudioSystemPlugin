#include <Engine/Core/Log.h>

#include "AudioSwitchStateScript.h"
#include "../Actors/AudioProxyActor.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

AudioSwitchStateScript::AudioSwitchStateScript(const SpawnParams& params)
    : AudioProxyDependentScript(params)
{
}

// ============================================================================
//  OnEnable
// ============================================================================

void AudioSwitchStateScript::OnEnable()
{
    // Resolve the sibling proxy (done by the base class).
    AudioProxyDependentScript::OnEnable();

    if (_proxy == nullptr)
        return;

    // Apply the initial switch state if one is configured.
    if (SwitchStateName.HasChars())
        SetState(SwitchStateName);
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioSwitchStateScript::OnUpdate()
{
    // Switch states are set on-demand — nothing to do per-frame.
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioSwitchStateScript::OnDisable()
{
    _currentStateName.Clear();

    AudioProxyDependentScript::OnDisable();
}

// ============================================================================
//  SetState
// ============================================================================

void AudioSwitchStateScript::SetState(const StringView& stateName, bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioSwitchStateScript] SetState: proxy is not available (component may be disabled).");
        return;
    }

    if (stateName.IsEmpty())
    {
        LOG(Warning, "[AudioSwitchStateScript] SetState: stateName is empty. Request ignored.");
        return;
    }

    const AudioSystemDataID stateId = AudioSystem::Get()->GetSwitchStateId(stateName);

    if (stateId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSwitchStateScript] SetState: switch state '{0}' could not be resolved.", String(stateName));
        return;
    }

    _currentStateName = stateName;

    AudioRequest req;
    req.Type = AudioRequestType::SetSwitchState;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = stateId;

    if (sync)
        AudioSystem::Get()->SendRequestSync(std::move(req));
    else
        AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  GetState
// ============================================================================

const String& AudioSwitchStateScript::GetState() const
{
    return _currentStateName;
}
