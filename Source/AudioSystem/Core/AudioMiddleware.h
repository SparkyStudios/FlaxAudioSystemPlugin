#pragma once

#include <Engine/Core/Types/StringView.h>

#include "AudioSystemData.h"

// ============================================================================
//  AudioMiddleware
//
//  Pure-virtual contract that every audio backend plugin must implement.
//  Middleware implementations are loaded at runtime as native plugins and
//  are never exposed to Flax reflection — do NOT add API_CLASS here.
//
//  Configuration is loaded from a Flax JSON asset; pass its virtual path
//  (or absolute path) as a StringView to LoadConfiguration().
// ============================================================================

/// \brief Abstract backend interface that decouples the AudioSystem from any
///        specific audio middleware (Wwise, FMOD, OpenAL, …).
///
/// A concrete middleware plugin subclasses this, overrides every pure-virtual
/// method, and registers an instance with the AudioSystem at startup.
/// The AudioSystem calls methods on this interface exclusively — it never
/// touches middleware-specific APIs directly.
class AUDIOSYSTEM_API AudioMiddleware
{
public:
    virtual ~AudioMiddleware() = default;

    // ========================================================================
    //  Lifecycle
    //
    //  Ordered sequence: LoadConfiguration → Startup → (Update loop) →
    //  Shutdown → Release. StopAllSounds can be called at any point while
    //  the middleware is running.
    // ========================================================================

    /// Load the middleware configuration from a Flax JSON asset.
    /// \param configFile Virtual or absolute path to the JSON configuration asset.
    /// \return true on success; false if the asset is missing or malformed.
    virtual bool LoadConfiguration(StringView configFile) = 0;

    /// Initialise the middleware engine. Call after LoadConfiguration succeeds.
    /// \return true if the backend started successfully.
    virtual bool Startup() = 0;

    /// Advance the middleware audio engine by one frame.
    /// \param dt Elapsed time in seconds since the last Update call.
    virtual void Update(float dt) = 0;

    /// Pause all middleware processing and release runtime audio resources.
    /// Pair with Release() to fully tear down the backend.
    virtual void Shutdown() = 0;

    /// Fully release all middleware resources. Call after Shutdown().
    virtual void Release() = 0;

    /// Immediately stop every currently-playing sound without unloading data.
    virtual void StopAllSounds() = 0;

#if USE_EDITOR
    /// Deploy middleware-specific files (sound banks, config, etc.) to the
    /// cooked build output directory. Called by the editor build hook during
    /// the GameCooker deploy phase.
    ///
    /// The middleware implementation is responsible for knowing which files
    /// to copy and where they should go within the output tree.
    ///
    /// \param outputPath  Absolute path to the cooked output root folder.
    /// \return true if deployment succeeded; false on error.
    virtual bool DeployFiles(const StringView& outputPath) = 0;
#endif

    // ========================================================================
    //  Entity
    //
    //  Audio entities map 1-to-1 to game objects in the middleware.
    //  The "world entity" is a special global entity used for non-spatial
    //  (2-D) sounds that are not attached to any scene object.
    // ========================================================================

    /// Create the global "world" entity used for non-spatial sounds.
    /// \param id Unique identifier to assign to the world entity.
    /// \return Newly allocated entity data, or nullptr on failure. Caller owns
    ///         the pointer and must eventually pass it to DestroyEntityData().
    virtual AudioSystemEntityData* CreateWorldEntity(AudioSystemDataID id) = 0;

    /// Create middleware data for a new spatial audio entity.
    /// \param id Unique identifier for the entity.
    /// \return Newly allocated entity data, or nullptr on failure.
    virtual AudioSystemEntityData* CreateEntityData(AudioSystemDataID id) = 0;

    /// Release middleware data previously created by CreateEntityData or
    /// CreateWorldEntity.
    virtual void DestroyEntityData(AudioSystemEntityData* data) = 0;

