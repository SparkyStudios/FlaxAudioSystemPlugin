#pragma once

#include <Engine/Core/Types/String.h>
#include <Engine/Core/Types/StringView.h>

#include "AudioSystemComponent.h"
#include "../Core/AudioSystemData.h"

// ============================================================================
//  AudioSwitchStateComponent
//
//  Activates a named switch state on the sibling AudioProxyComponent's entity.
//
//  On enable the configured SwitchStateName is applied automatically.
//  The active state can be changed at runtime via SetState().
// ============================================================================

/// \brief Activates a named switch state on a sibling AudioProxyComponent entity.
///
/// Requires a sibling AudioProxyComponent on the same Actor.
/// The switch state specified in SwitchStateName is applied when the component
/// becomes active.  Call SetState() to change the active state at runtime.
API_CLASS() class AUDIOSYSTEM_API AudioSwitchStateComponent : public AudioSystemProxyDependentComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSwitchStateComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// The switch state name to activate when this component becomes active.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"The switch state name to activate on init.\")")
    String SwitchStateName;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// Resolves the proxy and applies SwitchStateName if non-empty.
    void OnEnable() override;

    /// No per-frame work — switch states are set on-demand.
    void OnUpdate() override;

    /// Releases the proxy reference.
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// Resolve \p stateName to an ID and activate it on the entity.
    /// \param stateName  Name of the switch state to activate.
    /// \param sync       When true, the request blocks the calling thread.
    API_FUNCTION() void SetState(const StringView& stateName, bool sync = false);

    /// \return The name of the last switch state successfully submitted.
    API_FUNCTION() const String& GetState() const;

private:
    /// Name of the last switch state that was submitted to the audio system.
    String _currentStateName;
};
