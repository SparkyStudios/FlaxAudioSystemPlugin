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

#include <Engine/Core/Collections/Array.h>

#include "../Actors/AudioSystemEnvironmentActor.h"

// Forward declarations
class AudioListenerActor;
class AudioProxyActor;

/// <summary>
/// Tracks active audio environments and the default listener for a scene.
/// </summary>
class AUDIOSYSTEM_API AudioWorld
{
  public:
    // ========================================================================
    //  Per-frame update (called from AudioSystem::UpdateSound())
    // ========================================================================

    /// <summary>
    /// Called once per frame after the ATL has processed its requests.
    /// </summary>
    void Update();

    // ========================================================================
    //  Environment registration
    // ========================================================================

    /// <summary>
    /// Register an environment component as active.
    /// Logs a warning and does nothing if comp is null or already registered.
    /// </summary>
    void AddEnvironment(const AudioSystemEnvironmentActor* comp);

    /// <summary>
    /// Unregister an environment component.
    /// Logs a warning if comp is not currently registered.
    /// </summary>
    void RemoveEnvironment(const AudioSystemEnvironmentActor* comp);

    /// <returns>The list of currently active environment components (read-only).</returns>
    const Array<const AudioSystemEnvironmentActor*>& GetEnvironments() const;

    // ========================================================================
    //  Default listener
    // ========================================================================

    /// <summary>
    /// Set the default listener used for occlusion ray casting.
    /// Passing null clears the current default listener.
    /// </summary>
    void SetDefaultListener(const AudioListenerActor* listener);

    /// <returns>The current default listener, or null if none is set.</returns>
    const AudioListenerActor* GetDefaultListener() const;

    // ========================================================================
    //  Proxy registration
    // ========================================================================

    /// <summary>
    /// Register an active audio proxy for environment updates.
    /// </summary>
    void AddProxy(AudioProxyActor* proxy);

    /// <summary>
    /// Unregister a proxy (e.g., on EndPlay).
    /// </summary>
    void RemoveProxy(AudioProxyActor* proxy);

  private:
    Array<const AudioSystemEnvironmentActor*> _environments;
    Array<AudioProxyActor*>                   _proxies;
    const AudioListenerActor*                 _defaultListener = nullptr;
};
