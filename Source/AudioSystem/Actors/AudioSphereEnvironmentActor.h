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

#include "AudioSystemEnvironmentActor.h"

/// <summary>
/// Sphere-shaped audio environment zone Actor.
///
/// The sphere is centred on this Actor's world-space position. Proxies inside
/// Radius receive full send (1.0). A linear falloff between Radius and
/// MaxDistance blends the wet-send from 1.0 to 0.0.
/// </summary>
API_CLASS(Attributes = "ActorContextMenu(\"New/Audio System/Audio Sphere Environment\")")
class AUDIOSYSTEM_API AudioSphereEnvironmentActor : public AudioSystemEnvironmentActor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioSphereEnvironmentActor);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// Inner radius of the sphere zone (full wet-send inside this distance).
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Inner radius: full environment send inside this distance.\")")
    float Radius = 2.0f;

    /// <summary>
    /// Outer radius of the sphere zone (zero send beyond this distance).
    /// Must be greater than Radius. The falloff shell lies between Radius and MaxDistance.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(2), Limit(0), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Outer radius: zero send beyond this distance. Must be greater than Radius.\")")
    float MaxDistance = 5.0f;

    // ========================================================================
    //  AudioSystemEnvironmentActor interface
    // ========================================================================

    /// <param name="proxy">The audio proxy whose world position is tested.</param>
    /// <returns>1.0 inside the inner sphere, linearly attenuated in the falloff shell [Radius, MaxDistance], or 0.0 outside the outer sphere.</returns>
    float GetEnvironmentAmount(const AudioProxyActor* proxy) const override;

    // ========================================================================
    //  Debug draw (editor only)
    // ========================================================================

#if USE_EDITOR
    void OnDebugDraw() override;
    void OnDebugDrawSelected() override;
#endif
};
