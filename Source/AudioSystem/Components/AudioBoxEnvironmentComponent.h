#pragma once

#include <Engine/Core/Math/Vector3.h>

#include "AudioSystemEnvironmentComponent.h"

// ============================================================================
//  AudioBoxEnvironmentComponent
//
//  An environment zone defined by an axis-aligned box in the owner Actor's
//  local space. Proxies inside the box receive a full wet-send (1.0). Proxies
//  within MaxDistance of the box surface receive a linearly attenuated send.
//  Proxies beyond MaxDistance receive no send (0.0).
// ============================================================================

/// \brief Box-shaped audio environment zone.
///
/// The zone is defined by HalfExtents in the owner Actor's local space.
/// A linear falloff over MaxDistance is applied outside the box boundary.
API_CLASS() class AUDIOSYSTEM_API AudioBoxEnvironmentComponent
    : public AudioSystemEnvironmentComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioBoxEnvironmentComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Half-extents of the box in the owner Actor's local space.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Half-extents of the box zone in local Actor space.\")")
    Vector3 HalfExtents = Vector3(1.0f, 1.0f, 1.0f);

    /// Distance beyond the box surface over which the wet-send linearly fades
    /// from 1.0 (at the surface) to 0.0 (at MaxDistance).
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Falloff distance beyond the box surface.\")")
    float MaxDistance = 1.0f;

    // ========================================================================
    //  Script lifecycle override
    // ========================================================================

    /// No per-frame work — amounts are computed on demand by GetEnvironmentAmount().
    void OnUpdate() override;

    // ========================================================================
    //  AudioSystemEnvironmentComponent interface
    // ========================================================================

    /// \return 1.0 if proxy is inside the box, linearly attenuated in the
    ///         falloff shell, or 0.0 if beyond MaxDistance.
    float GetEnvironmentAmount(const AudioProxyComponent* proxy) const override;
};
