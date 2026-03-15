#include <Engine/Core/Log.h>
#include <Engine/Core/Math/Math.h>
#include <Engine/Core/Math/Vector3.h>
#include <Engine/Level/Actor.h>

#include "AudioSphereEnvironmentComponent.h"
#include "AudioProxyComponent.h"

AudioSphereEnvironmentComponent::AudioSphereEnvironmentComponent(const SpawnParams& params)
    : AudioSystemEnvironmentComponent(params)
{
}

// ============================================================================
//  OnUpdate
// ============================================================================

void AudioSphereEnvironmentComponent::OnUpdate()
{
    // Per-frame logic is handled by AudioWorldModule::Update().
    // Nothing to do here.
}

// ============================================================================
//  GetEnvironmentAmount
// ============================================================================

float AudioSphereEnvironmentComponent::GetEnvironmentAmount(const AudioProxyComponent* proxy) const
{
    if (proxy == nullptr)
        return 0.0f;

    Actor* owner = GetActor();
    if (owner == nullptr)
        return 0.0f;

    const Vector3 ownerPos = owner->GetPosition();
    const Vector3 proxyPos = proxy->GetActor()->GetPosition();
    const float dist       = Vector3::Distance(ownerPos, proxyPos);

    // Outside the sphere entirely: no send.
    if (Radius <= 0.0f || dist > Radius)
        return 0.0f;

    // Clamp MaxDistance to a sane range so the inner radius is non-negative.
    const float falloffWidth = Math::Max(MaxDistance, 0.0f);
    const float innerRadius  = Radius - falloffWidth;

    // Inside the inner (fully wet) radius: full send.
    if (dist <= innerRadius)
        return 1.0f;

    // Falloff shell [innerRadius, Radius]: linear fade.
    if (falloffWidth <= 0.0f)
        return 0.0f;

    return 1.0f - ((dist - innerRadius) / falloffWidth);
}
