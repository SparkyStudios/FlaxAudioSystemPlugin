#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Quaternion.h>
#include <Engine/Engine/Time.h>
#include <Engine/Level/Actor.h>

#include "AudioProxyComponent.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

// ============================================================================
//  Static state
// ============================================================================

AudioProxyComponent::AudioProxyComponent(const SpawnParams& params)
    : AudioSystemComponent(params)
{
}

// Entity IDs below 1000 are reserved for internal / listener use.
AudioSystemDataID AudioProxyComponent::_nextEntityId = 1000;

// ============================================================================
//  OnEnable
// ============================================================================

void AudioProxyComponent::OnEnable()
{
    // Allocate a unique entity ID (main-thread only — no atomics required).
    _entityId = ++_nextEntityId;
    _hasLastTransform = false;

    // Submit an async registration request to the audio system.
    AudioRequest req;
    req.Type = AudioRequestType::RegisterEntity;
    req.EntityId = _entityId;
    AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioProxyComponent::OnUpdate()
{
    if (_entityId == INVALID_AUDIO_SYSTEM_ID)
        return;

    const AudioSystemTransform current = ComputeCurrentTransform();
    if (_hasLastTransform && current == _lastTransform)
        return;

    AudioRequest req;
    req.Type = AudioRequestType::UpdateEntityTransform;
    req.EntityId  = _entityId;
    req.Transform = current;
    AudioSystem::Get()->SendRequest(std::move(req));

    _lastTransform    = current;
    _hasLastTransform = true;
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioProxyComponent::OnDisable()
{
    if (_entityId == INVALID_AUDIO_SYSTEM_ID)
        return;

    AudioRequest req;
    req.Type = AudioRequestType::UnregisterEntity;
    req.EntityId = _entityId;
    AudioSystem::Get()->SendRequest(std::move(req));

    _entityId         = INVALID_AUDIO_SYSTEM_ID;
    _hasLastTransform = false;
    _environmentAmounts.Clear();
}

// ============================================================================
//  GetEntityId
// ============================================================================

AudioSystemDataID AudioProxyComponent::GetEntityId() const
{
    return _entityId;
}

// ============================================================================
//  SetEnvironmentAmount
// ============================================================================

void AudioProxyComponent::SetEnvironmentAmount(AudioSystemDataID envId, float amount)
{
    if (envId == INVALID_AUDIO_SYSTEM_ID)
        return;

    if (_entityId == INVALID_AUDIO_SYSTEM_ID)
        return;

    // Check whether the amount has actually changed before sending a request.
    float* existing = _environmentAmounts.TryGet(envId);
    if (existing != nullptr && Math::NearEqual(*existing, amount))
        return;

    _environmentAmounts[envId] = amount;

    AudioRequest req;
    req.Type = AudioRequestType::SetEnvironmentAmount;
    req.EntityId = _entityId;
    req.ObjectId = envId;
    req.Amount   = amount;
    AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  GetEnvironmentAmount
// ============================================================================

float AudioProxyComponent::GetEnvironmentAmount(AudioSystemDataID envId) const
{
    const float* value = _environmentAmounts.TryGet(envId);
    return (value != nullptr) ? *value : 0.0f;
}

// ============================================================================
//  ComputeCurrentTransform  (private helper)
// ============================================================================

AudioSystemTransform AudioProxyComponent::ComputeCurrentTransform() const
{
    const Actor* owner = GetActor();

    AudioSystemTransform result;

    if (owner == nullptr)
        return result;

    const Vector3      pos = owner->GetPosition();
    const Quaternion   rot = owner->GetOrientation();
    const Vector3  forward = rot * Vector3::Forward;
    const Vector3       up = rot * Vector3::Up;

    // Derive velocity from the delta between the last recorded position and
    // the current position, divided by the elapsed frame time.
    Vector3 velocity = Vector3::Zero;
    const float dt = Time::GetDeltaTime();
    if (_hasLastTransform && dt > ZeroTolerance)
        velocity = (pos - _lastTransform.Position) / dt;

    result.Position = pos;
    result.Forward  = forward;
    result.Up       = up;
    result.Velocity = velocity;

    return result;
}
