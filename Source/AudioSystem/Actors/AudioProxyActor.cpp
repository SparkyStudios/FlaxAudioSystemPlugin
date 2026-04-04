// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "AudioProxyActor.h"

#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Quaternion.h>
#include <Engine/Engine/Time.h>
#include <Engine/Level/Scene/SceneRendering.h>
#include <Engine/Scripting/Scripting.h>

#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

// ============================================================================
//  Static state
// ============================================================================

// Entity IDs
AudioSystemDataID AudioProxyActor::_nextEntityId = 2;

AudioProxyActor::AudioProxyActor(const SpawnParams& params)
    : Actor(params)
{
}

// ============================================================================
//  OnEnable / OnDisable — viewport icon registration
// ============================================================================

void AudioProxyActor::OnEnable()
{
    Actor::OnEnable();

#if USE_EDITOR
    GetSceneRendering()->AddViewportIcon(this);
#endif
}

void AudioProxyActor::OnDisable()
{
#if USE_EDITOR
    GetSceneRendering()->RemoveViewportIcon(this);
#endif

    Actor::OnDisable();
}

// ============================================================================
//  OnBeginPlay
// ============================================================================

void AudioProxyActor::OnBeginPlay()
{
    Actor::OnBeginPlay();

    // Allocate a unique entity ID (main-thread only — no atomics required).
    _entityId        = ++_nextEntityId;
    _hasLastPosition = false;
    _transformDirty  = true;

    // Submit an async registration request to the audio system.
    AudioRequest req;
    req.Type     = AudioRequestType::RegisterEntity;
    req.EntityId = _entityId;
    AudioSystem::Get()->SendRequest(std::move(req));

    // Bind the per-frame update to receive Scripting::Update callbacks.
    Scripting::Update.Bind<AudioProxyActor, &AudioProxyActor::OnFrameUpdate>(this);

    AudioSystem::Get()->GetWorld().AddProxy(this);
}

// ============================================================================
//  OnEndPlay
// ============================================================================

void AudioProxyActor::OnEndPlay()
{
    AudioSystem::Get()->GetWorld().RemoveProxy(this);

    // Unbind the per-frame update before tearing down any state.
    Scripting::Update.Unbind<AudioProxyActor, &AudioProxyActor::OnFrameUpdate>(this);

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

void AudioProxyActor::OnTransformChanged()
{
    Actor::OnTransformChanged();
    _transformDirty = true;
}

// ============================================================================
//  OnFrameUpdate  (private — Scripting::Update delegate)
// ============================================================================

void AudioProxyActor::OnFrameUpdate()
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
    Vector3     velocity = Vector3::Zero;
    const float dt       = Time::GetDeltaTime();
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

AudioSystemDataID AudioProxyActor::GetEntityId() const
{
    return _entityId;
}

// ============================================================================
//  SetEnvironmentAmount
// ============================================================================

void AudioProxyActor::SetEnvironmentAmount(AudioSystemDataID envId, float amount)
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

float AudioProxyActor::GetEnvironmentAmount(AudioSystemDataID envId) const
{
    const float* value = _environmentAmounts.TryGet(envId);
    return (value != nullptr) ? *value : 0.0f;
}
