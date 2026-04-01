#pragma once

#include "AudioSystemEnvironmentActor.h"

// ============================================================================
//  AudioSphereEnvironmentComponent
//
//  An environment zone defined by a sphere centred on this Actor's world-space
//  position.  Proxies inside Radius receive a full wet-send (1.0).  Proxies in
//  the falloff shell [Radius, MaxDistance] receive a linearly attenuated send.
//  Proxies beyond MaxDistance receive no send (0.0).
//
//  This class extends AudioSystemEnvironmentActor (an Actor subclass), so
//  GetPosition() is called directly on `this` and on the proxy — no Script
//  owner indirection is required.
// ============================================================================

/// \brief Sphere-shaped audio environment zone Actor.
///
/// The sphere is centred on this Actor's world-space position.  Proxies inside
/// Radius receive full send (1.0).  A linear falloff between Radius and
/// MaxDistance blends the wet-send from 1.0 to 0.0.
API_CLASS(Attributes="ActorContextMenu(\"New/Audio/Audio Sphere Environment\")")
class AUDIOSYSTEM_API AudioSphereEnvironmentComponent : public AudioSystemEnvironmentActor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioSphereEnvironmentComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Inner radius of the sphere zone (full wet-send inside this distance).
    API_FIELD(Attributes="EditorOrder(1), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Inner radius: full environment send inside this distance.\")")
    float Radius = 2.0f;

    /// Outer radius of the sphere zone (zero send beyond this distance).
    /// Must be greater than Radius. The falloff shell lies between Radius and MaxDistance.
    API_FIELD(Attributes="EditorOrder(2), Limit(0), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Outer radius: zero send beyond this distance. Must be greater than Radius.\")")
    float MaxDistance = 5.0f;

    // ========================================================================
    //  AudioSystemEnvironmentActor interface
    // ========================================================================

    /// \return 1.0 inside the inner sphere, linearly attenuated in the falloff
    ///         shell [Radius, MaxDistance], or 0.0 outside the outer sphere.
    /// \param proxy  The audio proxy whose world position is tested.
    float GetEnvironmentAmount(const AudioProxyComponent* proxy) const override;

    // ========================================================================
    //  Debug draw (editor only)
    // ========================================================================

#if USE_EDITOR
    void OnDebugDraw() override;
    void OnDebugDrawSelected() override;
#endif
};
