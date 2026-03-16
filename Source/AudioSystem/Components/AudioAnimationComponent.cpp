#include <Engine/Core/Log.h>
#include <Engine/Level/Actor.h>
#include <Engine/Level/Actors/AnimatedModel.h>
#include <Engine/Scripting/ScriptingObject.h>

#include "AudioAnimationComponent.h"
#include "AudioTriggerComponent.h"

AudioAnimEvent::AudioAnimEvent(const SpawnParams& params)
    : AnimEvent(params)
{
}

// ============================================================================
//  AudioAnimEvent — OnEvent
// ============================================================================

void AudioAnimEvent::OnEvent(AnimatedModel* actor, Animation* anim, float time, float deltaTime)
{
    if (actor == nullptr)
    {
        LOG(Warning, "[AudioAnimEvent] OnEvent: AnimatedModel actor is null.");
        return;
    }

    if (!TriggerName.HasChars())
    {
        LOG(Warning, "[AudioAnimEvent] OnEvent: TriggerName is empty; nothing to play.");
        return;
    }

    // Audio scripts (AudioAnimationComponent, AudioTriggerComponent) live on
    // the AudioProxyComponent Actor, which is the parent of the AnimatedModel.
    Actor* proxyActor = actor->GetParent();
    if (proxyActor == nullptr)
    {
        LOG(Warning, "[AudioAnimEvent] OnEvent: AnimatedModel '{0}' has no parent Actor.", actor->GetName());
        return;
    }

    // First, check whether there is an AudioAnimationComponent on the proxy
    // that can remap the trigger name via EventTriggerMap.
    AudioAnimationComponent* dispatcher = proxyActor->GetScript<AudioAnimationComponent>();
    if (dispatcher != nullptr)
    {
        dispatcher->DispatchEvent(TriggerName);
        return;
    }

    // No dispatcher component — use TriggerName directly as PlayTriggerName.
    const Array<Script*>& scripts = proxyActor->Scripts;
    for (Script* script : scripts)
    {
        AudioTriggerComponent* trigger = ScriptingObject::Cast<AudioTriggerComponent>(script);
        if (trigger == nullptr)
            continue;

        if (trigger->PlayTriggerName == TriggerName)
        {
            trigger->Play();
            return;
        }
    }

    LOG(Warning, "[AudioAnimEvent] OnEvent: No AudioTriggerComponent with PlayTriggerName '{0}' found on proxy Actor '{1}'.",
        TriggerName, proxyActor->GetName());
}

// ============================================================================
//  AudioAnimationComponent — OnEnable
// ============================================================================

AudioAnimationComponent::AudioAnimationComponent(const SpawnParams& params)
    : AudioSystemProxyDependentComponent(params)
{
}

void AudioAnimationComponent::OnEnable()
{
    // Resolve the sibling proxy (base class handles null/missing proxy).
    AudioSystemProxyDependentComponent::OnEnable();

    if (_proxy == nullptr)
        return;

    Actor* owner = GetActor();
    if (owner == nullptr)
        return;

    _animatedModel = owner->GetChild<AnimatedModel>();
    if (_animatedModel == nullptr)
    {
        LOG(Warning, "[AudioAnimationComponent] OnEnable: No AnimatedModel found on Actor '{0}'. Animation events will not fire.",
            owner->GetName());
    }
}

// ============================================================================
//  AudioAnimationComponent — OnUpdate
// ============================================================================

void AudioAnimationComponent::OnUpdate()
{
    // Events are dispatched by AudioAnimEvent::OnEvent() — nothing to do here.
}

// ============================================================================
//  AudioAnimationComponent — OnDisable
// ============================================================================

void AudioAnimationComponent::OnDisable()
{
    _animatedModel = nullptr;
    AudioSystemProxyDependentComponent::OnDisable();
}

// ============================================================================
//  AudioAnimationComponent — DispatchEvent
// ============================================================================

void AudioAnimationComponent::DispatchEvent(const StringView& eventName)
{
    if (eventName.IsEmpty())
    {
        LOG(Warning, "[AudioAnimationComponent] DispatchEvent: eventName is empty.");
        return;
    }

    Actor* owner = GetActor();
    if (owner == nullptr)
        return;

    // Resolve the play trigger name: check EventTriggerMap first, then fall
    // back to using the event name directly.
    String resolvedTriggerName(eventName);
    const String* mapped = EventTriggerMap.TryGet(String(eventName));
    if (mapped != nullptr && mapped->HasChars())
        resolvedTriggerName = *mapped;

    // Find the sibling AudioTriggerComponent with the matching PlayTriggerName.
    const Array<Script*>& scripts = owner->Scripts;
    for (Script* script : scripts)
    {
        AudioTriggerComponent* trigger = ScriptingObject::Cast<AudioTriggerComponent>(script);
        if (trigger == nullptr)
            continue;

        if (trigger->PlayTriggerName == resolvedTriggerName)
        {
            trigger->Play();
            return;
        }
    }

    LOG(Warning, "[AudioAnimationComponent] DispatchEvent: No sibling AudioTriggerComponent with PlayTriggerName '{0}' found on Actor '{1}'.",
        resolvedTriggerName, owner->GetName());
}
