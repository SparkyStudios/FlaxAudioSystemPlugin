#pragma once

#include <Engine/Core/Math/Color.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Level/Actor.h>

#include "../Core/AudioSystemData.h"

// Forward declarations
class AudioProxyActor;

// ============================================================================
//  AudioSystemEnvironmentActor
//
//  Abstract base Actor for all audio environment zone actors.
//  Environments are registered with the AudioWorldModule on BeginPlay and
//  unregistered on EndPlay. They are NOT proxy-dependent — a single
//  environment zone can affect any number of proxies in the scene.
//
//  Subclasses must implement GetEnvironmentAmount() to describe how much a
//  given proxy is affected by the environment (range [0, 1]).
// ============================================================================

/// \brief Abstract base Actor class for audio environment zone actors.
///
/// Resolves the environment name to an ID on BeginPlay and registers itself
/// with the AudioWorldModule so that the per-frame update can push
/// wet-send amounts to nearby proxies.
API_CLASS(Abstract, Attributes="ActorContextMenu(\"New/Audio\")")
class AUDIOSYSTEM_API AudioSystemEnvironmentActor : public Actor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemEnvironmentActor);

public:
    explicit AudioSystemEnvironmentActor(const SpawnParams& params) : Actor(params) {}
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Name of the environment (aux bus / reverb send) as defined in the
    /// audio control collection.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Name of the environment (aux bus) as defined in the control collection.\")")
    String EnvironmentName;

    // ========================================================================
    //  Debug properties
    // ========================================================================

    /// Wireframe color used to visualize this environment zone in the editor viewport.
    API_FIELD(Attributes="EditorOrder(100), EditorDisplay(\"Debug\", \"Color\"), Tooltip(\"Wireframe color for the environment zone debug visualization.\")")
    Color EnvironmentColor = Color(0.0f, 0.8f, 1.0f, 1.0f);

    // ========================================================================
    //  Actor lifecycle overrides
    // ========================================================================

    /// Registers the viewport icon with the scene rendering system.
    void OnEnable() override;

    /// Unregisters the viewport icon from the scene rendering system.
    void OnDisable() override;

    /// Resolves the environment ID and registers with the AudioWorldModule.
    void OnBeginPlay() override;

    /// Unregisters this Actor from the AudioWorldModule.
    void OnEndPlay() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// \return How much the given proxy is affected by this environment [0, 1].
    ///         Subclasses must implement the distance / overlap logic.
    virtual float GetEnvironmentAmount(const AudioProxyActor* proxy) const = 0;

    /// \return The environment ID resolved from EnvironmentName.
    ///         Returns INVALID_AUDIO_SYSTEM_ID if not yet resolved or not found.
    AudioSystemDataID GetEnvironmentId() const;

    // ========================================================================
    //  Debug draw (editor only)
    // ========================================================================

#if USE_EDITOR
    /// Draws a dim wireframe of the environment zone every editor frame.
    void OnDebugDraw() override;

    /// Draws a full-color wireframe of the environment zone when selected.
    void OnDebugDrawSelected() override;
#endif

protected:
    /// Resolved ID for EnvironmentName. INVALID_AUDIO_SYSTEM_ID if not found.
    AudioSystemDataID _environmentId = INVALID_AUDIO_SYSTEM_ID;
};
