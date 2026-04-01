#include <Engine/Core/Log.h>

#include "AudioRtpcScript.h"
#include "../Actors/AudioProxyActor.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

AudioRtpcScript::AudioRtpcScript(const SpawnParams& params)
    : AudioProxyDependentScript(params)
{
}

// ============================================================================
//  OnEnable
// ============================================================================

void AudioRtpcScript::OnEnable()
{
    // Resolve the sibling proxy (done by the base class).
    AudioProxyDependentScript::OnEnable();

    if (_proxy == nullptr)
        return;

    // Resolve the RTPC ID from its name.
    if (!RtpcName.HasChars())
    {
        LOG(Warning, "[AudioRtpcScript] OnEnable: RtpcName is empty. No RTPC will be driven.");
        _rtpcId = INVALID_AUDIO_SYSTEM_ID;
        return;
    }

    _rtpcId = AudioSystem::Get()->GetRtpcId(RtpcName);

    if (_rtpcId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioRtpcScript] OnEnable: RTPC '{0}' could not be resolved.", RtpcName);
        return;
    }

    // Push the initial value.
    SetValue(InitialValue);
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioRtpcScript::OnUpdate()
{
    // RTPC values are set on-demand — nothing to do per-frame.
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioRtpcScript::OnDisable()
{
    if (_proxy != nullptr && _rtpcId != INVALID_AUDIO_SYSTEM_ID)
        ResetValue(false);

    _rtpcId       = INVALID_AUDIO_SYSTEM_ID;
    _currentValue = 0.0f;

    AudioProxyDependentScript::OnDisable();
}

// ============================================================================
//  SetValue
// ============================================================================

void AudioRtpcScript::SetValue(float value, bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioRtpcScript] SetValue: proxy is not available (component may be disabled).");
        return;
    }

    if (_rtpcId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioRtpcScript] SetValue: RTPC '{0}' is not resolved.", RtpcName);
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

void AudioRtpcScript::ResetValue(bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioRtpcScript] ResetValue: proxy is not available (component may be disabled).");
        return;
    }

    if (_rtpcId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioRtpcScript] ResetValue: RTPC '{0}' is not resolved.", RtpcName);
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

float AudioRtpcScript::GetValue() const
{
    return _currentValue;
}
