#include <Engine/Core/Log.h>

#include "AudioRtpcComponent.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

// ============================================================================
//  Scripting type registration
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE(AudioRtpcComponent, AudioSystemProxyDependentComponent,
    "AudioSystem.AudioRtpcComponent", nullptr, nullptr);

// ============================================================================
//  OnEnable
// ============================================================================

void AudioRtpcComponent::OnEnable()
{
    // Resolve the sibling proxy (done by the base class).
    AudioSystemProxyDependentComponent::OnEnable();

    if (_proxy == nullptr)
        return;

    // Resolve the RTPC ID from its name.
    if (!RtpcName.HasChars())
    {
        LOG(Warning, "[AudioRtpcComponent] OnEnable: RtpcName is empty. No RTPC will be driven.");
        _rtpcId = INVALID_AUDIO_SYSTEM_ID;
        return;
    }

    _rtpcId = AudioSystem::Get()->GetRtpcId(RtpcName);

    if (_rtpcId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioRtpcComponent] OnEnable: RTPC '{0}' could not be resolved.", RtpcName);
        return;
    }

    // Push the initial value.
    SetValue(InitialValue);
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioRtpcComponent::OnUpdate()
{
    // RTPC values are set on-demand — nothing to do per-frame.
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioRtpcComponent::OnDisable()
{
    if (_proxy != nullptr && _rtpcId != INVALID_AUDIO_SYSTEM_ID)
        ResetValue(false);

    _rtpcId       = INVALID_AUDIO_SYSTEM_ID;
    _currentValue = 0.0f;

    AudioSystemProxyDependentComponent::OnDisable();
}

// ============================================================================
//  SetValue
// ============================================================================

void AudioRtpcComponent::SetValue(float value, bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioRtpcComponent] SetValue: proxy is not available (component may be disabled).");
        return;
    }

    if (_rtpcId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioRtpcComponent] SetValue: RTPC '{0}' is not resolved.", RtpcName);
        return;
    }

    _currentValue = value;

    AudioRequest req;
    req.Type = AudioRequestType::SetRtpcValue;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = _rtpcId;
    req.Value    = value;

    if (sync)
        AudioSystem::Get()->SendRequestSync(std::move(req));
    else
        AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  ResetValue
// ============================================================================

void AudioRtpcComponent::ResetValue(bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioRtpcComponent] ResetValue: proxy is not available (component may be disabled).");
        return;
    }

    if (_rtpcId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioRtpcComponent] ResetValue: RTPC '{0}' is not resolved.", RtpcName);
        return;
    }

    AudioRequest req;
    req.Type = AudioRequestType::ResetRtpcValue;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = _rtpcId;

    if (sync)
        AudioSystem::Get()->SendRequestSync(std::move(req));
    else
        AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  GetValue
// ============================================================================

float AudioRtpcComponent::GetValue() const
{
    return _currentValue;
}
