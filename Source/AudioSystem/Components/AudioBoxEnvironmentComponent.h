#pragma once

#include <Engine/Core/Math/Vector3.h>

#include "AudioSystemEnvironmentActor.h"

// ============================================================================
//  AudioBoxEnvironmentComponent
//
//  An environment zone defined by an axis-aligned box in the Actor's local
//  space. Proxies inside the box receive a full wet-send (1.0). Proxies
//  within MaxDistance of the box surface receive a linearly attenuated send.
//  Proxies beyond MaxDistance receive no send (0.0).
// ============================================================================

/// \brief Box-shaped audio environment zone Actor.
///
/// The zone is defined by HalfExtents in the Actor's local space.
/// A linear falloff over MaxDistance is applied outside the box boundary.
API_CLASS(Attributes="ActorContextMenu(\"New/Audio/Audio Box Environment\")")
class AUDIOSYSTEM_API AudioBoxEnvironmentComponent : public AudioSystemEnvironmentActor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioBoxEnvironmentComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Half-extents of the box in the Actor's local space.
    API_FIELD(Attributes="EditorOrder(1), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Half-extents of the box zone in local space.\")")
    Vector3 HalfExtents = Vector3(1.0f, 1.0f, 1.0f);

    /// Distance beyond the box surface over which the wet-send linearly fades
    /// from 1.0 (at the surface) to 0.0 (at MaxDistance).
    API_FIELD(Attributes="EditorOrder(2), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Falloff distance beyond the box surface.\")")
    float MaxDistance = 1.0f;

    // ========================================================================
    //  AudioSystemEnvironmentActor interface
    // ========================================================================

    /// \return 1.0 if proxy is inside the box, linearly attenuated in the
    ///         falloff shell, or 0.0 if beyond MaxDistance.
    float GetEnvironmentAmount(const AudioProxyComponent* proxy) const override;

    // ========================================================================
    //  Debug draw (editor only)
    // ========================================================================

#if USE_EDITOR
    void OnDebugDraw() override;
    void OnDebugDrawSelected() override;
#endif
};
