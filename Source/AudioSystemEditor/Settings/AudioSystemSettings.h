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

#pragma once

#include <Engine/Core/Config/Settings.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingType.h>

/// <summary>
/// Editor preferences for the AudioSystem plugin.
///
/// Provides configurable settings that affects the behavior of the AudioSystem at runtime.
/// </summary>
API_CLASS(sealed, Namespace = "FlaxEditor.Content.Settings")
class AUDIOSYSTEMEDITOR_API AudioSystemSettings : public SettingsBase
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AudioSystemSettings);
    API_AUTO_SERIALIZATION();

  public:
    AudioSystemSettings();

    /// <summary>
    /// If checked, audio system will be enabled in build game. Can be used if game uses custom audio engine.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"Enable audio system in build game.\")")
    bool Enabled = true;

    /// <summary>
    /// Master gain level applied to all audio output. Range [0.0, 1.0].
    /// </summary>
    API_FIELD(Attributes = "Limit(0.0f, 1.0f), EditorOrder(1), Tooltip(\"Master gain level for all audio output. Range 0 to 1.\")")
    float MasterGain = 1.0f;

    /// <summary>
    /// When true, all audio output is silenced regardless of master gain.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(2), Tooltip(\"Mute all audio output.\")")
    bool MuteAudio = false;

    /// <returns>The global singleton instance.</returns>
    DECLARE_SETTINGS_GETTER(AudioSystemSettings);

    /// <summary>
    /// Applies the settings to the AudioSystem.
    /// </summary>
    void Apply() override;
};
