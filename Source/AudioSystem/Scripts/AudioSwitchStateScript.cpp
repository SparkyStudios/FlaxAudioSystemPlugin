#include <Engine/Core/Log.h>

#include "AudioSwitchStateComponent.h"
#include "AudioProxyComponent.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

AudioSwitchStateComponent::AudioSwitchStateComponent(const SpawnParams& params)
    : AudioSystemProxyDependentComponent(params)
{
}

// ============================================================================
//  OnEnable
// ============================================================================

void AudioSwitchStateComponent::OnEnable()
{
    // Resolve the sibling proxy (done by the base class).
    AudioSystemProxyDependentComponent::OnEnable();

    if (_proxy == nullptr)
        return;

    // Apply the initial switch state if one is configured.
    if (SwitchStateName.HasChars())
        SetState(SwitchStateName);
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioSwitchStateComponent::OnUpdate()
{
    // Switch states are set on-demand — nothing to do per-frame.
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioSwitchStateComponent::OnDisable()
{
    _currentStateName.Clear();

    AudioSystemProxyDependentComponent::OnDisable();
}

// ============================================================================
//  SetState
// ============================================================================

void AudioSwitchStateComponent::SetState(const StringView& stateName, bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioSwitchStateComponent] SetState: proxy is not available (component may be disabled).");
        return;
    }

    if (stateName.IsEmpty())
    {
        LOG(Warning, "[AudioSwitchStateComponent] SetState: stateName is empty. Request ignored.");
        return;
    }

    const AudioSystemDataID stateId = AudioSystem::Get()->GetSwitchStateId(stateName);

    if (stateId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSwitchStateComponent] SetState: switch state '{0}' could not be resolved.", String(stateName));
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

const String& AudioSwitchStateComponent::GetState() const
{
    return _currentStateName;
}
