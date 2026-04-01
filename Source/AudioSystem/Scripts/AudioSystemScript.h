#pragma once

#include <Engine/Scripting/Script.h>

#include "../Core/AudioSystemData.h"

// Forward declarations
class AudioProxyComponent;

// ============================================================================
//  AudioSystemComponent — abstract base for all audio scripts
//
//  Script-based audio components (trigger, RTPC, switch-state, animation)
//  derive from this class. Actor-based components (proxy, listener,
//  environment zones) derive directly from Actor or AudioSystemEnvironmentActor.
//  It is never instantiated directly.
// ============================================================================

/// \brief Abstract base class for all AudioSystem script components.
///
/// Derived classes must implement OnUpdate(). All audio scripts that need
/// per-frame processing inherit from this class.
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemComponent : public Script
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemComponent);

public:
    explicit AudioSystemComponent(const SpawnParams& params) : Script(params) {}

protected:
    /// Called every frame when the component is active. Must be implemented by subclasses.
    void OnUpdate() override = 0;
};

// ============================================================================
//  AudioSystemProxyDependentComponent — base for components that need an
//  AudioProxyComponent sibling on the same Actor.
//
//  OnEnable locates the sibling proxy automatically. If none is found the
//  component disables itself and logs a warning.
// ============================================================================

/// \brief Abstract base for audio components that require a sibling AudioProxyComponent.
///
/// Resolves the sibling proxy in OnEnable and releases the reference in OnDisable.
/// Subclasses can access the proxy via _proxy and the entity ID via GetEntityId().
API_CLASS(Abstract) class AUDIOSYSTEM_API AudioSystemProxyDependentComponent
    : public AudioSystemComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(AudioSystemProxyDependentComponent);

public:
    explicit AudioSystemProxyDependentComponent(const SpawnParams& params) : AudioSystemComponent(params) {}

    /// Called when this component becomes active.
    /// Locates the sibling AudioProxyComponent on the owner Actor.
    /// Disables itself (with a logged warning) if no proxy is found.
    void OnEnable() override;

    /// Called when this component becomes inactive.
    /// Clears the cached proxy pointer.
    void OnDisable() override;

protected:
    /// \return The AudioSystemDataID assigned by the sibling proxy.
    ///         Returns INVALID_AUDIO_SYSTEM_ID if no proxy has been resolved.
    AudioSystemDataID GetEntityId() const;

    /// Cached pointer to the sibling AudioProxyComponent resolved in OnEnable.
    AudioProxyComponent* _proxy = nullptr;
};