    /// Register an entity with the middleware so it can receive audio requests.
    /// \return true if the entity was registered successfully.
    virtual bool AddEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// Reset an entity to its default state without unregistering it.
    /// \return true on success.
    virtual bool ResetEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// Notify the middleware that an entity's properties (other than transform)
    /// may have changed and should be re-evaluated.
    /// \return true on success.
    virtual bool UpdateEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// Unregister an entity from the middleware. Does not free \p data.
    /// \return true on success.
    virtual bool RemoveEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// Push an updated world-space transform to the middleware for an entity.
    /// \param transform Position, velocity, forward, and up vectors.
    /// \return true on success.
    virtual bool SetEntityTransform(AudioSystemDataID id, AudioSystemEntityData* data, const AudioSystemTransform& transform) = 0;

    // ========================================================================
    //  Listener
    //
    //  Listeners represent the "ears" in the scene (typically the camera or
    //  the player character). Most middleware supports multiple simultaneous
    //  listeners.
    // ========================================================================

    /// Create middleware data for a new audio listener.
    /// \param id Unique identifier for the listener.
    /// \return Newly allocated listener data, or nullptr on failure.
    virtual AudioSystemListenerData* CreateListenerData(AudioSystemDataID id) = 0;

    /// Release middleware data previously created by CreateListenerData.
    virtual void DestroyListenerData(AudioSystemListenerData* data) = 0;

    /// Register a listener with the middleware.
    /// \return true if the listener was registered successfully.
    virtual bool AddListener(AudioSystemDataID id, AudioSystemListenerData* data) = 0;

    /// Reset a listener to its default state without unregistering it.
    /// \return true on success.
    virtual bool ResetListener(AudioSystemDataID id, AudioSystemListenerData* data) = 0;

    /// Unregister a listener from the middleware. Does not free \p data.
    /// \return true on success.
    virtual bool RemoveListener(AudioSystemDataID id, AudioSystemListenerData* data) = 0;

    /// Push an updated world-space transform to the middleware for a listener.
    /// \param transform Position, velocity, forward, and up vectors.
    /// \return true on success.
    virtual bool SetListenerTransform(AudioSystemDataID id, AudioSystemListenerData* data, const AudioSystemTransform& transform) = 0;

    // ========================================================================
    //  Event
    //
    //  Event data objects represent a single running instance of a trigger
    //  inside the middleware (e.g. a playing Wwise event or FMOD instance).
    // ========================================================================

    /// Allocate a new event instance for the given logical ID.
    /// \return Newly allocated event data, or nullptr on failure.
    virtual AudioSystemEventData* CreateEventData(AudioSystemDataID id) = 0;

    /// Reset an event instance back to its initial state (e.g. after stopping).
    virtual void ResetEventData(AudioSystemEventData* data) = 0;

    /// Release event instance data previously created by CreateEventData.
    virtual void DestroyEventData(AudioSystemEventData* data) = 0;

    // ========================================================================
    //  Trigger
    //
    //  Triggers map to middleware events / cues. Loading prepares the data;
    //  activating starts playback; unloading releases resources.
    // ========================================================================

