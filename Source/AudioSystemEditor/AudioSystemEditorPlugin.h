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

#include <Engine/Scripting/Plugins/EditorPlugin.h>

// ============================================================================
//  AudioSystemEditorPlugin
//
//  Entry point for the AudioSystem editor plugin (C++ side).
//
//  Initialize() bootstraps the C++ sub-systems in this order:
//    1. Load (or create) AudioSystemSettings.
//    2. Subscribe to GameCooker.DeployFiles for middleware deployment.
//
//  All other editor features (toolbar, icons, asset proxies, play-mode events)
//  are handled by the C# EditorPlugin layer.
//
//  Deinitialize() reverses all registrations in the opposite order.
// ============================================================================

/// <summary>
/// Entry point for the AudioSystem editor plugin.
/// </summary>
API_CLASS()
class AUDIOSYSTEMEDITOR_API AudioSystemEditorPlugin : public EditorPlugin
{
    DECLARE_SCRIPTING_TYPE(AudioSystemEditorPlugin);

  public:
    /// <summary>
    /// Initializes the AudioSystem editor plugin and subscribe to GameCooker.DeployFiles middleware deployment.
    ///
    /// All other editor features (toolbar, icons, asset proxies, play-mode events)
    /// are handled by the C# EditorPlugin layer.
    /// </summary>
    void Initialize() override;
    void Deinitialize() override;

  private:
    static bool _initialized;
};
