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

// ============================================================================
//  AudioSystemBuildHook
//
//  Subscribes to the GameCooker deployment event and delegates file
//  deployment to the loaded audio middleware. The hook itself has no
//  knowledge of middleware-specific assets or file layouts.
//
//  Lifecycle:
//    Register()   — called from AudioSystemEditorPlugin::Initialize()
//    Unregister() — called from AudioSystemEditorPlugin::Deinitialize()
// ============================================================================

class AUDIOSYSTEMEDITOR_API AudioSystemBuildHook
{
  public:
    /// <summary>
    /// Subscribe to GameCooker.DeployFiles.
    /// </summary>
    static void Register();

    /// <summary>
    /// Unsubscribe from GameCooker.DeployFiles.
    /// </summary>
    static void Unregister();

  private:
    AudioSystemBuildHook()  = delete;
    ~AudioSystemBuildHook() = delete;

    /// <summary>
    /// Called by GameCooker during the deploy phase.
    /// Retrieves CookingData via GameCooker::GetCurrentData() and
    /// forwards to AudioSystem::DeployFiles().
    /// </summary>
    static void OnDeployFiles();
};
