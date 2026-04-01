#pragma once

#include <Engine/Core/Math/Vector3.h>

#include "AudioSystemEnvironmentActor.h"

// ============================================================================
//  AudioBoxEnvironmentActor
//
//  An environment zone defined by two axis-aligned boxes in the Actor's local
//  space.  Proxies inside HalfExtents receive a full wet-send (1.0).  Proxies
//  between HalfExtents and MaxExtents receive a linearly attenuated send
//  (per-axis minimum).  Proxies outside MaxExtents receive no send (0.0).
// ============================================================================

/// \brief Box-shaped audio environment zone Actor.
///
/// The inner zone is defined by HalfExtents and the outer zone by MaxExtents,
/// both in the Actor's local space.  A per-axis linear falloff is applied
/// between the two boxes.
API_CLASS(Attributes="ActorContextMenu(\"New/Audio/Audio Box Environment\")")
class AUDIOSYSTEM_API AudioBoxEnvironmentActor : public AudioSystemEnvironmentActor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioBoxEnvironmentActor);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Half-extents of the inner box in the Actor's local space (full wet-send inside).
    API_FIELD(Attributes="EditorOrder(1), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Inner half-extents: full environment send inside this box.\")")
    Vector3 HalfExtents = Vector3(1.0f, 1.0f, 1.0f);

    /// Half-extents of the outer box in the Actor's local space (zero send outside).
    /// Each axis must be >= the corresponding HalfExtents axis. The falloff zone
    /// lies between HalfExtents and MaxExtents.
    API_FIELD(Attributes="EditorOrder(2), ValueCategory(Utils.ValueCategory.Distance), Tooltip(\"Outer half-extents: zero send outside this box. Must be >= HalfExtents per axis.\")")
    Vector3 MaxExtents = Vector3(2.0f, 2.0f, 2.0f);

    // ========================================================================
    //  AudioSystemEnvironmentActor interface
    // ========================================================================

    /// \return 1.0 if proxy is inside HalfExtents, linearly attenuated in the
    ///         falloff zone between HalfExtents and MaxExtents, or 0.0 outside.
    float GetEnvironmentAmount(const AudioProxyActor* proxy) const override;

    // ========================================================================
    //  Debug draw (editor only)
    // ========================================================================

#if USE_EDITOR
    void OnDebugDraw() override;
    void OnDebugDrawSelected() override;
#endif
};
