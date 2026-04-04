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

#include <Engine/Core/Math/Color.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Level/Actor.h>

#include "../Core/AudioSystemData.h"

// Forward declarations
class AudioProxyActor;

/// <summary>
/// Abstract base Actor class for audio environment zone actors.
///
/// Resolves the environment name to an ID on BeginPlay and registers itself
/// with the AudioWorld so that the per-frame update can push
/// wet-send amounts to nearby proxies.
/// </summary>
API_CLASS(Abstract, Attributes = "ActorContextMenu(\"New/Audio\")")
class AUDIOSYSTEM_API AudioSystemEnvironmentActor : public Actor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemEnvironmentActor);

  public:
    explicit AudioSystemEnvironmentActor(const SpawnParams& params) : Actor(params)
    {
    }

    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// Name of the environment (aux bus / reverb send) as defined in the
    /// audio control collection.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"Name of the environment (aux bus) as defined in the control collection.\")")
    String EnvironmentName;

    // ========================================================================
    //  Debug properties
    // ========================================================================

    /// <summary>
    /// Wireframe color used to visualize this environment zone in the editor viewport.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(100), EditorDisplay(\"Debug\", \"Color\"), Tooltip(\"Wireframe color for the environment zone debug visualization.\")")
    Color EnvironmentColor = Color(0.0f, 0.8f, 1.0f, 1.0f);

    // ========================================================================
    //  Actor lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Registers the viewport icon with the scene rendering system.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// Unregisters the viewport icon from the scene rendering system.
    /// </summary>
    void OnDisable() override;

    /// <summary>
    /// Resolves the environment ID and registers with the AudioWorld.
    /// </summary>
    void OnBeginPlay() override;

    /// <summary>
    /// Unregisters this Actor from the AudioWorld.
    /// </summary>
    void OnEndPlay() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// <returns>How much the given proxy is affected by this environment [0, 1]. Subclasses must implement the distance / overlap logic.</returns>
    virtual float GetEnvironmentAmount(const AudioProxyActor* proxy) const = 0;

    /// <returns>The environment ID resolved from EnvironmentName. Returns INVALID_AUDIO_SYSTEM_ID if not yet resolved or not found.</returns>
    AudioSystemDataID GetEnvironmentId() const;

    // ========================================================================
    //  Debug draw (editor only)
    // ========================================================================

#if USE_EDITOR
    /// <summary>
    /// Draws a dim wireframe of the environment zone every editor frame.
    /// </summary>
    void OnDebugDraw() override;

    /// <summary>
    /// Draws a full-color wireframe of the environment zone when selected.
    /// </summary>
    void OnDebugDrawSelected() override;
#endif

  protected:
    /// <summary>
    /// Resolved ID for EnvironmentName. INVALID_AUDIO_SYSTEM_ID if not found.
    /// </summary>
    AudioSystemDataID _environmentId = INVALID_AUDIO_SYSTEM_ID;
};
