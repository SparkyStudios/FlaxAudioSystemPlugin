#pragma once

#include "AudioSystemEnvironmentComponent.h"

// ============================================================================
//  AudioSphereEnvironmentComponent
//
//  An environment zone defined by a sphere centred on the owner Actor.
//  Proxies whose distance from the Actor centre is less than
//  (Radius - MaxDistance) receive a full wet-send (1.0). Proxies in the
//  falloff shell [Radius - MaxDistance, Radius] receive a linearly
//  attenuated send. Proxies beyond Radius receive no send (0.0).
// ============================================================================

/// \brief Sphere-shaped audio environment zone.
///
/// The sphere is centred on the owner Actor's world-space position.
/// A linear falloff over MaxDistance is applied inside the outer rim of the sphere.
API_CLASS() class AUDIOSYSTEM_API AudioSphereEnvironmentComponent
    : public AudioSystemEnvironmentComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSphereEnvironmentComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Outer radius of the sphere zone in world units.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Outer radius of the sphere zone in world units.\")")
    float Radius = 5.0f;

    /// Width of the falloff shell just inside the outer radius.
    /// The wet-send transitions from 1.0 at (Radius - MaxDistance) to 0.0 at Radius.
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Falloff distance inside the outer sphere radius.\")")
    float MaxDistance = 1.0f;

    // ========================================================================
    //  Script lifecycle override
    // ========================================================================

    /// No per-frame work — amounts are computed on demand by GetEnvironmentAmount().
    void OnUpdate() override;

    // ========================================================================
    //  AudioSystemEnvironmentComponent interface
    // ========================================================================

    /// \return 1.0 inside the inner sphere, linearly attenuated in the falloff
    ///         shell, or 0.0 outside the outer sphere.
    float GetEnvironmentAmount(const AudioProxyComponent* proxy) const override;
};
