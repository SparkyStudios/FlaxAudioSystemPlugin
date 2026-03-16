#pragma once

#include "AudioSystemEnvironmentActor.h"

// ============================================================================
//  AudioSphereEnvironmentComponent
//
//  An environment zone defined by a sphere centred on this Actor's world-space
//  position.  Proxies whose distance from the centre is less than
//  (Radius - MaxDistance) receive a full wet-send (1.0).  Proxies in the
//  falloff shell [Radius - MaxDistance, Radius] receive a linearly attenuated
//  send.  Proxies beyond Radius receive no send (0.0).
//
//  This class extends AudioSystemEnvironmentActor (an Actor subclass), so
//  GetPosition() is called directly on `this` and on the proxy — no Script
//  owner indirection is required.
// ============================================================================

/// \brief Sphere-shaped audio environment zone Actor.
///
/// The sphere is centred on this Actor's world-space position.  A linear
/// falloff over MaxDistance is applied inside the outer rim of the sphere,
/// blending the wet-send from 1.0 at the inner boundary to 0.0 at Radius.
API_CLASS(Attributes="ActorContextMenu(\"New/Audio/Audio Sphere Environment\")")
class AUDIOSYSTEM_API AudioSphereEnvironmentComponent : public AudioSystemEnvironmentActor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioSphereEnvironmentComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Outer radius of the sphere zone in world units.
    /// Proxies beyond this distance receive no wet-send.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Outer radius of the sphere zone in world units.\")")
    float Radius = 5.0f;

    /// Width of the falloff shell just inside the outer radius.
    /// The wet-send transitions from 1.0 at (Radius - MaxDistance) to 0.0 at
    /// Radius.  Clamped to 0 at runtime so negative values are safe to author.
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Falloff distance inside the outer sphere radius.\")")
    float MaxDistance = 1.0f;

    // ========================================================================
    //  AudioSystemEnvironmentActor interface
    // ========================================================================

    /// \return 1.0 inside the inner sphere, linearly attenuated in the falloff
    ///         shell [innerRadius, Radius], or 0.0 outside the outer sphere.
    /// \param proxy  The audio proxy whose world position is tested.
    float GetEnvironmentAmount(const AudioProxyComponent* proxy) const override;
};
