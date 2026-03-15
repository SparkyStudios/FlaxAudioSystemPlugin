#include <Engine/Core/Log.h>
#include <Engine/Core/Types/DateTime.h>
#include <Engine/Core/Types/TimeSpan.h>

#include "AudioTranslationLayer.h"

// ============================================================================
//  Constants
// ============================================================================

/// ID assigned to the global "world" entity (used for non-spatial sounds).
static constexpr AudioSystemDataID WORLD_ENTITY_ID = 1;

// ============================================================================
//  Lifecycle
// ============================================================================

bool AudioTranslationLayer::Startup()
{
    if (_middleware == nullptr)
    {
        LOG(Error, "[ATL] Startup failed: no middleware has been set.");
        return false;
    }

    // Create the global world entity used for non-spatial (2-D) sounds.
    AudioSystemEntityData* worldData = _middleware->CreateWorldEntity(WORLD_ENTITY_ID);
    if (worldData == nullptr)
    {
        LOG(Error, "[ATL] Startup failed: middleware could not create the world entity.");
        return false;
    }

    if (!_middleware->AddEntity(WORLD_ENTITY_ID, worldData))
    {
        LOG(Error, "[ATL] Startup failed: middleware rejected the world entity registration.");
        _middleware->DestroyEntityData(worldData);
        return false;
    }

    ATLEntity* worldEntity = New<ATLEntity>();
    worldEntity->Id = WORLD_ENTITY_ID;
    worldEntity->pData = worldData;
    _entities.Add(WORLD_ENTITY_ID, worldEntity);

    _lastUpdateTime = DateTime::Now();

    LOG(Info, "[ATL] Startup complete. World entity registered (id={0}).", WORLD_ENTITY_ID);
    return true;
}

void AudioTranslationLayer::Shutdown()
{
    if (_middleware == nullptr)
    {
        // Already torn down or never started — clear maps defensively.
        ClearAllMaps();
        return;
    }

    // --- Banks ----------------------------------------------------------
    for (auto& kv : _banks)
    {
        ATLSoundBank* bank = kv.Value;
        if (bank != nullptr)
        {
            if (bank->pData != nullptr)
            {
                _middleware->UnloadBank(bank->pData);
                _middleware->DestroyBank(bank->pData);
            }
            Delete(bank);
        }
    }
    _banks.Clear();

    // --- Events ---------------------------------------------------------
    // Events are owned by triggers; destroy their middleware data but leave
    // the ATLTrigger.Events dictionary cleanup to the trigger loop below.
    for (auto& kv : _events)
    {
        ATLEvent* ev = kv.Value;
        if (ev != nullptr)
        {
            if (ev->pData != nullptr)
                _middleware->DestroyEventData(ev->pData);
            Delete(ev);
        }
    }
    _events.Clear();

    // --- Triggers -------------------------------------------------------
    for (auto& kv : _triggers)
    {
        ATLTrigger* trigger = kv.Value;
        if (trigger != nullptr)
        {
            if (trigger->pData != nullptr)
                _middleware->DestroyTriggerData(trigger->pData);
            // ATLTrigger.Events holds non-owning pointers — already freed above.
            Delete(trigger);
        }
    }
    _triggers.Clear();

    // --- RTPCs ----------------------------------------------------------
    for (auto& kv : _rtpcs)
    {
        ATLRtpc* rtpc = kv.Value;
        if (rtpc != nullptr)
        {
            if (rtpc->pData != nullptr)
                _middleware->DestroyRtpcData(rtpc->pData);
            Delete(rtpc);
        }
    }
    _rtpcs.Clear();

    // --- Switch states --------------------------------------------------
    for (auto& kv : _switchStates)
    {
        ATLSwitchState* ss = kv.Value;
        if (ss != nullptr)
        {
            if (ss->pData != nullptr)
                _middleware->DestroySwitchStateData(ss->pData);
            Delete(ss);
        }
    }
    _switchStates.Clear();

    // --- Environments ---------------------------------------------------
    for (auto& kv : _environments)
    {
        ATLEnvironment* env = kv.Value;
        if (env != nullptr)
        {
            if (env->pData != nullptr)
                _middleware->DestroyEnvironmentData(env->pData);
            Delete(env);
        }
    }
    _environments.Clear();

    // --- Listeners ------------------------------------------------------
    for (auto& kv : _listeners)
    {
        ATLListener* listener = kv.Value;
        if (listener != nullptr)
        {
            if (listener->pData != nullptr)
            {
                _middleware->RemoveListener(listener->Id, listener->pData);
                _middleware->DestroyListenerData(listener->pData);
            }
            Delete(listener);
        }
    }
    _listeners.Clear();

    // --- Entities -------------------------------------------------------
    // The world entity lives here; destroy it last so that the middleware can
    // cleanly unregister any sounds still attached to it.
    for (auto& kv : _entities)
    {
        ATLEntity* entity = kv.Value;
        if (entity != nullptr)
        {
            if (entity->pData != nullptr)
            {
                _middleware->RemoveEntity(entity->Id, entity->pData);
                _middleware->DestroyEntityData(entity->pData);
            }
            Delete(entity);
        }
    }
    _entities.Clear();

    // Shut down and release the middleware backend.
    _middleware->Shutdown();
    _middleware->Release();

    LOG(Info, "[ATL] Shutdown complete.");
}

