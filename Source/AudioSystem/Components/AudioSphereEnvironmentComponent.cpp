#include <Engine/Core/Math/Math.h>
#include <Engine/Core/Math/Vector3.h>

#include "AudioSphereEnvironmentComponent.h"
#include "AudioProxyComponent.h"

AudioSphereEnvironmentComponent::AudioSphereEnvironmentComponent(const SpawnParams& params)
    : AudioSystemEnvironmentActor(params)
{
}

// ============================================================================
//  GetEnvironmentAmount
// ============================================================================

float AudioSphereEnvironmentComponent::GetEnvironmentAmount(const AudioProxyComponent* proxy) const
{
    if (proxy == nullptr)
        return 0.0f;

    // Both this and the proxy are Actors — call GetPosition() directly.
    const Vector3 ownerPos = GetPosition();
    const Vector3 proxyPos = proxy->GetPosition();
    const float   dist     = Vector3::Distance(ownerPos, proxyPos);

    // Outside the sphere entirely: no send.
    if (Radius <= 0.0f || dist > Radius)
        return 0.0f;

    // Clamp MaxDistance to a sane range so innerRadius is non-negative.
    const float falloffWidth = Math::Max(MaxDistance, 0.0f);
    const float innerRadius  = Radius - falloffWidth;

    // Inside the inner (fully wet) sphere: full send.
    if (dist <= innerRadius)
        return 1.0f;

    // Degenerate falloff — treat entire sphere as a hard boundary.
    if (falloffWidth <= 0.0f)
        return 0.0f;

    // Falloff shell [innerRadius, Radius]: linear fade from 1 to 0.
    return 1.0f - ((dist - innerRadius) / falloffWidth);
}
