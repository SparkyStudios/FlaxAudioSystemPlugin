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

#include "AudioBoxEnvironmentActor.h"

#include <Engine/Core/Math/Math.h>
#include <Engine/Core/Math/Vector3.h>

#include "AudioProxyActor.h"

AudioBoxEnvironmentActor::AudioBoxEnvironmentActor(const SpawnParams& params)
    : AudioSystemEnvironmentActor(params)
{
}

// ============================================================================
//  GetEnvironmentAmount
// ============================================================================

float AudioBoxEnvironmentActor::GetEnvironmentAmount(const AudioProxyActor* proxy) const
{
    if (proxy == nullptr)
        return 0.0f;

    const Vector3 proxyWorldPos = proxy->GetPosition();

    // Transform the proxy world position into this Actor's local space so
    // the box test is always axis-aligned regardless of Actor rotation/scale.
    const Vector3 localPos = GetTransform().WorldToLocal(proxyWorldPos);
    const Vector3 absPos   = Vector3(Math::Abs(localPos.X), Math::Abs(localPos.Y), Math::Abs(localPos.Z));

    // Clamp MaxExtents so each axis is at least as large as HalfExtents.
    const Vector3 outerExtents = Vector3::Max(MaxExtents, HalfExtents);

    // Outside the outer box entirely: no send.
    if (absPos.X >= outerExtents.X || absPos.Y >= outerExtents.Y || absPos.Z >= outerExtents.Z)
        return 0.0f;

    // Inside the inner box: full send.
    if (absPos.X <= HalfExtents.X && absPos.Y <= HalfExtents.Y && absPos.Z <= HalfExtents.Z)
        return 1.0f;

    // Falloff zone: compute per-axis interpolation factor and take the minimum.
    // For each axis, the factor is 1.0 at HalfExtents and 0.0 at outerExtents.
    float minFactor = 1.0f;

    for (int32 i = 0; i < 3; ++i)
    {
        const float inner = (&HalfExtents.X)[i];
        const float outer = (&outerExtents.X)[i];
        const float pos   = (&absPos.X)[i];

        if (pos > inner)
        {
            const float range = outer - inner;
            if (range <= 0.0f)
            {
                minFactor = 0.0f;
                break;
            }
            const float factor = 1.0f - ((pos - inner) / range);
            minFactor          = Math::Min(minFactor, factor);
        }
    }

    return Math::Max(minFactor, 0.0f);
}

// ============================================================================
//  Debug draw (editor only)
// ============================================================================

#if USE_EDITOR
#include <Engine/Core/Math/Color.h>
#include <Engine/Core/Math/OrientedBoundingBox.h>
#include <Engine/Debug/DebugDraw.h>

static constexpr float WiresDimAlpha = 0.35f;

void AudioBoxEnvironmentActor::OnDebugDraw()
{
    const Color   dimColor     = EnvironmentColor.AlphaMultiplied(WiresDimAlpha);
    const Vector3 outerExtents = Vector3::Max(MaxExtents, HalfExtents);

    // Inner box (full send boundary).
    OrientedBoundingBox innerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, HalfExtents * 2.0f, innerBox);
    innerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(innerBox, dimColor, 0, true);

    // Outer box (zero send boundary).
    OrientedBoundingBox outerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, outerExtents * 2.0f, outerBox);
    outerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(outerBox, dimColor, 0, true);

    Actor::OnDebugDraw();
}

void AudioBoxEnvironmentActor::OnDebugDrawSelected()
{
    const Vector3 outerExtents = Vector3::Max(MaxExtents, HalfExtents);

    OrientedBoundingBox innerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, HalfExtents * 2.0f, innerBox);
    innerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(innerBox, EnvironmentColor, 0, false);

    OrientedBoundingBox outerBox;
    OrientedBoundingBox::CreateCentered(Vector3::Zero, outerExtents * 2.0f, outerBox);
    outerBox.Transform(GetTransform());
    DEBUG_DRAW_WIRE_BOX(outerBox, EnvironmentColor, 0, false);

    Actor::OnDebugDrawSelected();
}

#endif
