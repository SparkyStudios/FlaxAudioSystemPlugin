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

#include "AudioWorld.h"

#include <Engine/Core/Log.h>

#include "../Actors/AudioProxyActor.h"

// ============================================================================
//  Per-frame update
// ============================================================================

void AudioWorld::Update()
{
    // For each active proxy, query each environment for the send amount
    // and push it to the proxy (which dispatches to the audio system if changed).
    for (AudioProxyActor* proxy : _proxies)
    {
        if (proxy == nullptr)
            continue;

        for (const AudioSystemEnvironmentActor* env : _environments)
        {
            if (env == nullptr)
                continue;

            const AudioSystemDataID envId = env->GetEnvironmentId();
            if (envId == INVALID_AUDIO_SYSTEM_ID)
                continue;

            const float amount = env->GetEnvironmentAmount(proxy);
            proxy->SetEnvironmentAmount(envId, amount);
        }
    }
}

// ============================================================================
//  Environment registration
// ============================================================================

void AudioWorld::AddEnvironment(const AudioSystemEnvironmentActor* comp)
{
    if (comp == nullptr)
    {
        LOG(Warning, "[AudioWorld] AddEnvironment: null component pointer — ignoring.");
        return;
    }

    if (_environments.Contains(comp))
    {
        LOG(Warning, "[AudioWorld] AddEnvironment: component is already registered — ignoring duplicate.");
        return;
    }

    _environments.Add(comp);
}

void AudioWorld::RemoveEnvironment(const AudioSystemEnvironmentActor* comp)
{
    const int32 index = _environments.Find(comp);

    if (index == -1)
    {
        LOG(Warning, "[AudioWorld] RemoveEnvironment: component was not registered — ignoring.");
        return;
    }

    _environments.RemoveAt(index);
}

const Array<const AudioSystemEnvironmentActor*>& AudioWorld::GetEnvironments() const
{
    return _environments;
}

// ============================================================================
//  Default listener
// ============================================================================

void AudioWorld::SetDefaultListener(const AudioListenerActor* listener)
{
    _defaultListener = listener;
}

const AudioListenerActor* AudioWorld::GetDefaultListener() const
{
    return _defaultListener;
}

// ============================================================================
//  Proxy registration
// ============================================================================

void AudioWorld::AddProxy(AudioProxyActor* proxy)
{
    if (proxy == nullptr)
    {
        LOG(Warning, "[AudioWorld] AddProxy: null proxy pointer — ignoring.");
        return;
    }

    if (_proxies.Contains(proxy))
        return;

    _proxies.Add(proxy);
}

void AudioWorld::RemoveProxy(AudioProxyActor* proxy)
{
    const int32 index = _proxies.Find(proxy);
    if (index != -1)
        _proxies.RemoveAt(index);
}
