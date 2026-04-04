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

#include "AudioSystemEnvironmentActor.h"

#include <Engine/Core/Log.h>
#include <Engine/Level/Scene/SceneRendering.h>

#include "../Core/AudioSystem.h"

// ============================================================================
//  OnEnable / OnDisable — viewport icon registration
// ============================================================================

void AudioSystemEnvironmentActor::OnEnable()
{
    Actor::OnEnable();

#if USE_EDITOR
    GetSceneRendering()->AddViewportIcon(this);
#endif
}

void AudioSystemEnvironmentActor::OnDisable()
{
#if USE_EDITOR
    GetSceneRendering()->RemoveViewportIcon(this);
#endif

    Actor::OnDisable();
}

// ============================================================================
//  OnBeginPlay
// ============================================================================

void AudioSystemEnvironmentActor::OnBeginPlay()
{
    Actor::OnBeginPlay();

    if (!EnvironmentName.HasChars())
    {
        LOG(Warning, "[AudioSystemEnvironmentActor] OnBeginPlay: EnvironmentName is empty. No environment will be applied.");
        _environmentId = INVALID_AUDIO_SYSTEM_ID;
        return;
    }

    _environmentId = AudioSystem::Get()->GetEnvironmentId(EnvironmentName);

    if (_environmentId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSystemEnvironmentActor] OnBeginPlay: Environment '{0}' could not be resolved.", EnvironmentName);
    }

    AudioSystem::Get()->GetWorld().AddEnvironment(this);
}

// ============================================================================
//  OnEndPlay
// ============================================================================

void AudioSystemEnvironmentActor::OnEndPlay()
{
    AudioSystem::Get()->GetWorld().RemoveEnvironment(this);
    _environmentId = INVALID_AUDIO_SYSTEM_ID;

    Actor::OnEndPlay();
}

// ============================================================================
//  GetEnvironmentId
// ============================================================================

AudioSystemDataID AudioSystemEnvironmentActor::GetEnvironmentId() const
{
    return _environmentId;
}

// ============================================================================
//  Debug draw (editor only)
// ============================================================================

#if USE_EDITOR

void AudioSystemEnvironmentActor::OnDebugDraw()
{
    Actor::OnDebugDraw();
}

void AudioSystemEnvironmentActor::OnDebugDrawSelected()
{
    Actor::OnDebugDrawSelected();
}

#endif
