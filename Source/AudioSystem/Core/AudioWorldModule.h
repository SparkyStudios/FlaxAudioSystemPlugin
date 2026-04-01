#pragma once

#include <Engine/Core/Collections/Array.h>

#include "../Components/AudioSystemEnvironmentActor.h"

// Forward declarations
class AudioListenerComponent;
class AudioProxyComponent;

// ============================================================================
//  AudioWorldModule
//
//  Scene-level audio state tracker. Maintains the list of active environment
//  components and the default listener used for occlusion ray casting.
//
//  Owned as a value member of AudioSystem; updated explicitly from
//  AudioSystem::UpdateSound() — it is NOT a Flax scene module.
// ============================================================================

/// \brief Tracks active audio environments and the default listener for a scene.
class AUDIOSYSTEM_API AudioWorldModule
{
public:
    // ========================================================================
    //  Per-frame update (called from AudioSystem::UpdateSound())
    // ========================================================================

    /// Called once per frame after the ATL has processed its requests.
    /// TODO (Phase 7): iterate _environments and update proxy environment amounts
    ///                 on each ATL audio object based on overlap with active zones.
    void Update();

    // ========================================================================
    //  Environment registration
    // ========================================================================

    /// Register an environment component as active.
    /// Logs a warning and does nothing if \p comp is null or already registered.
    void AddEnvironment(const AudioSystemEnvironmentActor* comp);

    /// Unregister an environment component.
    /// Logs a warning if \p comp is not currently registered.
    void RemoveEnvironment(const AudioSystemEnvironmentActor* comp);

    /// \return The list of currently active environment components (read-only).
    const Array<const AudioSystemEnvironmentActor*>& GetEnvironments() const;

    // ========================================================================
    //  Default listener
    // ========================================================================

    /// Set the default listener used for occlusion ray casting.
    /// Passing null clears the current default listener.
    void SetDefaultListener(const AudioListenerComponent* listener);

    /// \return The current default listener, or null if none is set.
    const AudioListenerComponent* GetDefaultListener() const;

    // ========================================================================
    //  Proxy registration
    // ========================================================================

    /// Register an active audio proxy for environment updates.
    void AddProxy(AudioProxyComponent* proxy);

    /// Unregister a proxy (e.g., on EndPlay).
    void RemoveProxy(AudioProxyComponent* proxy);

private:
    Array<const AudioSystemEnvironmentActor*> _environments;
    Array<AudioProxyComponent*> _proxies;
    const AudioListenerComponent* _defaultListener = nullptr;
};
