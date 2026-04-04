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

#include <Engine/Core/Math/Vector3.h>
#include <Engine/Level/Actor.h>
#include <Engine/Scripting/ScriptingObjectReference.h>

#include "../Core/AudioSystemData.h"

/// <summary>
/// An Actor that acts as an audio listener in the scene.
///
/// Place this actor in the scene tree (typically as a child of a Camera) to
/// register a spatial listener with the audio middleware. The listener tracks
/// the world-space transform of either this Actor or separate override actors
/// for position and orientation, and propagates the result to the middleware
/// each frame.
/// </summary>
API_CLASS(Attributes = "ActorContextMenu(\"New/Audio/Audio Listener\")")
class AUDIOSYSTEM_API AudioListenerActor : public Actor
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCENE_OBJECT(AudioListenerActor);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// When true, registers this listener as the scene-wide default listener
    /// used by the obstruction/occlusion system.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"Registers this as the scene default listener.\")")
    bool IsDefault = false;

    /// <summary>
    /// Optional actor to use as the source of position data.
    /// If null, this Actor's position is used instead.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(1), Tooltip(\"Override actor for position data. Uses this Actor if null.\")")
    ScriptingObjectReference<Actor> PositionObject;

    /// <summary>
    /// Optional actor to use as the source of orientation data.
    /// If null, this Actor's orientation is used instead.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(2), Tooltip(\"Override actor for orientation data. Uses this Actor if null.\")")
    ScriptingObjectReference<Actor> OrientationObject;

    // ========================================================================
    //  Actor lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Registers the viewport icon with the scene rendering system.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// Unregisters the viewport icon from the scene rendering system.
    /// </summary>
    void OnDisable() override;

    /// <summary>
    /// Allocates a unique listener ID, registers this listener with the middleware,
    /// and binds the per-frame update delegate.
    /// If IsDefault is true, sets this as the scene default listener.
    /// </summary>
    void OnBeginPlay() override;

    /// <summary>
    /// Unbinds the per-frame update delegate, unregisters this listener from the
    /// middleware, and clears the default listener reference if applicable.
    /// </summary>
    void OnEndPlay() override;

    /// <summary>
    /// Marks the transform dirty so OnFrameUpdate will flush the change to the
    /// middleware on the next frame.
    /// </summary>
    void OnTransformChanged() override;

    // ========================================================================
    //  Listener ID
    // ========================================================================

    /// <returns>The unique listener ID assigned in OnBeginPlay. Returns INVALID_AUDIO_SYSTEM_ID before OnBeginPlay has run.</returns>
    API_FUNCTION()
    uint64 GetListenerId() const;

  private:
    // ========================================================================
    //  Helpers
    // ========================================================================

    /// <summary>
    /// Called every frame via Scripting::Update. Computes the current transform
    /// and submits an UpdateListenerTransform request if it has changed.
    /// </summary>
    void OnFrameUpdate();

    /// <summary>
    /// Build an AudioSystemTransform using the configured position/orientation sources.
    /// </summary>
    AudioSystemTransform ComputeCurrentTransform() const;

    // ========================================================================
    //  State
    // ========================================================================

    /// <summary>
    /// Unique listener ID allocated in OnBeginPlay; reset to INVALID_AUDIO_SYSTEM_ID on end-play.
    /// </summary>
    AudioSystemDataID _listenerId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// Transform captured at the end of the previous OnFrameUpdate call.
    /// </summary>
    AudioSystemTransform _lastTransform;

    /// <summary>
    /// Whether _lastTransform holds a valid previous-frame snapshot.
    /// </summary>
    bool _hasLastTransform = false;

    /// <summary>
    /// Set by OnTransformChanged; cleared after OnFrameUpdate flushes the change.
    /// </summary>
    bool _transformDirty = false;

    /// <summary>
    /// Whether this actor was registered as the default listener in OnBeginPlay.
    /// </summary>
    bool _registeredAsDefault = false;

    /// <summary>
    /// Source of monotonically increasing listener IDs. Listener IDs start at
    /// 2000 to avoid collisions with the entity ID space (starting at 1000).
    /// </summary>
    static AudioSystemDataID _nextListenerId;
};
