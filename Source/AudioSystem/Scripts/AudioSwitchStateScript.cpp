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

    if (SwitchStateName.IsEmpty())
        return;

    if (SwitchName.IsEmpty())
    {
        LOG(Warning, "[AudioSwitchStateScript] OnEnable: SwitchName is empty. Initial state '{0}' will not be applied.", SwitchStateName);
        return;
    }

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
    SetStateForSwitch(SwitchName, stateName, sync);
}

// ============================================================================
//  SetStateForSwitch
// ============================================================================

void AudioSwitchStateScript::SetStateForSwitch(const StringView& switchName, const StringView& stateName, bool sync)
{
    if (_proxy == nullptr)
    {
        LOG(Warning, "[AudioSwitchStateScript] SetStateForSwitch: proxy is not available (script may be disabled).");
        return;
    }

    if (switchName.IsEmpty())
    {
        LOG(Warning, "[AudioSwitchStateScript] SetStateForSwitch: switchName is empty. Request ignored.");
        return;
    }

    if (stateName.IsEmpty())
    {
        LOG(Warning, "[AudioSwitchStateScript] SetStateForSwitch: stateName is empty. Request ignored.");
        return;
    }

    const String switchNameString(switchName);
    const String stateNameString(stateName);
    const AudioSystemDataID stateId = AudioSystem::Get()->GetSwitchStateId(switchName, stateName);

    if (stateId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioSwitchStateScript] SetStateForSwitch: switch '{0}' state '{1}' could not be resolved.", switchNameString, stateNameString);
        return;
    }

    AudioRequest req;
    req.Type     = AudioRequestType::SetSwitchState;
    req.EntityId = _proxy->GetEntityId();
    req.ObjectId = stateId;
    req.Callback = [this, stateNameString](bool success)
    {
        if (!success)
        {
            LOG(Warning, "[AudioSwitchStateScript] SetStateForSwitch: failed to set switch state '{0}'.", stateNameString);
            return;
        }

        _currentStateName = stateNameString;
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
