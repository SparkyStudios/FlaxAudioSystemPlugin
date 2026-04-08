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

#include <Engine/Animations/AnimEvent.h>
#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/ISerializable.h>
#include <Engine/Core/Types/String.h>

#include "AudioSystemScript.h"

// Forward declarations
class AnimatedModel;
class Animation;
class AudioTriggerScript;

API_STRUCT()
struct AUDIOSYSTEM_API AudioAnimationTriggerMapping : public ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(AudioAnimationTriggerMapping);
    API_AUTO_SERIALIZATION();

    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"Animation event name to match.\")")
    String EventName;

    API_FIELD(Attributes = "EditorOrder(1), Tooltip(\"PlayTriggerName of the AudioTriggerScript to play.\")")
    String TriggerName;
};

/// <summary>
/// Animation event that plays a named AudioTriggerScript.
///
/// Attach to an Animation asset event track. Set TriggerName to match the
/// PlayTriggerName on a sibling AudioTriggerScript on the AnimatedModel's Actor.
/// </summary>
API_CLASS()
class AUDIOSYSTEM_API AudioAnimEvent : public AnimEvent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioAnimEvent);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// The PlayTriggerName of the sibling AudioTriggerScript to play
    /// when this animation event fires.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"PlayTriggerName of the AudioTriggerScript to play when this event fires.\")")
    String TriggerName;

    // ========================================================================
    //  AnimEvent interface
    // ========================================================================

    /// <summary>
    /// Called by the AnimGraph when this event's timeline position is reached.
    /// Finds a sibling AudioTriggerScript on actor whose PlayTriggerName
    /// matches TriggerName and calls Play() on it.
    /// </summary>
    API_FUNCTION()
    void OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;
};

/// <summary>
/// Centralised dispatcher that maps animation event names to AudioTriggerScript play calls.
///
/// Requires a sibling AudioProxyActor. Attach AudioAnimEvent instances to
/// your animation assets and set their TriggerName to a key in EventTriggerMappings.
/// EventTriggerMappings then maps that key to the PlayTriggerName of the sibling
/// AudioTriggerScript that should be played.
///
/// If EventTriggerMappings is empty, AudioAnimEvent still works directly using
/// TriggerName as the AudioTriggerScript PlayTriggerName.
/// </summary>
API_CLASS(Attributes = "FlaxEngine.Category(\"Audio System\")")
class AUDIOSYSTEM_API AudioAnimationScript : public AudioProxyDependentScript
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioAnimationScript);

  public:
    // ========================================================================
    //  Serialized properties
    // ========================================================================

    /// <summary>
    /// Optional remapping table: animation event name → AudioTriggerScript
    /// PlayTriggerName.  When empty, AudioAnimEvent uses its TriggerName directly.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(0), Tooltip(\"Optional event-to-trigger remapping entries. Leave empty to use AudioAnimEvent.TriggerName directly.\")")
    Array<AudioAnimationTriggerMapping> EventTriggerMappings;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// <summary>
    /// Resolves the sibling proxy (base class) and validates the AnimatedModel.
    /// </summary>
    void OnEnable() override;

    /// <summary>
    /// No per-frame work — events are dispatched by AudioAnimEvent::OnEvent().
    /// </summary>
    void OnUpdate() override;

    /// <summary>
    /// Releases the cached proxy reference (base class).
    /// </summary>
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// <summary>
    /// Dispatch an event by name.  Looks up eventName in EventTriggerMappings;
    /// falls back to using eventName directly as the PlayTriggerName if no
    /// mapping is found.  Finds the matching sibling AudioTriggerScript
    /// and calls Play() on it.
    ///
    /// This is called by AudioAnimEvent::OnEvent() when it finds this component
    /// on the same Actor.
    /// </summary>
    API_FUNCTION()
    void DispatchEvent(const StringView& eventName);

  private:
    /// <summary>
    /// Cached pointer to the owner Actor's AnimatedModel (resolved in OnEnable).
    /// </summary>
    AnimatedModel* _animatedModel = nullptr;
};
