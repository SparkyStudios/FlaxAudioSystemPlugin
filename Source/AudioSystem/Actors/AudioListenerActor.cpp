#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Quaternion.h>
#include <Engine/Engine/Time.h>
#include <Engine/Level/Scene/SceneRendering.h>
#include <Engine/Scripting/Scripting.h>

#include "AudioListenerActor.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

// ============================================================================
//  Static state
// ============================================================================

// Listener IDs start at 2000 to avoid collisions with entity IDs (from 1000).
AudioSystemDataID AudioListenerActor::_nextListenerId = 2000;

AudioListenerActor::AudioListenerActor(const SpawnParams& params)
    : Actor(params)
{
}

// ============================================================================
//  OnEnable / OnDisable — viewport icon registration
// ============================================================================

void AudioListenerActor::OnEnable()
{
    Actor::OnEnable();

#if USE_EDITOR
    GetSceneRendering()->AddViewportIcon(this);
#endif
}

void AudioListenerActor::OnDisable()
{
#if USE_EDITOR
    GetSceneRendering()->RemoveViewportIcon(this);
#endif

    Actor::OnDisable();
}

// ============================================================================
//  OnBeginPlay
// ============================================================================

void AudioListenerActor::OnBeginPlay()
{
    Actor::OnBeginPlay();

    // Allocate a unique listener ID (main-thread only — no atomics required).
    _listenerId = ++_nextListenerId;
    _hasLastTransform = false;
    _transformDirty = true;
    _registeredAsDefault = false;

    AudioRequest req;
    req.Type = AudioRequestType::RegisterListener;
    req.EntityId = _listenerId;
    AudioSystem::Get()->SendRequest(std::move(req));

    if (IsDefault)
    {
        AudioSystem::Get()->GetWorldModule().SetDefaultListener(this);
        _registeredAsDefault = true;
    }

    Scripting::Update.Bind<AudioListenerActor, &AudioListenerActor::OnFrameUpdate>(this);
}

// ============================================================================
//  OnEndPlay
// ============================================================================

void AudioListenerActor::OnEndPlay()
{
    Scripting::Update.Unbind<AudioListenerActor, &AudioListenerActor::OnFrameUpdate>(this);

    if (_registeredAsDefault)
    {
        AudioSystem::Get()->GetWorldModule().SetDefaultListener(nullptr);
        _registeredAsDefault = false;
    }

    if (_listenerId != INVALID_AUDIO_SYSTEM_ID)
    {
        AudioRequest req;
        req.Type = AudioRequestType::UnregisterListener;
        req.EntityId = _listenerId;
        AudioSystem::Get()->SendRequest(std::move(req));
        _listenerId = INVALID_AUDIO_SYSTEM_ID;
    }

    _hasLastTransform = false;
    _transformDirty = false;

    Actor::OnEndPlay();
}

// ============================================================================
//  OnTransformChanged
// ============================================================================

void AudioListenerActor::OnTransformChanged()
{
    Actor::OnTransformChanged();
    _transformDirty = true;
}

// ============================================================================
//  OnFrameUpdate  (private — bound to Scripting::Update)
// ============================================================================

void AudioListenerActor::OnFrameUpdate()
{
    if (_listenerId == INVALID_AUDIO_SYSTEM_ID)
        return;

    // When override actors are active, poll every frame since OnTransformChanged
    // only fires for THIS actor's transform.
    const bool hasOverrides = PositionObject.Get() != nullptr || OrientationObject.Get() != nullptr;
    if (!_transformDirty && !hasOverrides)
        return;

    const AudioSystemTransform current = ComputeCurrentTransform();
    if (_hasLastTransform && current == _lastTransform)
    {
        _transformDirty = false;
        return;
    }

    AudioRequest req;
    req.Type = AudioRequestType::UpdateListenerTransform;
    req.EntityId  = _listenerId;
    req.Transform = current;
    AudioSystem::Get()->SendRequest(std::move(req));

    _lastTransform    = current;
    _hasLastTransform = true;
    _transformDirty   = false;
}

// ============================================================================
//  GetListenerId
// ============================================================================

AudioSystemDataID AudioListenerActor::GetListenerId() const
{
    return _listenerId;
}

// ============================================================================
//  ComputeCurrentTransform  (private helper)
// ============================================================================

AudioSystemTransform AudioListenerActor::ComputeCurrentTransform() const
{
    const Actor* posSource = PositionObject.Get();
    const Actor* oriSource = OrientationObject.Get();

    // Fall back to this actor when override actors are not set.
    if (posSource == nullptr)
        posSource = this;
    if (oriSource == nullptr)
        oriSource = this;

    AudioSystemTransform result;

    const Vector3 pos = posSource->GetPosition();
    const Quaternion rot = oriSource->GetOrientation();
    result.Forward = rot * Vector3::Forward;
    result.Up      = rot * Vector3::Up;

    // Derive velocity from the delta between the last recorded position and
    // the current position, divided by the elapsed frame time.
    Vector3 velocity = Vector3::Zero;
    const float dt = Time::GetDeltaTime();
    if (_hasLastTransform && dt > ZeroTolerance)
        velocity = (pos - _lastTransform.Position) / dt;

    result.Position = pos;
    result.Velocity = velocity;

    return result;
}
