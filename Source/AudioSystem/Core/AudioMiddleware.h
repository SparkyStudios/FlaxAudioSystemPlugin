// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

/// <summary>
/// Abstract backend interface that decouples the AudioSystem from any
///        specific audio middleware (Wwise, FMOD, OpenAL, …).
///
/// A concrete middleware plugin subclasses this, overrides every pure-virtual
/// method, and registers an instance with the AudioSystem at startup.
/// The AudioSystem calls methods on this interface exclusively — it never
/// touches middleware-specific APIs directly.
/// </summary>
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

    /// <summary>
    /// Load the middleware configuration from a Flax JSON asset.
    /// </summary>
    /// <param name="configFile">Virtual or absolute path to the JSON configuration asset.</param>
    /// <returns>true on success; false if the asset is missing or malformed.</returns>
    virtual bool LoadConfiguration(StringView configFile) = 0;

    /// <summary>
    /// Initialise the middleware engine. Call after LoadConfiguration succeeds.
    /// </summary>
    /// <returns>true if the backend started successfully.</returns>
    virtual bool Startup() = 0;

    /// <summary>
    /// Advance the middleware audio engine by one frame.
    /// </summary>
    /// <param name="dt">Elapsed time in seconds since the last Update call.</param>
    virtual void Update(float dt) = 0;

    /// <summary>
    /// Pause all middleware processing and release runtime audio resources.
    /// Pair with Release() to fully tear down the backend.
    /// </summary>
    virtual void Shutdown() = 0;

    /// <summary>
    /// Fully release all middleware resources. Call after Shutdown().
    /// </summary>
    virtual void Release() = 0;

    /// <summary>
    /// Immediately stop every currently-playing sound without unloading data.
    /// </summary>
    virtual void StopAllSounds() = 0;

#if USE_EDITOR
    /// <summary>
    /// Deploy middleware-specific files (sound banks, config, etc.) to the
    /// cooked build output directory. Called by the editor build hook during
    /// the GameCooker deploy phase.
    ///
    /// The middleware implementation is responsible for knowing which files
    /// to copy and where they should go within the output tree.
    /// </summary>
    /// <param name="outputPath">Absolute path to the cooked output root folder.</param>
    /// <returns>true if deployment succeeded; false on error.</returns>
    virtual bool DeployFiles(const StringView& outputPath) = 0;
