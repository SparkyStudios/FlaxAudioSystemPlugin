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

#include <Engine/Core/ISerializable.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/Script.h>

#include "../Core/AudioSystem.h"

/// <summary>
/// Controls the loading of sound banks.
///
/// This script allows to automatically or manually load sound banks. In both cases, sound banks
/// are always unloaded when the script is destroyed.
/// </summary>
API_CLASS(Attributes = "FlaxEngine.Category(\"Audio System\")")
class AUDIOSYSTEM_API AudioPreloadScript : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioPreloadScript);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// The name of the sound bank to load.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"The name of the sound bank to load.\")")
    String SoundBankName;

    /// <summary>
    /// Whether to automatically load the sound bank on enable.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1), Tooltip(\"Whether to automatically load the sound bank on enable.\")")
    bool LoadOnEnable = true;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Loads the provided sound bank if <ref cref="LoadOnEnable" /> is <c>true</c>.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// Unloads the provided sound bank if loaded.
    /// </summary>
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// <summary>
    /// Manually loads the sound bank.
    ///
    /// This operation is noop if the sound bank was already loaded.
    /// </summary>
    /// <returns><c>true</c> if the sound bank was loaded successfully, <c>false</c> otherwise.</returns>
    API_FUNCTION()
    bool Load();

    /// <summary>
    /// Manually unloads the sound bank.
    ///
    /// This operation is noop if the sound bank was not loaded.
    /// </summary>
    /// <returns><c>true</c> if the sound bank was unloaded successfully, <c>false</c> otherwise.</returns>
    API_FUNCTION()
    bool Unload();

  private:
    /// <summary>
    /// Whether the sound bank is loaded.
    /// </summary>
    bool _loaded = false;

    /// <summary>
    /// The ID of the sound bank.
    /// </summary>
    AudioSystemControlID _bankId = INVALID_AUDIO_SYSTEM_ID;
};
