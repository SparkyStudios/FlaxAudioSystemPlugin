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

#include "AudioPreloadScript.h"

#include <Engine/Core/Log.h>

// ============================================================================
//  AudioPreloadScript
// ============================================================================

AudioPreloadScript::AudioPreloadScript(const SpawnParams& params) : Script(params)
{
}

// ============================================================================
//  AudioPreloadScript — OnEnable
// ============================================================================

void AudioPreloadScript::OnEnable()
{
    Script::OnEnable();

    AudioSystem* audioSystem = AudioSystem::Get();
    _bankId                  = audioSystem->GetBankId(SoundBankName);

    if (LoadOnEnable)
        Load();
}

// ============================================================================
//  AudioPreloadScript — OnDisable
// ============================================================================

void AudioPreloadScript::OnDisable()
{
    Unload();

    Script::OnDisable();
}

// ============================================================================
//  AudioPreloadScript — Load
// ============================================================================

bool AudioPreloadScript::Load()
{
    if (_loaded)
    {
        LOG(Warning, "[AudioPreloadScript] Bank already loaded: {0}", SoundBankName);
        return false;
    }

    if (_bankId == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Warning, "[AudioPreloadScript] Bank not found: {0}", SoundBankName);
        return false;
    }

    AudioSystem* audioSystem = AudioSystem::Get();

    AudioRequest req;
    req.Type     = AudioRequestType::LoadBank;
    req.ObjectId = _bankId;

    _loaded = audioSystem->SendRequestSync(MoveTemp(req));

    if (!_loaded)
    {
        LOG(Warning, "[AudioPreloadScript] Failed to load bank: {0}", SoundBankName);
    }

    return _loaded;
}

// ============================================================================
//  AudioPreloadScript — Unload
// ============================================================================

bool AudioPreloadScript::Unload()
{
    if (!_loaded)
        return false;

    AudioSystem* audioSystem = AudioSystem::Get();

    AudioRequest req;
    req.Type     = AudioRequestType::UnloadBank;
    req.ObjectId = _bankId;

    if (!audioSystem->SendRequestSync(MoveTemp(req)))
        return false;

    _loaded = false;
    return true;
}