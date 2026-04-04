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

#include "AudioListenerActor.h"

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

// Listener IDs
AudioSystemDataID AudioListenerActor::_nextListenerId = 2;

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
    _listenerId          = ++_nextListenerId;
    _hasLastTransform    = false;
    _transformDirty      = true;
    _registeredAsDefault = false;

    AudioRequest req;
    req.Type              = AudioRequestType::RegisterListener;
    req.ListenerId        = _listenerId;
    req.IsDefaultListener = IsDefault;
    AudioSystem::Get()->SendRequest(std::move(req));

    if (IsDefault)
    {
        AudioSystem::Get()->GetWorld().SetDefaultListener(this);
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
        AudioSystem::Get()->GetWorld().SetDefaultListener(nullptr);
        _registeredAsDefault = false;
    }

    if (_listenerId != INVALID_AUDIO_SYSTEM_ID)
    {
        AudioRequest req;
        req.Type       = AudioRequestType::UnregisterListener;
        req.ListenerId = _listenerId;
        AudioSystem::Get()->SendRequest(std::move(req));
        _listenerId = INVALID_AUDIO_SYSTEM_ID;
    }

    _hasLastTransform = false;
    _transformDirty   = false;

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
    req.Type       = AudioRequestType::UpdateListenerTransform;
    req.ListenerId = _listenerId;
    req.Transform  = current;
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

    const Vector3    pos = posSource->GetPosition();
    const Quaternion rot = oriSource->GetOrientation();
    result.Forward       = rot * Vector3::Forward;
    result.Up            = rot * Vector3::Up;

    // Derive velocity from the delta between the last recorded position and
    // the current position, divided by the elapsed frame time.
    Vector3     velocity = Vector3::Zero;
    const float dt       = Time::GetDeltaTime();
    if (_hasLastTransform && dt > ZeroTolerance)
        velocity = (pos - _lastTransform.Position) / dt;

    result.Position = pos;
    result.Velocity = velocity;

    return result;
}
