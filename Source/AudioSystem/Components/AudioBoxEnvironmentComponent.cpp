#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Math.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Level/Actor.h>

#include "AudioBoxEnvironmentComponent.h"
#include "AudioProxyComponent.h"

// ============================================================================
//  Scripting type registration
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE(AudioBoxEnvironmentComponent, AudioSystemEnvironmentComponent,
    "AudioSystem.AudioBoxEnvironmentComponent", nullptr, nullptr);

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioBoxEnvironmentComponent::OnUpdate()
{
    // Per-frame logic is handled by AudioWorldModule::Update().
    // Nothing to do here.
}

// ============================================================================
//  GetEnvironmentAmount
// ============================================================================

float AudioBoxEnvironmentComponent::GetEnvironmentAmount(const AudioProxyComponent* proxy) const
{
    if (proxy == nullptr)
        return 0.0f;

    Actor* owner = GetActor();
    if (owner == nullptr)
        return 0.0f;

    const Vector3 proxyWorldPos = proxy->GetActor()->GetPosition();

    // Transform the proxy world position into the owner Actor's local space so
    // the box test is always axis-aligned regardless of Actor rotation/scale.
    const Vector3 localPos = owner->GetTransform().WorldToLocal(proxyWorldPos);

    // Clamp local position to the box surface.
    const Vector3 clamped = Vector3::Clamp(localPos, -HalfExtents, HalfExtents);

    // Distance from the proxy's local position to the nearest point on the box.
    const float dist = Vector3::Distance(localPos, clamped);

    // Inside the box: full send.
    if (dist <= 0.0f)
        return 1.0f;

    // Beyond the falloff shell: no send.
    if (MaxDistance <= 0.0f || dist >= MaxDistance)
        return 0.0f;

    // Linear falloff from 1 (at box surface) to 0 (at MaxDistance).
    return 1.0f - (dist / MaxDistance);
}
