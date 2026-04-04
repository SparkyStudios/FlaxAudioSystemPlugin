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

#include "AudioSphereEnvironmentActor.h"

#include <Engine/Core/Math/Math.h>
#include <Engine/Core/Math/Vector3.h>

#include "AudioProxyActor.h"

AudioSphereEnvironmentActor::AudioSphereEnvironmentActor(const SpawnParams& params)
    : AudioSystemEnvironmentActor(params)
{
}

// ============================================================================
//  GetEnvironmentAmount
// ============================================================================

float AudioSphereEnvironmentActor::GetEnvironmentAmount(const AudioProxyActor* proxy) const
{
    if (proxy == nullptr)
        return 0.0f;

    const Vector3 ownerPos = GetPosition();
    const Vector3 proxyPos = proxy->GetPosition();
    const float   dist     = Vector3::Distance(ownerPos, proxyPos);

    // Clamp MaxDistance so the falloff width is never negative.
    const float outerRadius = Math::Max(MaxDistance, Radius);

    // Beyond the outer sphere: no send.
    if (outerRadius <= 0.0f || dist >= outerRadius)
        return 0.0f;

    // Inside the inner sphere: full send.
    if (dist <= Radius)
        return 1.0f;

    // Falloff shell [Radius, outerRadius]: linear fade from 1 to 0.
    const float falloffWidth = outerRadius - Radius;
    if (falloffWidth <= 0.0f)
        return 0.0f;

    return 1.0f - ((dist - Radius) / falloffWidth);
}

// ============================================================================
//  Debug draw (editor only)
// ============================================================================

#if USE_EDITOR
#include <Engine/Core/Math/BoundingSphere.h>
#include <Engine/Core/Math/Color.h>
#include <Engine/Debug/DebugDraw.h>

static constexpr float WiresDimAlpha = 0.35f;

void AudioSphereEnvironmentActor::OnDebugDraw()
{
    const Color   dimColor = EnvironmentColor.AlphaMultiplied(WiresDimAlpha);
    const Vector3 center   = GetPosition();

    // Inner sphere (full send boundary).
    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, Radius), dimColor, 0, true);
    // Outer sphere (zero send boundary).
    const float outerRadius = Math::Max(MaxDistance, Radius);
    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, outerRadius), dimColor, 0, true);

    Actor::OnDebugDraw();
}

void AudioSphereEnvironmentActor::OnDebugDrawSelected()
{
    const Vector3 center = GetPosition();

    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, Radius), EnvironmentColor, 0, false);
    const float outerRadius = Math::Max(MaxDistance, Radius);
    DEBUG_DRAW_WIRE_SPHERE(BoundingSphere(center, outerRadius), EnvironmentColor, 0, false);

    Actor::OnDebugDrawSelected();
}

#endif
