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

#include "../Core/AudioSystemData.h"
#include "AudioSystemScript.h"

/// <summary>
/// Drives a named RTPC on a parent AudioProxyActor entity.
///
/// Requires to be attached to a AudioProxyActor.
///
/// The initial value is pushed when the component becomes active.
/// Calling ResetValue() or disabling the component restores the default.
/// </summary>
API_CLASS(Attributes = "FlaxEngine.Category(\"Audio System\")")
class AUDIOSYSTEM_API AudioRtpcScript : public AudioProxyDependentScript
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioRtpcScript);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// Name of the RTPC.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"Name of the RTPC.\")")
    String RtpcName;

    /// <summary>
    /// Value applied to the RTPC when this component becomes active.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1), Tooltip(\"Value applied on component enable.\")")
    float InitialValue = 0.0f;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Resolves the proxy and RTPC ID, then applies the initial value.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// No per-frame work — RTPC values are set on-demand.
    /// </summary>
    void OnUpdate() override;

    /// <summary>
    /// Resets the RTPC to its default and releases the proxy reference.
    /// </summary>
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// <summary>
    /// Set the RTPC to value on the actor.
    /// </summary>
    /// <param name="value">The new parameter value.</param>
    /// <param name="sync">When true, the request blocks the calling thread.</param>
    API_FUNCTION()
    void SetValue(float value, bool sync = false);

    /// <summary>
    /// Reset the RTPC to its middleware-defined default on the actor.
    /// </summary>
    /// <param name="sync">When true, the request blocks the calling thread.</param>
    API_FUNCTION()
    void ResetValue(bool sync = false);

    /// <returns>The last value that was successfully submitted via SetValue().</returns>
    API_FUNCTION()
    float GetValue() const;

  private:
    /// <summary>
    /// Resolved ID for RtpcName. INVALID_AUDIO_SYSTEM_ID if not found.
    /// </summary>
    AudioSystemDataID _rtpcId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// Last value submitted to the audio system.
    /// </summary>
    float _currentValue = 0.0f;
};
