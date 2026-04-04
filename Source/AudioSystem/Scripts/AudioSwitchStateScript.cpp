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

#include "AudioSwitchStateScript.h"

#include <Engine/Core/Log.h>

#include "../Actors/AudioProxyActor.h"
#include "../Core/AudioSystem.h"
#include "../Core/AudioSystemRequests.h"

AudioSwitchStateScript::AudioSwitchStateScript(const SpawnParams& params) : AudioProxyDependentScript(params)
{
}

// ============================================================================
//  OnEnable
// ============================================================================

void AudioSwitchStateScript::OnEnable()
{
    AudioProxyDependentScript::OnEnable();

    if (_proxy == nullptr)
        return;

    // Apply the initial switch state if one is configured.
    if (SwitchStateName.HasChars())
        SetState(SwitchStateName);
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioSwitchStateScript::OnUpdate()
{
    // Switch states are set on-demand — nothing to do per-frame.
}

// ============================================================================
//  OnDisable
// ============================================================================

void AudioSwitchStateScript::OnDisable()
{
    _currentStateName.Clear();

    AudioProxyDependentScript::OnDisable();
}

// ============================================================================
//  SetState
// ============================================================================

void AudioSwitchStateScript::SetState(const StringView& stateName, bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioSwitchStateScript] SetState: proxy is not available (script may be disabled).");
        return;
    }

    if (stateName.IsEmpty())
    {
        LOG(Warning, "[AudioSwitchStateScript] SetState: stateName is empty. Request ignored.");
        return;
    }

    const AudioSystemDataID stateId = AudioSystem::Get()->GetSwitchStateId(stateName);

    if (stateId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSwitchStateScript] SetState: switch state '{0}' could not be resolved.", String(stateName));
        return;
    }

    AudioRequest req;
    req.Type     = AudioRequestType::SetSwitchState;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = stateId;
    req.Callback = [this, stateName](bool success)
    {
        if (!success)
        {
            LOG(Warning, "[AudioSwitchStateScript] SetState: failed to set switch state '{0}'.", String(stateName));
            return;
        }

        _currentStateName = stateName;
    };

    if (sync)
        AudioSystem::Get()->SendRequestSync(std::move(req));
    else
        AudioSystem::Get()->SendRequest(std::move(req));
}

// ============================================================================
//  GetState
// ============================================================================

const String& AudioSwitchStateScript::GetState() const
{
    return _currentStateName;
}