void AudioTranslationLayer::Update()
{
    if (_middleware == nullptr)
    {
        LOG(Warning, "[ATL] Update called with no middleware bound — skipping.");
        return;
    }

    const DateTime now = DateTime::Now();
    const float dt = static_cast<float>((now - _lastUpdateTime).GetTotalSeconds());
    _lastUpdateTime = now;

    _middleware->Update(dt);
}

// ============================================================================
//  Middleware binding
// ============================================================================

void AudioTranslationLayer::SetMiddleware(AudioMiddleware* middleware)
{
    _middleware = middleware;
}

// ============================================================================
//  Request dispatch  (Phase 5 stub)
// ============================================================================

bool AudioTranslationLayer::ProcessRequest(Variant&& request, bool sync)
{
    // TODO (Phase 5): Implement full request dispatch.
    //
    // Once Phase 5 defines the concrete request variant types, this method
    // should contain an if/else-if chain that matches the variant type tag,
    // calls the corresponding _middleware->* method, and updates the ATL maps.
    //
    // Example sketch:
    //   if (request.Type == ActivateTriggerRequest::TypeTag)
    //   {
    //       auto& req = request.As<ActivateTriggerRequest>();
    //       return _middleware->ActivateTrigger(req.EntityId, req.TriggerData, req.EventData);
    //   }

    return false;
}

// ============================================================================
//  Control ID lookup by hashed name
//
//  NOTE (Phase 5): These maps will be populated by the asset-loading pipeline.
//  For now they are always empty, so every lookup returns INVALID_AUDIO_SYSTEM_ID.
// ============================================================================

AudioSystemDataID AudioTranslationLayer::GetTriggerId(StringView name) const
{
    // TODO (Phase 5): look up hashed name in a name→ID index.
    return INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetRtpcId(StringView name) const
{
    // TODO (Phase 5): look up hashed name in a name→ID index.
    return INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetSwitchStateId(StringView name) const
{
    // TODO (Phase 5): look up hashed name in a name→ID index.
    return INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetEnvironmentId(StringView name) const
{
    // TODO (Phase 5): look up hashed name in a name→ID index.
    return INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetBankId(StringView name) const
{
    // TODO (Phase 5): look up hashed name in a name→ID index.
    return INVALID_AUDIO_SYSTEM_ID;
}

// ============================================================================
//  Internal helpers
// ============================================================================

void AudioTranslationLayer::ClearAllMaps()
{
    // Free ATL wrapper objects only — does NOT call any middleware teardown.
    // This is the emergency path when _middleware is null during Shutdown().

    for (auto& kv : _banks)       Delete(kv.Value);
    for (auto& kv : _events)      Delete(kv.Value);
    for (auto& kv : _triggers)    Delete(kv.Value);
    for (auto& kv : _rtpcs)       Delete(kv.Value);
    for (auto& kv : _switchStates) Delete(kv.Value);
    for (auto& kv : _environments) Delete(kv.Value);
    for (auto& kv : _listeners)   Delete(kv.Value);
    for (auto& kv : _entities)    Delete(kv.Value);

    _banks.Clear();
    _events.Clear();
    _triggers.Clear();
    _rtpcs.Clear();
    _switchStates.Clear();
    _environments.Clear();
    _listeners.Clear();
    _entities.Clear();
}
