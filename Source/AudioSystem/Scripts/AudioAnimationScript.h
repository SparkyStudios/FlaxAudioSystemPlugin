#pragma once

#include <Engine/Animations/AnimEvent.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Types/String.h>

#include "AudioSystemScript.h"

// Forward declarations
class AnimatedModel;
class Animation;
class AudioTriggerScript;

// ============================================================================
//  AudioAnimEvent
//
//  A Flax AnimEvent subclass that bridges animation timeline events to
//  AudioTriggerScript playback.
//
//  Place one AudioAnimEvent instance on an animation track.  Set TriggerName
//  to the PlayTriggerName of the sibling AudioTriggerScript you want to
//  play.  When the animation reaches this event the engine calls OnEvent(),
//  which walks the Actor's scripts, finds a matching AudioTriggerScript,
//  and calls Play() on it.
// ============================================================================

/// \brief Animation event that plays a named AudioTriggerScript.
///
/// Attach to an Animation asset event track. Set TriggerName to match the
/// PlayTriggerName on a sibling AudioTriggerScript on the AnimatedModel's Actor.
API_CLASS() class AUDIOSYSTEM_API AudioAnimEvent : public AnimEvent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioAnimEvent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// The PlayTriggerName of the sibling AudioTriggerScript to play
    /// when this animation event fires.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"PlayTriggerName of the AudioTriggerScript to play when this event fires.\")")
    String TriggerName;

    // ========================================================================
    //  AnimEvent interface
    // ========================================================================

    /// Called by the AnimGraph when this event's timeline position is reached.
    /// Finds a sibling AudioTriggerScript on \p actor whose PlayTriggerName
    /// matches TriggerName and calls Play() on it.
    API_FUNCTION() void OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;
};

// ============================================================================
//  AudioAnimationScript
//
//  Optional component that provides a centralised EventTriggerMap dictionary
//  as an alternative to placing individual AudioAnimEvent instances directly
//  on animation assets.
//
//  When AnimatedModel fires a named event (via AudioAnimEvent), this component
//  can act as a dispatcher: look up the event name in EventTriggerMap, find
//  the matching sibling AudioTriggerScript by PlayTriggerName, and call
//  Play().
//
//  NOTE: This component supports the dispatch path only. Events must still
//  originate from AudioAnimEvent instances placed on the animation asset.
//  The EventTriggerMap allows remapping event names without editing assets.
// ============================================================================

/// \brief Centralised dispatcher that maps animation event names to AudioTriggerScript play calls.
///
/// Requires a sibling AudioProxyComponent. Attach AudioAnimEvent instances to
/// your animation assets and set their TriggerName to a key in EventTriggerMap.
/// EventTriggerMap then maps that key to the PlayTriggerName of the sibling
/// AudioTriggerScript that should be played.
///
/// If EventTriggerMap is empty, AudioAnimEvent still works directly using
/// TriggerName as the AudioTriggerScript PlayTriggerName.
API_CLASS() class AUDIOSYSTEM_API AudioAnimationScript
    : public AudioProxyDependentScript
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioAnimationScript);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Optional remapping table: animation event name → AudioTriggerScript
    /// PlayTriggerName.  When empty, AudioAnimEvent uses its TriggerName directly.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Maps animation event name to the PlayTriggerName of a sibling AudioTriggerScript. Leave empty to use AudioAnimEvent.TriggerName directly.\")")
    Dictionary<String, String> EventTriggerMap;

    // ========================================================================
    //  Script lifecycle overrides
    // ========================================================================

    /// Resolves the sibling proxy (base class) and validates the AnimatedModel.
    void OnEnable() override;

    /// No per-frame work — events are dispatched by AudioAnimEvent::OnEvent().
    void OnUpdate() override;

    /// Releases the cached proxy reference (base class).
    void OnDisable() override;

    // ========================================================================
    //  Public API
    // ========================================================================

    /// Dispatch an event by name.  Looks up \p eventName in EventTriggerMap;
    /// falls back to using \p eventName directly as the PlayTriggerName if no
    /// mapping is found.  Finds the matching sibling AudioTriggerScript
    /// and calls Play() on it.
    ///
    /// This is called by AudioAnimEvent::OnEvent() when it finds this component
    /// on the same Actor.
    API_FUNCTION() void DispatchEvent(const StringView& eventName);

private:
    /// Cached pointer to the owner Actor's AnimatedModel (resolved in OnEnable).
    AnimatedModel* _animatedModel = nullptr;
};
