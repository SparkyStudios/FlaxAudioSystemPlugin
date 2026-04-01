#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Quaternion.h>
#include <Engine/Engine/Time.h>
#include <Engine/Level/Scene/SceneRendering.h>
#include <Engine/Scripting/Scripting.h>

#include "AudioProxyComponent.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

// ============================================================================
//  Static state
// ============================================================================

// Entity IDs below 1000 are reserved for internal / listener use.
AudioSystemDataID AudioProxyComponent::_nextEntityId = 1000;

AudioProxyComponent::AudioProxyComponent(const SpawnParams& params)
    : Actor(params)
{
}

// ============================================================================
//  OnEnable / OnDisable — viewport icon registration
// ============================================================================

void AudioProxyComponent::OnEnable()
{
    Actor::OnEnable();

#if USE_EDITOR
    GetSceneRendering()->AddViewportIcon(this);
#endif
}

void AudioProxyComponent::OnDisable()
{
#if USE_EDITOR
    GetSceneRendering()->RemoveViewportIcon(this);
#endif

    Actor::OnDisable();
}

// ============================================================================
//  OnBeginPlay
// ============================================================================

void AudioProxyComponent::OnBeginPlay()
{
    Actor::OnBeginPlay();

    // Allocate a unique entity ID (main-thread only — no atomics required).
    _entityId = ++_nextEntityId;
    _hasLastPosition = false;
    _transformDirty  = true;

    // Submit an async registration request to the audio system.
    AudioRequest req;
    req.Type     = AudioRequestType::RegisterEntity;
    req.EntityId = _entityId;
    AudioSystem::Get()->SendRequest(std::move(req));

    // Bind the per-frame update to receive Scripting::Update callbacks.
    Scripting::Update.Bind<AudioProxyComponent, &AudioProxyComponent::OnFrameUpdate>(this);

    AudioSystem::Get()->GetWorldModule().AddProxy(this);
}

// ============================================================================
//  OnEndPlay
// ============================================================================

void AudioProxyComponent::OnEndPlay()
{
    AudioSystem::Get()->GetWorldModule().RemoveProxy(this);

    // Unbind the per-frame update before tearing down any state.
    Scripting::Update.Unbind<AudioProxyComponent, &AudioProxyComponent::OnFrameUpdate>(this);

    if (_entityId != INVALID_AUDIO_SYSTEM_ID)
    {
        AudioRequest req;
        req.Type     = AudioRequestType::UnregisterEntity;
        req.EntityId = _entityId;
        AudioSystem::Get()->SendRequest(std::move(req));

        _entityId        = INVALID_AUDIO_SYSTEM_ID;
        _hasLastPosition = false;
        _transformDirty  = false;
        _environmentAmounts.Clear();
    }

    Actor::OnEndPlay();
}

// ============================================================================
//  OnTransformChanged
// ============================================================================

void AudioProxyComponent::OnTransformChanged()
{
    Actor::OnTransformChanged();
    _transformDirty = true;
}

// ============================================================================
//  OnFrameUpdate  (private — Scripting::Update delegate)
// ============================================================================

void AudioProxyComponent::OnFrameUpdate()
{
    if (_entityId == INVALID_AUDIO_SYSTEM_ID)
        return;
    if (!_transformDirty)
        return;

    const Vector3    pos     = GetPosition();
    const Quaternion rot     = GetOrientation();
    const Vector3    forward = rot * Vector3::Forward;
    const Vector3    up      = rot * Vector3::Up;

    // Derive velocity from the delta between the last recorded position and
    // the current position, divided by the elapsed frame time.
    Vector3 velocity = Vector3::Zero;
    const float dt = Time::GetDeltaTime();
    if (_hasLastPosition && dt > ZeroTolerance)
        velocity = (pos - _lastPosition) / dt;

    AudioSystemTransform transform;
    transform.Position = pos;
    transform.Forward  = forward;
    transform.Up       = up;
    transform.Velocity = velocity;

    AudioRequest req;
    req.Type      = AudioRequestType::UpdateEntityTransform;
    req.EntityId  = _entityId;
    req.Transform = transform;
    AudioSystem::Get()->SendRequest(std::move(req));

    _lastPosition    = pos;
    _hasLastPosition = true;
    _transformDirty  = false;
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
    req.Type     = AudioRequestType::SetEnvironmentAmount;
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
