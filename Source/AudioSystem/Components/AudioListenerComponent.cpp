#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Quaternion.h>
#include <Engine/Core/Time.h>
#include <Engine/Level/Actor.h>

#include "AudioListenerComponent.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

// ============================================================================
//  Static state
// ============================================================================

// Listener IDs start at 2000 to avoid collisions with entity IDs (from 1000).
AudioSystemDataID AudioListenerComponent::_nextListenerId = 2000;

// ============================================================================
//  Scripting type registration
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE(AudioListenerComponent, AudioSystemComponent,
    "AudioSystem.AudioListenerComponent", nullptr, nullptr);

// ============================================================================
//  OnEnable
// ============================================================================

void AudioListenerComponent::OnEnable()
{
    // Allocate a unique listener ID (main-thread only — no atomics required).
    _listenerId = ++_nextListenerId;
    _hasLastTransform = false;
    _registeredAsDefault = false;

    RegisterListenerRequest req;
    req.EntityId = _listenerId;
    AudioSystem::Get()->SendRequest(std::move(req));

    if (IsDefault)
    {
        AudioSystem::Get()->GetWorldModule().SetDefaultListener(this);
        _registeredAsDefault = true;
    }
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioListenerComponent::OnUpdate()
{
    if (_listenerId == INVALID_AUDIO_SYSTEM_ID)
        return;

    const AudioSystemTransform current = ComputeCurrentTransform();
    if (_hasLastTransform && current == _lastTransform)
        return;

    UpdateListenerTransformRequest req;
    req.EntityId  = _listenerId;
    req.Transform = current;
    AudioSystem::Get()->SendRequest(std::move(req));

    _lastTransform    = current;
    _hasLastTransform = true;
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioListenerComponent::OnDisable()
{
    if (_listenerId == INVALID_AUDIO_SYSTEM_ID)
        return;

    if (_registeredAsDefault)
    {
        AudioSystem::Get()->GetWorldModule().SetDefaultListener(nullptr);
        _registeredAsDefault = false;
    }

    UnregisterListenerRequest req;
    req.EntityId = _listenerId;
    AudioSystem::Get()->SendRequest(std::move(req));

    _listenerId       = INVALID_AUDIO_SYSTEM_ID;
    _hasLastTransform = false;
}

// ============================================================================
//  GetListenerId
// ============================================================================

AudioSystemDataID AudioListenerComponent::GetListenerId() const
{
    return _listenerId;
}

// ============================================================================
//  ComputeCurrentTransform  (private helper)
// ============================================================================

AudioSystemTransform AudioListenerComponent::ComputeCurrentTransform() const
{
    const Actor* posSource = PositionObject.Get();
    const Actor* oriSource = OrientationObject.Get();
    const Actor* owner     = GetActor();

    // Fall back to the owner actor when override actors are not set.
    if (posSource == nullptr)
        posSource = owner;
    if (oriSource == nullptr)
        oriSource = owner;

    AudioSystemTransform result;

    if (posSource == nullptr)
        return result;

    const Vector3 pos = posSource->GetPosition();

    Vector3 forward = Vector3::Forward;
    Vector3 up      = Vector3::Up;
    if (oriSource != nullptr)
    {
        const Quaternion rot = oriSource->GetOrientation();
        forward = rot * Vector3::Forward;
        up      = rot * Vector3::Up;
    }

    // Derive velocity from the delta between the last recorded position and
    // the current position, divided by the elapsed frame time.
    Vector3 velocity = Vector3::Zero;
    const float dt = Time::GetDeltaTime();
    if (_hasLastTransform && dt > ZERO_TOLERANCE)
        velocity = (pos - _lastTransform.Position) / dt;

    result.Position = pos;
    result.Forward  = forward;
    result.Up       = up;
    result.Velocity = velocity;

    return result;
}
