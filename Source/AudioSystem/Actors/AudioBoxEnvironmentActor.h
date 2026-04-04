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

#include <Engine/Core/Math/Vector3.h>

#include "AudioSystemEnvironmentActor.h"

/// <summary>
/// Box-shaped audio environment zone Actor.
///
/// The inner zone is defined by HalfExtents and the outer zone by MaxExtents,
/// both in the Actor's local space.  A per-axis linear falloff is applied
/// between the two boxes.
/// </summary>
API_CLASS(Attributes = "ActorContextMenu(\"New/Audio/Audio Box Environment\")")
class AUDIOSYSTEM_API AudioBoxEnvironmentActor : public AudioSystemEnvironmentActor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioBoxEnvironmentActor);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// Half-extents of the inner box in the Actor's local space (full wet-send inside).
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Inner half-extents: full environment send inside this box.\")")
    Vector3 HalfExtents = Vector3(1.0f, 1.0f, 1.0f);

    /// <summary>
    /// Half-extents of the outer box in the Actor's local space (zero send outside).
    /// Each axis must be >= the corresponding HalfExtents axis. The falloff zone
    /// lies between HalfExtents and MaxExtents.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(2), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Outer half-extents: zero send outside this box. Must be >= HalfExtents per axis.\")")
    Vector3 MaxExtents = Vector3(2.0f, 2.0f, 2.0f);

    // ========================================================================
    //  AudioSystemEnvironmentActor interface
    // ========================================================================

    /// <returns>
    /// 1.0 if proxy is inside HalfExtents, linearly attenuated in the falloff zone between HalfExtents and MaxExtents, or 0.0 outside.
    /// </returns>
    float GetEnvironmentAmount(const AudioProxyActor* proxy) const override;

    // ========================================================================
    //  Debug draw (editor only)
    // ========================================================================

#if USE_EDITOR
    void OnDebugDraw() override;
    void OnDebugDrawSelected() override;
#endif
};
