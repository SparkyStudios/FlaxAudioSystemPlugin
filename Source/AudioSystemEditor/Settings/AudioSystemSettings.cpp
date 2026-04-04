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

#include "AudioSystemSettings.h"

#include <Engine/Content/Content.h>
#include <Engine/Content/JsonAsset.h>
#include <Engine/Core/Config/GameSettings.h>
#include <Engine/Core/Log.h>
#include <Engine/Scripting/Internal/InternalCalls.h>

#include "../../AudioSystem/Core/AudioSystem.h"

IMPLEMENT_GAME_SETTINGS_GETTER(AudioSystemSettings, "Audio System");

// ============================================================================
//  Constructor
// ============================================================================

AudioSystemSettings::AudioSystemSettings()
{
}

// ============================================================================
//  SettingsBase
// ============================================================================

void AudioSystemSettings::Apply()
{
    LOG(Info, "[AudioSystem] Applying settings.");

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr || !audioSystem->IsInitialized())
    {
        LOG(Info, "[AudioSystem] AudioSystem not yet started — cannot apply settings.");
        return;
    }

    audioSystem->SetMasterVolume(MasterGain);
    audioSystem->SetMasterMute(MuteAudio);
}

DEFINE_INTERNAL_CALL(void)
AudioSystemSettingsInternal_Apply()
{
    auto settings = AudioSystemSettings::Get();
    if (!settings)
        return;

    settings->Apply();
}
