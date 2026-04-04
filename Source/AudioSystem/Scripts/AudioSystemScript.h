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

#include <Engine/Scripting/Script.h>

#include "../Core/AudioSystemData.h"

// Forward declarations
class AudioProxyActor;

/// <summary>
/// Abstract base class for all AudioSystem scripts.
///
/// Derived classes must implement OnUpdate(). All audio scripts that need
/// per-frame processing inherit from this class.
/// </summary>
API_CLASS(Abstract)
class AUDIOSYSTEM_API AudioSystemScript : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemScript);

  public:
    explicit AudioSystemScript(const SpawnParams& params) : Script(params)
    {
    }

  protected:
    /// <summary>
    /// Called every frame when the component is active. Must be implemented by subclasses.
    /// </summary>
    void OnUpdate() override = 0;
};

/// <summary>
/// Abstract base for audio scripts that require to be attached on a AudioProxyActor.
///
/// Resolves the parent proxy in OnEnable and releases the reference in OnDisable.
/// Subclasses can access the proxy via _proxy and the entity ID via GetEntityId().
/// </summary>
API_CLASS(Abstract)
class AUDIOSYSTEM_API AudioProxyDependentScript : public AudioSystemScript
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioProxyDependentScript);

  public:
    explicit AudioProxyDependentScript(const SpawnParams& params) : AudioSystemScript(params)
    {
    }

    /// <summary>
    /// Called when this script becomes active.
    /// Locates the parent AudioProxyActor on the owner Actor.
    /// Disables itself (with a logged warning) if no proxy is found.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// Called when this script becomes inactive.
    /// Clears the cached proxy pointer.
    /// </summary>
    void OnDisable() override;

  protected:
    /// <returns>The AudioSystemDataID assigned by the parent proxy. Returns INVALID_AUDIO_SYSTEM_ID if no proxy has
    /// been resolved.</returns>
    AudioSystemDataID GetEntityId() const;

    /// <summary>
    /// Cached pointer to the parent AudioProxyActor resolved in OnEnable.
    /// </summary>
    AudioProxyActor* _proxy = nullptr;
};
