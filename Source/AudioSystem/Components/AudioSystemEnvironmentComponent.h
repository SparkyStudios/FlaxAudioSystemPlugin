#pragma once

#include <Engine/Core/Types/String.h>

#include "AudioSystemComponent.h"
#include "../Core/AudioSystemData.h"

// Forward declarations
class AudioProxyComponent;

// ============================================================================
//  AudioSystemEnvironmentComponent
//
//  Abstract base for all audio environment zone components.
//  Environments are registered with the AudioWorldModule when enabled and
//  unregistered when disabled. They are NOT proxy-dependent — a single
//  environment zone can affect any number of proxies in the scene.
//
//  Subclasses must implement GetEnvironmentAmount() to describe how much a
//  given proxy is affected by the environment (range [0, 1]).
// ============================================================================

/// \brief Abstract base class for audio environment zone components.
///
/// Resolves the environment name to an ID on enable and registers itself
/// with the AudioWorldModule so that the per-frame update can push
/// wet-send amounts to nearby proxies.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemEnvironmentComponent
    : public AudioSystemComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemEnvironmentComponent);

public:
    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// Resolves the environment ID and registers with the AudioWorldModule.
    void OnEnable() override;

    /// Unregisters this component from the AudioWorldModule.
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// \return How much the given proxy is affected by this environment [0, 1].
    ///         Subclasses must implement the distance / overlap logic.
    virtual float GetEnvironmentAmount(const AudioProxyComponent* proxy) const = 0;

    /// \return The environment ID resolved from EnvironmentName.
    ///         Returns INVALID_AUDIO_SYSTEM_ID if not yet resolved or not found.
    AudioSystemDataID GetEnvironmentId() const;

protected:
    /// Name of the environment (aux bus / reverb send) as defined in the
    /// audio control collection.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Name of the environment (aux bus) as defined in the control collection.\")")
    String EnvironmentName;

    /// Resolved ID for EnvironmentName. INVALID_AUDIO_SYSTEM_ID if not found.
    AudioSystemDataID _environmentId = INVALID_AUDIO_SYSTEM_ID;
};
