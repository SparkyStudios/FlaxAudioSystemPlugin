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

#include "AudioSystemScript.h"

#include <Engine/Core/Log.h>
#include <Engine/Level/Actor.h>

#include "../Actors/AudioProxyActor.h"
#include "../Core/AudioSystemData.h"

// ============================================================================
//  OnEnable
// ============================================================================

void AudioProxyDependentScript::OnEnable()
{
    Actor* owner = GetActor();
    if (owner == nullptr)
    {
        LOG(Warning, "[AudioProxyDependentScript] OnEnable: owner Actor is null. Script will be disabled.");
        SetEnabled(false);
        return;
    }

    _proxy = Cast<AudioProxyActor>(owner);
    if (_proxy == nullptr)
    {
        LOG(Warning,
            "[AudioProxyDependentScript] OnEnable: owner Actor '{0}' is not an AudioProxyActor. Attach this script to an AudioProxyActor actor.",
            owner->GetName());
        SetEnabled(false);
        return;
    }
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioProxyDependentScript::OnDisable()
{
    _proxy = nullptr;
}

// ============================================================================
//  GetEntityId
// ============================================================================

AudioSystemDataID AudioProxyDependentScript::GetEntityId() const
{
    if (_proxy == nullptr)
        return INVALID_AUDIO_SYSTEM_ID;

    return _proxy->GetEntityId();
}
