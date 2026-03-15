#pragma once

#include <Engine/Animations/AnimEvent.h>
#include <Engine/Core/Collections/Dictionary.h>
#include <Engine/Core/Types/String.h>

#include "AudioSystemComponent.h"

// Forward declarations
class AnimatedModel;
class Animation;
class AudioTriggerComponent;

// ============================================================================
//  AudioAnimEvent
//
//  A Flax AnimEvent subclass that bridges animation timeline events to
//  AudioTriggerComponent playback.
//
//  Place one AudioAnimEvent instance on an animation track.  Set TriggerName
//  to the PlayTriggerName of the sibling AudioTriggerComponent you want to
//  play.  When the animation reaches this event the engine calls OnEvent(),
//  which walks the Actor's scripts, finds a matching AudioTriggerComponent,
//  and calls Play() on it.
// ============================================================================

/// \brief Animation event that plays a named AudioTriggerComponent.
///
/// Attach to an Animation asset event track. Set TriggerName to match the
/// PlayTriggerName on a sibling AudioTriggerComponent on the AnimatedModel's Actor.
API_CLASS() class AUDIOSYSTEM_API AudioAnimEvent : public AnimEvent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioAnimEvent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// The PlayTriggerName of the sibling AudioTriggerComponent to play
    /// when this animation event fires.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"PlayTriggerName of the AudioTriggerComponent to play when this event fires.\")")
    String TriggerName;

    // ========================================================================
    //  AnimEvent interface
    // ========================================================================

    /// Called by the AnimGraph when this event's timeline position is reached.
    /// Finds a sibling AudioTriggerComponent on \p actor whose PlayTriggerName
    /// matches TriggerName and calls Play() on it.
    API_FUNCTION() void OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime) override;
};

// ============================================================================
//  AudioAnimationComponent
//
//  Optional component that provides a centralised EventTriggerMap dictionary
//  as an alternative to placing individual AudioAnimEvent instances directly
//  on animation assets.
//
//  When AnimatedModel fires a named event (via AudioAnimEvent), this component
//  can act as a dispatcher: look up the event name in EventTriggerMap, find
//  the matching sibling AudioTriggerComponent by PlayTriggerName, and call
//  Play().
//
//  NOTE: This component supports the dispatch path only. Events must still
//  originate from AudioAnimEvent instances placed on the animation asset.
//  The EventTriggerMap allows remapping event names without editing assets.
// ============================================================================

/// \brief Centralised dispatcher that maps animation event names to AudioTriggerComponent play calls.
///
/// Requires a sibling AudioProxyComponent. Attach AudioAnimEvent instances to
/// your animation assets and set their TriggerName to a key in EventTriggerMap.
/// EventTriggerMap then maps that key to the PlayTriggerName of the sibling
/// AudioTriggerComponent that should be played.
///
/// If EventTriggerMap is empty, AudioAnimEvent still works directly using
/// TriggerName as the AudioTriggerComponent PlayTriggerName.
API_CLASS() class AUDIOSYSTEM_API AudioAnimationComponent
    : public AudioSystemProxyDependentComponent
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioAnimationComponent);

public:
    // ========================================================================
    //  Serialised properties
    // ========================================================================

    /// Optional remapping table: animation event name → AudioTriggerComponent
    /// PlayTriggerName.  When empty, AudioAnimEvent uses its TriggerName directly.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Maps animation event name to the PlayTriggerName of a sibling AudioTriggerComponent. Leave empty to use AudioAnimEvent.TriggerName directly.\")")
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
    /// mapping is found.  Finds the matching sibling AudioTriggerComponent
    /// and calls Play() on it.
    ///
    /// This is called by AudioAnimEvent::OnEvent() when it finds this component
    /// on the same Actor.
    API_FUNCTION() void DispatchEvent(const StringView& eventName);

private:
    /// Cached pointer to the owner Actor's AnimatedModel (resolved in OnEnable).
    AnimatedModel* _animatedModel = nullptr;
};
