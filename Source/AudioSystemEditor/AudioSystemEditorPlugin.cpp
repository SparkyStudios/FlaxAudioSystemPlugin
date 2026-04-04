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

#include "AudioSystemEditorPlugin.h"

#include <Engine/Core/Log.h>

#include "Build/AudioSystemBuildHook.h"
#include "Settings/AudioSystemSettings.h"

// ============================================================================
//  Static state
// ============================================================================

bool AudioSystemEditorPlugin::_initialized = false;

// ============================================================================
//  Constructor
// ============================================================================

AudioSystemEditorPlugin::AudioSystemEditorPlugin(const SpawnParams& params) : EditorPlugin(params)
{
    _description.Name        = TEXT("AudioSystemEditor");
    _description.Category    = TEXT("Audio");
    _description.Author      = TEXT("Sparky Studios");
    _description.Description = TEXT("Editor tools for the AudioSystem plugin.");
}

// ============================================================================
//  Initialize
// ============================================================================

void AudioSystemEditorPlugin::Initialize()
{
    EditorPlugin::Initialize();

    if (_initialized)
        return;

    AudioSystemBuildHook::Register();

    _initialized = true;
    LOG(Info, "[AudioSystem] Editor plugin initialised.");
}

// ============================================================================
//  Deinitialize
// ============================================================================

void AudioSystemEditorPlugin::Deinitialize()
{
    if (_initialized)
    {
        AudioSystemBuildHook::Unregister();

        _initialized = false;
        LOG(Info, "[AudioSystem] Editor plugin deinitialised.");
    }

    EditorPlugin::Deinitialize();
}