#endif

    // ========================================================================
    //  Entity
    //
    //  Audio entities map 1-to-1 to game objects in the middleware.
    //  The "world entity" is a special global entity used for non-spatial
    //  (2-D) sounds that are not attached to any scene object.
    // ========================================================================

    /// <summary>
    /// Create the global "world" entity used for non-spatial sounds.
    /// </summary>
    /// <param name="id">Unique identifier to assign to the world entity.</param>
    /// <returns>Newly allocated entity data, or nullptr on failure. Caller owns the pointer and must eventually pass it to DestroyEntityData().</returns>
    virtual AudioSystemEntityData* CreateWorldEntity(AudioSystemDataID id) = 0;

    /// <summary>
    /// Create middleware data for a new spatial audio entity.
    /// </summary>
    /// <param name="id">Unique identifier for the entity.</param>
    /// <returns>Newly allocated entity data, or nullptr on failure.</returns>
    virtual AudioSystemEntityData* CreateEntityData(AudioSystemDataID id) = 0;

    /// <summary>
    /// Release middleware data previously created by CreateEntityData or
    /// CreateWorldEntity.
    /// </summary>
    virtual void DestroyEntityData(AudioSystemEntityData* data) = 0;

    /// <summary>
    /// Register an entity with the middleware so it can receive audio requests.
    /// </summary>
    /// <returns>true if the entity was registered successfully.</returns>
    virtual bool AddEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// <summary>
    /// Reset an entity to its default state without unregistering it.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool ResetEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// <summary>
    /// Notify the middleware that an entity's properties (other than transform)
    /// may have changed and should be re-evaluated.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool UpdateEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// <summary>
    /// Unregister an entity from the middleware. Does not free data.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool RemoveEntity(AudioSystemDataID id, AudioSystemEntityData* data) = 0;

    /// <summary>
    /// Push an updated world-space transform to the middleware for an entity.
    /// </summary>
    /// <param name="transform">Position, velocity, forward, and up vectors.</param>
    /// <returns>true on success.</returns>
    virtual bool SetEntityTransform(AudioSystemDataID id, AudioSystemEntityData* data, const AudioSystemTransform& transform) = 0;

    // ========================================================================
    //  Listener
    //
    //  Listeners represent the "ears" in the scene (typically the camera or
    //  the player character). Most middleware supports multiple simultaneous
    //  listeners.
    // ========================================================================

    /// <summary>
    /// Create middleware data for a new audio listener.
    /// </summary>
    /// <param name="id">Unique identifier for the listener.</param>
    /// <param name="isDefault">Whether the listener is the default listener.</param>
    /// <returns>Newly allocated listener data, or nullptr on failure.</returns>
    virtual AudioSystemListenerData* CreateListenerData(AudioSystemDataID id, bool isDefault) = 0;

    /// <summary>
    /// Release middleware data previously created by CreateListenerData.
    /// </summary>
    virtual void DestroyListenerData(AudioSystemListenerData* data) = 0;

    /// <summary>
    /// Register a listener with the middleware.
    /// </summary>
    /// <returns>true if the listener was registered successfully.</returns>
    virtual bool AddListener(AudioSystemDataID id, AudioSystemListenerData* data) = 0;

    /// <summary>
    /// Reset a listener to its default state without unregistering it.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool ResetListener(AudioSystemDataID id, AudioSystemListenerData* data) = 0;

    /// <summary>
    /// Unregister a listener from the middleware. Does not free data.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool RemoveListener(AudioSystemDataID id, AudioSystemListenerData* data) = 0;

    /// <summary>
    /// Push an updated world-space transform to the middleware for a listener.
    /// </summary>
    /// <param name="transform">Position, velocity, forward, and up vectors.</param>
    /// <returns>true on success.</returns>
    virtual bool SetListenerTransform(AudioSystemDataID id, AudioSystemListenerData* data, const AudioSystemTransform& transform) = 0;

    // ========================================================================
    //  Event
    //
    //  Event data objects represent a single running instance of a trigger
    //  inside the middleware (e.g. a playing Wwise event or FMOD instance).
    // ========================================================================

    /// <summary>
    /// Allocate a new event instance for the given logical ID.
    /// </summary>
    /// <returns>Newly allocated event data, or nullptr on failure.</returns>
    virtual AudioSystemEventData* CreateEventData(AudioSystemDataID id) = 0;

    /// <summary>
    /// Reset an event instance back to its initial state (e.g. after stopping).
    /// </summary>
    virtual void ResetEventData(AudioSystemEventData* data) = 0;

    /// <summary>
    /// Release event instance data previously created by CreateEventData.
    /// </summary>
    virtual void DestroyEventData(AudioSystemEventData* data) = 0;

    // ========================================================================
    //  Trigger
    //
    //  Triggers map to middleware events / cues. Loading prepares the data;
    //  activating starts playback; unloading releases resources.
    // ========================================================================

    /// <summary>
    /// Prepare the trigger's audio data for playback (pre-load / bank prime).
    /// </summary>
    /// <returns>true if the load request was accepted.</returns>
    virtual bool LoadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) = 0;

    /// <summary>
    /// Start playing the trigger on the specified entity.
    /// </summary>
    /// <returns>true if the activation request was accepted.</returns>
    virtual bool ActivateTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) = 0;

    /// <summary>
    /// Release audio data loaded by LoadTrigger.
    /// </summary>
    /// <returns>true if the unload request was accepted.</returns>
    virtual bool UnloadTrigger(AudioSystemDataID entityId, AudioSystemTriggerData* triggerData, AudioSystemEventData* eventData) = 0;

    /// <summary>
    /// Stop a specific running event instance on the given entity.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool StopEvent(AudioSystemDataID entityId, AudioSystemEventData* eventData) = 0;

    /// <summary>
    /// Stop all running events on the given entity.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool StopAllEvents(AudioSystemDataID entityId) = 0;

    // ========================================================================
    //  RTPC (Real-Time Parameter Control)
    //
    //  RTPCs are named floating-point parameters that drive middleware
    //  behaviours (e.g. music intensity, character speed).
    // ========================================================================

    /// <summary>
    /// Set an RTPC to a specific value on the given entity.
    /// </summary>
    /// <param name="value">New parameter value (range is middleware-defined).</param>
    /// <returns>true on success.</returns>
    virtual bool SetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data, float value) = 0;

    /// <summary>
    /// Reset an RTPC to its default (middleware-defined) value on the entity.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool ResetRtpc(AudioSystemDataID entityId, AudioSystemRtpcData* data) = 0;

    // ========================================================================
    //  Switch / State
    //
    //  Switches (or "states" in some middleware) are named discrete values
    //  that select between different audio behaviours or variations.
    // ========================================================================

    /// <summary>
    /// Apply a switch-state value to the given entity.
    /// </summary>
    /// <returns>true on success.</returns>
    virtual bool SetSwitchState(AudioSystemDataID entityId, AudioSystemSwitchStateData* data) = 0;

    // ========================================================================
    //  Obstruction & Occlusion
    //
    //  Provides pre-computed obstruction (partial line-of-sight blockage) and
    //  occlusion (full blockage through geometry) values to the middleware so
    //  it can attenuate or filter sounds accordingly.
    // ========================================================================

    /// <summary>
    /// Supply obstruction and occlusion coefficients for an entity.
    /// </summary>
    /// <param name="obstruction">Partial-blockage factor [0.0, 1.0].</param>
    /// <param name="occlusion">Full-blockage factor [0.0, 1.0].</param>
    /// <returns>true on success.</returns>
    virtual bool SetObstructionAndOcclusion(AudioSystemDataID entityId, AudioSystemEntityData* entityData, float obstruction, float occlusion) = 0;

    // ========================================================================
    //  Environment
    //
    //  Environments (aux buses, reverb sends) model acoustic spaces.
    //  An entity can contribute to multiple environments simultaneously,
    //  each with its own send amount.
    // ========================================================================

    /// <summary>
    /// Set how strongly an entity contributes to a given environment.
    /// </summary>
    /// <param name="amount">Contribution factor [0.0, 1.0].</param>
    /// <returns>true on success.</returns>
    virtual bool SetEnvironmentAmount(AudioSystemDataID entityId, AudioSystemEnvironmentData* envData, float amount) = 0;

    // ========================================================================
    //  Bank
    //
    //  Sound banks bundle compressed audio assets. They must be loaded before
    //  any triggers or RTPCs that reference their assets can be used.
    // ========================================================================

    /// <summary>
    /// Load a sound bank into middleware memory.
    /// </summary>
    /// <returns>true if the bank was loaded successfully.</returns>
    virtual bool LoadBank(AudioSystemBankData* data) = 0;

    /// <summary>
    /// Unload a previously loaded sound bank from middleware memory.
    /// </summary>
    /// <returns>true if the bank was unloaded successfully.</returns>
    virtual bool UnloadBank(AudioSystemBankData* data) = 0;

    /// <summary>
    /// Release bank descriptor data previously created by the middleware.
    /// </summary>
    virtual void DestroyBank(AudioSystemBankData* data) = 0;

    // ========================================================================
    //  Control Data Destroy
    //
    //  Paired destroy calls for each control-descriptor type. Must be called
    //  after the control has been deregistered from all entities.
    // ========================================================================

    /// <summary>
    /// Release trigger descriptor data.
    /// </summary>
    virtual void DestroyTriggerData(AudioSystemTriggerData* data) = 0;

    /// <summary>
    /// Release RTPC descriptor data.
    /// </summary>
    virtual void DestroyRtpcData(AudioSystemRtpcData* data) = 0;

    /// <summary>
    /// Release switch-state descriptor data.
    /// </summary>
    virtual void DestroySwitchStateData(AudioSystemSwitchStateData* data) = 0;

    /// <summary>
    /// Release environment descriptor data.
    /// </summary>
    virtual void DestroyEnvironmentData(AudioSystemEnvironmentData* data) = 0;

    // ========================================================================
    //  Global
    //
    //  Session-wide settings that affect the entire middleware instance.
    // ========================================================================

    /// <summary>
    /// Set the active voice/localisation language for middleware assets.
    /// </summary>
    virtual void SetLanguage(StringView language) = 0;

    /// <summary>
    /// Respond to a master-gain change issued by the AudioSystem.
    /// </summary>
    /// <param name="gain">New master gain value (range is implementation-defined).</param>
    virtual void OnMasterGainChange(float gain) = 0;

    /// <summary>
    /// Respond to a mute-state change issued by the AudioSystem.
    /// </summary>
    /// <param name="muted">true to mute all output; false to unmute.</param>
    virtual void OnMuteChange(bool muted) = 0;

    /// <summary>
    /// Called when the application loses focus (e.g. Alt-Tab on PC).
    /// Implementations should pause or duck audio as appropriate.
    /// </summary>
    virtual void OnLoseFocus() = 0;

    /// <summary>
    /// Called when the application regains focus.
    /// Implementations should restore the audio state set before OnLoseFocus.
    /// </summary>
    virtual void OnGainFocus() = 0;

    // ========================================================================
    //  Query
    //
    //  Read-only accessors for middleware state inspected by the AudioSystem.
    // ========================================================================

    /// <returns>The human-readable name of this middleware (e.g. "Amplitude", "Wwise", "FMOD").</returns>
    virtual StringView GetMiddlewareName() const = 0;

    /// <returns>The current master gain value.</returns>
    virtual float GetMasterGain() const = 0;

    /// <returns>true if the middleware output is currently muted.</returns>
    virtual bool GetMute() const = 0;
};
