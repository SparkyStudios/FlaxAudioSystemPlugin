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

#include <Engine/Core/Types/String.h>
#include <Engine/Core/Types/StringView.h>

#include "../Core/AudioSystemData.h"
#include "AudioSystemScript.h"

/// <summary>
/// Activates a named switch state on a parent AudioProxyActor entity.
///
/// Requires to be attached to a AudioProxyActor.
///
/// The switch state specified in DefaultSwitchState is applied within Switch
/// when the script becomes active. Call SetState() to use the configured
/// Switch, or SetStateForSwitch() to target an explicit switch at runtime.
/// </summary>
API_CLASS()
class AUDIOSYSTEM_API AudioSwitchStateScript : public AudioProxyDependentScript
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSwitchStateScript);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// The switch name that owns the state to activate when this component becomes active.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"The switch name that owns the state to activate on init.\")")
    String Switch;

    /// <summary>
    /// The switch state name to activate when this component becomes active.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1), Tooltip(\"The switch state name to activate on init.\")")
    String DefaultSwitchState;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Resolves the proxy and applies DefaultSwitchState if Switch and DefaultSwitchState are configured.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// No per-frame work — switch states are set on-demand.
    /// </summary>
    void OnUpdate() override;

    /// <summary>
    /// Releases the proxy reference.
    /// </summary>
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// <summary>
    /// Resolve stateName within the configured Switch and activate it on
    /// the entity.
    /// </summary>
    /// <param name="stateName">Name of the switch state to activate.</param>
    /// <param name="sync">When true, the request blocks the calling thread.</param>
    API_FUNCTION()
    void SetState(const StringView& stateName, bool sync = false);

    /// <summary>
    /// Resolve stateName within switchName and activate it on the entity.
    /// </summary>
    /// <param name="switchName">Name of the switch that owns the state.</param>
    /// <param name="stateName">Name of the switch state to activate.</param>
    /// <param name="sync">When true, the request blocks the calling thread.</param>
    API_FUNCTION()
    void SetStateForSwitch(const StringView& switchName, const StringView& stateName, bool sync = false);

    /// <returns>The name of the last switch state successfully submitted.</returns>
    API_FUNCTION()
    const String& GetState() const;

  private:
    /// <summary>
    /// Name of the last switch state that was submitted to the audio system.
    /// </summary>
    String _currentStateName;
};
