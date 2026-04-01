#include <Engine/Core/Log.h>
#include <Engine/Level/Actor.h>
#include <Engine/Level/Actors/AnimatedModel.h>
#include <Engine/Scripting/ScriptingObject.h>

#include "AudioAnimationScript.h"
#include "AudioTriggerScript.h"

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

    // Audio scripts (AudioAnimationScript, AudioTriggerScript) live on
    // the AudioProxyActor Actor, which is the parent of the AnimatedModel.
    Actor* proxyActor = actor->GetParent();
    if (proxyActor == nullptr)
    {
        LOG(Warning, "[AudioAnimEvent] OnEvent: AnimatedModel '{0}' has no parent Actor.", actor->GetName());
        return;
    }

    // First, check whether there is an AudioAnimationScript on the proxy
    // that can remap the trigger name via EventTriggerMap.
    AudioAnimationScript* dispatcher = proxyActor->GetScript<AudioAnimationScript>();
    if (dispatcher != nullptr)
    {
        dispatcher->DispatchEvent(TriggerName);
        return;
    }

    // No dispatcher component — use TriggerName directly as PlayTriggerName.
    const Array<Script*>& scripts = proxyActor->Scripts;
    for (Script* script : scripts)
    {
        AudioTriggerScript* trigger = ScriptingObject::Cast<AudioTriggerScript>(script);
        if (trigger == nullptr)
            continue;

        if (trigger->PlayTriggerName == TriggerName)
        {
            trigger->Play();
            return;
        }
    }

    LOG(Warning, "[AudioAnimEvent] OnEvent: No AudioTriggerScript with PlayTriggerName '{0}' found on proxy Actor '{1}'.",
        TriggerName, proxyActor->GetName());
}

// ============================================================================
//  AudioAnimationScript — OnEnable
// ============================================================================

AudioAnimationScript::AudioAnimationScript(const SpawnParams& params)
    : AudioProxyDependentScript(params)
{
}

void AudioAnimationScript::OnEnable()
{
    // Resolve the sibling proxy (base class handles null/missing proxy).
    AudioProxyDependentScript::OnEnable();

    if (_proxy == nullptr)
        return;

    Actor* owner = GetActor();
    if (owner == nullptr)
        return;

    _animatedModel = owner->GetChild<AnimatedModel>();
    if (_animatedModel == nullptr)
    {
        LOG(Warning, "[AudioAnimationScript] OnEnable: No AnimatedModel found on Actor '{0}'. Animation events will not fire.",
            owner->GetName());
    }
}

// ============================================================================
//  AudioAnimationScript — OnUpdate
// ============================================================================

void AudioAnimationScript::OnUpdate()
{
    // Events are dispatched by AudioAnimEvent::OnEvent() — nothing to do here.
}

// ============================================================================
//  AudioAnimationScript — OnDisable
// ============================================================================

void AudioAnimationScript::OnDisable()
{
    _animatedModel = nullptr;
    AudioProxyDependentScript::OnDisable();
}

// ============================================================================
//  AudioAnimationScript — DispatchEvent
// ============================================================================

void AudioAnimationScript::DispatchEvent(const StringView& eventName)
{
    if (eventName.IsEmpty())
    {
        LOG(Warning, "[AudioAnimationScript] DispatchEvent: eventName is empty.");
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

    // Find the sibling AudioTriggerScript with the matching PlayTriggerName.
    const Array<Script*>& scripts = owner->Scripts;
    for (Script* script : scripts)
    {
        AudioTriggerScript* trigger = ScriptingObject::Cast<AudioTriggerScript>(script);
        if (trigger == nullptr)
            continue;

        if (trigger->PlayTriggerName == resolvedTriggerName)
        {
            trigger->Play();
            return;
        }
    }

    LOG(Warning, "[AudioAnimationScript] DispatchEvent: No sibling AudioTriggerScript with PlayTriggerName '{0}' found on Actor '{1}'.",
        resolvedTriggerName, owner->GetName());
}