    /// Prepare the trigger's audio data for playback (pre-load / bank prime).
    /// \return true if the load request was accepted.
    virtual bool LoadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) = 0;

    /// Start playing the trigger on the specified entity.
    /// \return true if the activation request was accepted.
    virtual bool ActivateTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) = 0;

    /// Release audio data loaded by LoadTrigger.
    /// \return true if the unload request was accepted.
    virtual bool UnloadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) = 0;

    /// Stop a specific running event instance on the given entity.
    /// \return true on success.
    virtual bool StopEvent(AudioSystemDataID entityId, AudioSystemEventData* eventData) = 0;

    /// Stop all running events on the given entity.
    /// \return true on success.
    virtual bool StopAllEvents(AudioSystemDataID entityId) = 0;

    // ========================================================================
    //  RTPC (Real-Time Parameter Control)
    //
    //  RTPCs are named floating-point parameters that drive middleware
    //  behaviours (e.g. music intensity, character speed).
    // ========================================================================

    /// Set an RTPC to a specific value on the given entity.
    /// \param value New parameter value (range is middleware-defined).
    /// \return true on success.
    virtual bool SetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data, float value) = 0;

    /// Reset an RTPC to its default (middleware-defined) value on the entity.
    /// \return true on success.
    virtual bool ResetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data) = 0;

    // ========================================================================
    //  Switch / State
    //
    //  Switches (or "states" in some middleware) are named discrete values
    //  that select between different audio behaviours or variations.
    // ========================================================================

    /// Apply a switch-state value to the given entity.
    /// \return true on success.
    virtual bool SetSwitchState(AudioSystemDataID entityId, AudioSystemSwitchStateData* data) = 0;

    // ========================================================================
    //  Obstruction & Occlusion
    //
    //  Provides pre-computed obstruction (partial line-of-sight blockage) and
    //  occlusion (full blockage through geometry) values to the middleware so
    //  it can attenuate or filter sounds accordingly.
    // ========================================================================

    /// Supply obstruction and occlusion coefficients for an entity.
    /// \param obstruction Partial-blockage factor [0.0, 1.0].
    /// \param occlusion   Full-blockage factor [0.0, 1.0].
    /// \return true on success.
    virtual bool SetObstructionAndOcclusion(AudioSystemDataID entityId, AudioSystemEntityData* entityData, float obstruction, float occlusion) = 0;

    // ========================================================================
    //  Environment
    //
    //  Environments (aux buses, reverb sends) model acoustic spaces.
    //  An entity can contribute to multiple environments simultaneously,
    //  each with its own send amount.
    // ========================================================================

    /// Set how strongly an entity contributes to a given environment.
    /// \param amount Contribution factor [0.0, 1.0].
    /// \return true on success.
    virtual bool SetEnvironmentAmount(AudioSystemDataID entityId, AudioSystemEnvironmentData* envData, float amount) = 0;

    // ========================================================================
    //  Bank
    //
    //  Sound banks bundle compressed audio assets. They must be loaded before
    //  any triggers or RTPCs that reference their assets can be used.
    // ========================================================================

    /// Load a sound bank into middleware memory.
    /// \return true if the bank was loaded successfully.
    virtual bool LoadBank(AudioSystemBankData* data) = 0;

    /// Unload a previously loaded sound bank from middleware memory.
    /// \return true if the bank was unloaded successfully.
    virtual bool UnloadBank(AudioSystemBankData* data) = 0;

    /// Release bank descriptor data previously created by the middleware.
    virtual void DestroyBank(AudioSystemBankData* data) = 0;

    // ========================================================================
    //  Control Data Destroy
    //
    //  Paired destroy calls for each control-descriptor type. Must be called
    //  after the control has been deregistered from all entities.
    // ========================================================================

    /// Release trigger descriptor data.
    virtual void DestroyTriggerData(AudioSystemTriggerData* data) = 0;

    /// Release RTPC descriptor data.
    virtual void DestroyRtpcData(AudioSystemRtpcData* data) = 0;

    /// Release switch-state descriptor data.
    virtual void DestroySwitchStateData(AudioSystemSwitchStateData* data) = 0;

    /// Release environment descriptor data.
    virtual void DestroyEnvironmentData(AudioSystemEnvironmentData* data) = 0;

    // ========================================================================
    //  Global
    //
    //  Session-wide settings that affect the entire middleware instance.
    // ========================================================================

    /// Set the active voice/localisation language for middleware assets.
    virtual void SetLanguage(StringView language) = 0;

    /// Respond to a master-gain change issued by the AudioSystem.
    /// \param gain New master gain value (range is implementation-defined).
    virtual void OnMasterGainChange(float gain) = 0;

    /// Respond to a mute-state change issued by the AudioSystem.
    /// \param muted true to mute all output; false to unmute.
    virtual void OnMuteChange(bool muted) = 0;

    /// Called when the application loses focus (e.g. Alt-Tab on PC).
    /// Implementations should pause or duck audio as appropriate.
    virtual void OnLoseFocus() = 0;

    /// Called when the application regains focus.
    /// Implementations should restore the audio state set before OnLoseFocus.
    virtual void OnGainFocus() = 0;

    // ========================================================================
    //  Query
    //
    //  Read-only accessors for middleware state inspected by the AudioSystem.
    // ========================================================================

    /// \return The human-readable name of this middleware (e.g. "Wwise", "FMOD").
    virtual StringView GetMiddlewareName() const = 0;

    /// \return The current master gain value.
    virtual float GetMasterGain() const = 0;

    /// \return true if the middleware output is currently muted.
    virtual bool GetMute() const = 0;
};
