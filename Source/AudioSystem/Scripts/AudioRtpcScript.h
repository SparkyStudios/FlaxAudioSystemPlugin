#pragma once

#include <Engine/Core/Types/String.h>

#include "AudioSystemScript.h"
#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioRtpcScript
//
//  Sets and resets a named Real-Time Parameter Control (RTPC) value on the
//  sibling AudioProxyActor's entity.
//
//  On enable the RTPC name is resolved to an ID and the initial value is
//  applied.  Values can be changed at any time via SetValue().  On disable
//  the RTPC is reset to its middleware-defined default.
// ============================================================================

/// \brief Drives a named RTPC on a sibling AudioProxyActor entity.
///
/// Requires a sibling AudioProxyActor on the same Actor.
/// The initial value is pushed when the component becomes active.
/// Calling ResetValue() or disabling the component restores the default.
API_CLASS() class AUDIOSYSTEM_API AudioRtpcScript : public AudioProxyDependentScript
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioRtpcScript);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Name of the RTPC as defined in the audio control collection.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Name of the RTPC as defined in the control collection.\")")
    String RtpcName;

    /// Value applied to the RTPC when this component becomes active.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Value applied on component enable.\")")
    float InitialValue = 0.0f;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// Resolves the proxy and RTPC ID, then applies the initial value.
    void OnEnable() override;

    /// No per-frame work — RTPC values are set on-demand.
    void OnUpdate() override;

    /// Resets the RTPC to its default and releases the proxy reference.
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// Set the RTPC to \p value on the entity.
    /// \param value  The new parameter value.
    /// \param sync   When true, the request blocks the calling thread.
    API_FUNCTION() void SetValue(float value, bool sync = false);

    /// Reset the RTPC to its middleware-defined default on the entity.
    /// \param sync  When true, the request blocks the calling thread.
    API_FUNCTION() void ResetValue(bool sync = false);

    /// \return The last value that was successfully submitted via SetValue().
    API_FUNCTION() float GetValue() const;

private:
    /// Resolved ID for RtpcName. INVALID_AUDIO_SYSTEM_ID if not found.
    AudioSystemDataID _rtpcId = INVALID_AUDIO_SYSTEM_ID;

    /// Last value submitted to the audio system.
    float _currentValue = 0.0f;
};
