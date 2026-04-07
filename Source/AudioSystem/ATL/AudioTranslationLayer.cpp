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

#include "AudioTranslationLayer.h"

#include <Engine/Core/Log.h>
#include <Engine/Core/Types/DateTime.h>
#include <Engine/Core/Types/TimeSpan.h>

// ============================================================================
//  Constants
// ============================================================================

/// <summary>
/// ID assigned to the global "world" entity (used for non-spatial sounds).
/// </summary>
static constexpr AudioSystemDataID WORLD_ENTITY_ID = 1;

// ============================================================================
//  Helpers
// ============================================================================

/// <summary>
/// Hash a control name for reverse-lookup in the name maps.
/// </summary>
static uint32 HashName(const StringView& name)
{
    return GetHash(String(name));
}

/// <summary>
/// Build the qualified ATL lookup name for a switch state.
/// </summary>
static String BuildSwitchStateQualifiedName(StringView switchName, StringView stateName)
{
    return String(switchName) + TEXT("/") + String(stateName);
}

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

    // Initialise the middleware backend before creating any entities.
    if (!_middleware->Startup())
    {
        LOG(Error, "[ATL] Startup failed: middleware Startup() returned false.");
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
    worldEntity->Id        = WORLD_ENTITY_ID;
    worldEntity->pData     = worldData;
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

    // --- Switches -------------------------------------------------------
    for (auto& kv : _switches)
    {
        ATLSwitch* sw = kv.Value;
        if (sw != nullptr)
        {
            if (sw->pData != nullptr)
                Delete(sw->pData);
            Delete(sw);
        }
    }
    _switches.Clear();

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
    const float    dt  = static_cast<float>((now - _lastUpdateTime).GetTotalSeconds());
    _lastUpdateTime    = now;

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
//  Request dispatch
// ============================================================================

bool AudioTranslationLayer::ProcessRequest(AudioRequest&& request, bool sync)
{
    if (_middleware == nullptr)
    {
        LOG(Error, "[ATL] ProcessRequest: no middleware bound.");
        return false;
    }

    switch (request.Type)
    {
        case AudioRequestType::RegisterEntity:
            return HandleRegisterEntity(request);
        case AudioRequestType::UnregisterEntity:
            return HandleUnregisterEntity(request);
        case AudioRequestType::UpdateEntityTransform:
            return HandleUpdateEntityTransform(request);
        case AudioRequestType::RegisterListener:
            return HandleRegisterListener(request);
        case AudioRequestType::UnregisterListener:
            return HandleUnregisterListener(request);
        case AudioRequestType::UpdateListenerTransform:
            return HandleUpdateListenerTransform(request);
        case AudioRequestType::LoadTrigger:
            return HandleLoadTrigger(request);
        case AudioRequestType::ActivateTrigger:
            return HandleActivateTrigger(request);
        case AudioRequestType::StopEvent:
            return HandleStopEvent(request);
        case AudioRequestType::StopAllEvents:
            return HandleStopAllEvents(request);
        case AudioRequestType::UnloadTrigger:
            return HandleUnloadTrigger(request);
        case AudioRequestType::SetRtpcValue:
            return HandleSetRtpcValue(request);
        case AudioRequestType::ResetRtpcValue:
            return HandleResetRtpcValue(request);
        case AudioRequestType::SetSwitchState:
            return HandleSetSwitchState(request);
        case AudioRequestType::SetObstructionOcclusion:
            return HandleSetObstructionOcclusion(request);
        case AudioRequestType::SetEnvironmentAmount:
            return HandleSetEnvironmentAmount(request);
        case AudioRequestType::LoadBank:
            return HandleLoadBank(request);
        case AudioRequestType::UnloadBank:
            return HandleUnloadBank(request);
        case AudioRequestType::Shutdown:
            LOG(Info, "[ATL] Shutdown request received.");
            return true;
        default:
            LOG(Warning, "[ATL] Unknown request type: {0}.", static_cast<int32>(request.Type));
            return false;
    }
}

// ============================================================================
//  Request handlers — Entity
// ============================================================================

bool AudioTranslationLayer::HandleRegisterEntity(const AudioRequest& request)
{
    const AudioSystemDataID id = request.EntityId;
    if (id == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Error, "[ATL] RegisterEntity: invalid entity ID.");
        return false;
    }

    if (_entities.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterEntity: entity {0} already registered.", id);
        return true;
    }

    AudioSystemEntityData* data = _middleware->CreateEntityData(id);
    if (data == nullptr)
    {
        LOG(Error, "[ATL] RegisterEntity: middleware failed to create entity data for {0}.", id);
        return false;
    }

    if (!_middleware->AddEntity(id, data))
    {
        LOG(Error, "[ATL] RegisterEntity: middleware rejected entity {0}.", id);
        _middleware->DestroyEntityData(data);
        return false;
    }

    ATLEntity* entity = New<ATLEntity>();
    entity->Id        = id;
    entity->pData     = data;
    _entities.Add(id, entity);
    return true;
}

bool AudioTranslationLayer::HandleUnregisterEntity(const AudioRequest& request)
{
    const AudioSystemDataID id     = request.EntityId;
    ATLEntity*              entity = nullptr;

    if (!_entities.TryGet(id, entity) || entity == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterEntity: entity {0} not found.", id);
        return false;
    }

    if (entity->pData != nullptr)
    {
        _middleware->RemoveEntity(id, entity->pData);
        _middleware->DestroyEntityData(entity->pData);
    }

    _entities.Remove(id);
    Delete(entity);
    return true;
}

bool AudioTranslationLayer::HandleUpdateEntityTransform(const AudioRequest& request)
{
    const AudioSystemDataID id     = request.EntityId;
    ATLEntity*              entity = nullptr;

    if (!_entities.TryGet(id, entity) || entity == nullptr)
    {
        LOG(Warning, "[ATL] UpdateEntityTransform: entity {0} not found.", id);
        return false;
    }

    return _middleware->SetEntityTransform(id, entity->pData, request.Transform);
}

// ============================================================================
//  Request handlers — Listener
// ============================================================================

bool AudioTranslationLayer::HandleRegisterListener(const AudioRequest& request)
{
    const AudioSystemDataID id = request.ListenerId;
    if (id == INVALID_AUDIO_SYSTEM_ID)
    {
        LOG(Error, "[ATL] RegisterListener: invalid listener ID.");
        return false;
    }

    if (_listeners.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterListener: listener {0} already registered.", id);
        return true;
    }

    AudioSystemListenerData* data = _middleware->CreateListenerData(id, request.IsDefaultListener);
    if (data == nullptr)
    {
        LOG(Error, "[ATL] RegisterListener: middleware failed to create listener data for {0}.", id);
        return false;
    }

    if (!_middleware->AddListener(id, data))
    {
        LOG(Error, "[ATL] RegisterListener: middleware rejected listener {0}.", id);
        _middleware->DestroyListenerData(data);
        return false;
    }

    ATLListener* listener = New<ATLListener>();
    listener->Id          = id;
    listener->pData       = data;
    _listeners.Add(id, listener);
    return true;
}

bool AudioTranslationLayer::HandleUnregisterListener(const AudioRequest& request)
{
    const AudioSystemDataID id       = request.ListenerId;
    ATLListener*            listener = nullptr;

    if (!_listeners.TryGet(id, listener) || listener == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterListener: listener {0} not found.", id);
        return false;
    }

    if (listener->pData != nullptr)
    {
        _middleware->RemoveListener(id, listener->pData);
        _middleware->DestroyListenerData(listener->pData);
    }

    _listeners.Remove(id);
    Delete(listener);
    return true;
}

bool AudioTranslationLayer::HandleUpdateListenerTransform(const AudioRequest& request)
{
    const AudioSystemDataID id       = request.ListenerId;
    ATLListener*            listener = nullptr;

    if (!_listeners.TryGet(id, listener) || listener == nullptr)
    {
        LOG(Warning, "[ATL] UpdateListenerTransform: listener {0} not found.", id);
        return false;
    }

    return _middleware->SetListenerTransform(id, listener->pData, request.Transform);
}

// ============================================================================
//  Request handlers — Trigger / Event
// ============================================================================

bool AudioTranslationLayer::HandleLoadTrigger(const AudioRequest& request)
{
    const AudioSystemDataID entityId  = request.EntityId;
    const AudioSystemDataID triggerId = request.ObjectId;

    ATLEntity* entity = nullptr;
    if (!_entities.TryGet(entityId, entity) || entity == nullptr)
    {
        LOG(Warning, "[ATL] LoadTrigger: entity {0} not found.", entityId);
        return false;
    }

    ATLTrigger* trigger = nullptr;
    if (!_triggers.TryGet(triggerId, trigger) || trigger == nullptr)
    {
        LOG(Warning, "[ATL] LoadTrigger: trigger {0} not found.", triggerId);
        return false;
    }

    // Create an event instance to track the loading operation.
    AudioSystemEventData* eventData = _middleware->CreateEventData(triggerId);
    if (eventData == nullptr)
    {
        LOG(Error, "[ATL] LoadTrigger: middleware failed to create event data.");
        return false;
    }

    const bool result = _middleware->LoadTrigger(entityId, trigger->pData, eventData);
    if (!result)
    {
        _middleware->DestroyEventData(eventData);
    }
    return result;
}

bool AudioTranslationLayer::HandleActivateTrigger(const AudioRequest& request)
{
    const AudioSystemDataID entityId  = request.EntityId;
    const AudioSystemDataID triggerId = request.ObjectId;

    ATLEntity* entity = nullptr;
    if (!_entities.TryGet(entityId, entity) || entity == nullptr)
    {
        LOG(Warning, "[ATL] ActivateTrigger: entity {0} not found.", entityId);
        return false;
    }

    ATLTrigger* trigger = nullptr;
    if (!_triggers.TryGet(triggerId, trigger) || trigger == nullptr)
    {
        LOG(Warning, "[ATL] ActivateTrigger: trigger {0} not found.", triggerId);
        return false;
    }

    // Create a new event instance for this activation.
    AudioSystemEventData* eventData = _middleware->CreateEventData(triggerId);
    if (eventData == nullptr)
    {
        LOG(Error, "[ATL] ActivateTrigger: middleware failed to create event data.");
        return false;
    }

    if (!_middleware->ActivateTrigger(entityId, trigger->pData, eventData))
    {
        _middleware->DestroyEventData(eventData);
        return false;
    }

    // Track the event instance.
    const AudioSystemDataID eventId = _nextEventId++;

    ATLEvent* atlEvent = New<ATLEvent>();
    atlEvent->Id       = eventId;
    atlEvent->pData    = eventData;
    _events.Add(eventId, atlEvent);
    trigger->Events.Add(eventId, atlEvent);

    return true;
}

bool AudioTranslationLayer::HandleStopEvent(const AudioRequest& request)
{
    const AudioSystemDataID entityId = request.EntityId;
    const AudioSystemDataID eventId  = request.ObjectId;

    ATLEvent* atlEvent = nullptr;
    if (!_events.TryGet(eventId, atlEvent) || atlEvent == nullptr)
    {
        LOG(Warning, "[ATL] StopEvent: event {0} not found.", eventId);
        return false;
    }

    const bool result = _middleware->StopEvent(entityId, atlEvent->pData);

    if (atlEvent->pData != nullptr)
        _middleware->DestroyEventData(atlEvent->pData);

    // Remove the event reference from the owning trigger's Events dictionary
    // to prevent a dangling pointer.
    for (auto& triggerKv : _triggers)
    {
        ATLTrigger* trigger = triggerKv.Value;
        if (trigger != nullptr && trigger->Events.ContainsKey(eventId))
        {
            trigger->Events.Remove(eventId);
            break;
        }
    }

    _events.Remove(eventId);
    Delete(atlEvent);

    return result;
}

bool AudioTranslationLayer::HandleStopAllEvents(const AudioRequest& request)
{
    const AudioSystemDataID entityId = request.EntityId;
    return _middleware->StopAllEvents(entityId);
}

bool AudioTranslationLayer::HandleUnloadTrigger(const AudioRequest& request)
{
    const AudioSystemDataID entityId  = request.EntityId;
    const AudioSystemDataID triggerId = request.ObjectId;

    ATLTrigger* trigger = nullptr;
    if (!_triggers.TryGet(triggerId, trigger) || trigger == nullptr)
    {
        LOG(Warning, "[ATL] UnloadTrigger: trigger {0} not found.", triggerId);
        return false;
    }

    AudioSystemEventData* eventData = _middleware->CreateEventData(triggerId);
    if (eventData == nullptr)
    {
        LOG(Error, "[ATL] UnloadTrigger: middleware failed to create event data.");
        return false;
    }

    const bool result = _middleware->UnloadTrigger(entityId, trigger->pData, eventData);
    _middleware->DestroyEventData(eventData);
    return result;
}

// ============================================================================
//  Request handlers — RTPC / Switch / Environment
// ============================================================================

bool AudioTranslationLayer::HandleSetRtpcValue(const AudioRequest& request)
{
    const AudioSystemDataID entityId = request.EntityId;
    const AudioSystemDataID rtpcId   = request.ObjectId;

    ATLRtpc* rtpc = nullptr;
    if (!_rtpcs.TryGet(rtpcId, rtpc) || rtpc == nullptr)
    {
        LOG(Warning, "[ATL] SetRtpcValue: RTPC {0} not found.", rtpcId);
        return false;
    }

    return _middleware->SetRtpc(entityId, rtpc->pData, request.Value);
}

bool AudioTranslationLayer::HandleResetRtpcValue(const AudioRequest& request)
{
    const AudioSystemDataID entityId = request.EntityId;
    const AudioSystemDataID rtpcId   = request.ObjectId;

    ATLRtpc* rtpc = nullptr;
    if (!_rtpcs.TryGet(rtpcId, rtpc) || rtpc == nullptr)
    {
        LOG(Warning, "[ATL] ResetRtpcValue: RTPC {0} not found.", rtpcId);
        return false;
    }

    return _middleware->ResetRtpc(entityId, rtpc->pData);
}

bool AudioTranslationLayer::HandleSetSwitchState(const AudioRequest& request)
{
    const AudioSystemDataID objectId = request.ObjectId;

    ATLSwitchState* switchState = nullptr;
    if (!_switchStates.TryGet(objectId, switchState) || switchState == nullptr)
    {
        LOG(Warning, "[ATL] SetSwitchState: switch state {0} not found.", objectId);
        return false;
    }

    return _middleware->SetSwitchState(request.EntityId, switchState->pData);
}

bool AudioTranslationLayer::HandleSetObstructionOcclusion(const AudioRequest& request)
{
    const AudioSystemDataID entityId = request.EntityId;

    ATLEntity* entity = nullptr;
    if (!_entities.TryGet(entityId, entity) || entity == nullptr)
    {
        LOG(Warning, "[ATL] SetObstructionOcclusion: entity {0} not found.", entityId);
        return false;
    }

    return _middleware->SetObstructionAndOcclusion(entityId, entity->pData, request.Obstruction, request.Occlusion);
}

bool AudioTranslationLayer::HandleSetEnvironmentAmount(const AudioRequest& request)
{
    const AudioSystemDataID entityId = request.EntityId;
    const AudioSystemDataID envId    = request.ObjectId;

    ATLEnvironment* env = nullptr;
    if (!_environments.TryGet(envId, env) || env == nullptr)
    {
        LOG(Warning, "[ATL] SetEnvironmentAmount: environment {0} not found.", envId);
        return false;
    }

    return _middleware->SetEnvironmentAmount(entityId, env->pData, request.Amount);
}

// ============================================================================
//  Request handlers — Bank
// ============================================================================

bool AudioTranslationLayer::HandleLoadBank(const AudioRequest& request)
{
    const AudioSystemDataID bankId = request.ObjectId;

    ATLSoundBank* bank = nullptr;
    if (!_banks.TryGet(bankId, bank) || bank == nullptr)
    {
        LOG(Warning, "[ATL] LoadBank: bank {0} not found.", bankId);
        return false;
    }

    return _middleware->LoadBank(bank->pData);
}

bool AudioTranslationLayer::HandleUnloadBank(const AudioRequest& request)
{
    const AudioSystemDataID bankId = request.ObjectId;

    ATLSoundBank* bank = nullptr;
    if (!_banks.TryGet(bankId, bank) || bank == nullptr)
    {
        LOG(Warning, "[ATL] UnloadBank: bank {0} not found.", bankId);
        return false;
    }

    return _middleware->UnloadBank(bank->pData);
}

// ============================================================================
//  Control registration
// ============================================================================

bool AudioTranslationLayer::RegisterTrigger(AudioSystemDataID id, const StringView& name, AudioSystemTriggerData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterTrigger: invalid id or null data.");
        return false;
    }

    if (_triggers.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterTrigger: trigger {0} already registered.", id);
        return false;
    }

    ATLTrigger* trigger = New<ATLTrigger>();
    trigger->Id         = id;
    trigger->pData      = data;
    _triggers.Add(id, trigger);

    if (name.HasChars())
        _triggerNameMap.Add(HashName(name), id);

    return true;
}

bool AudioTranslationLayer::RegisterRtpc(AudioSystemDataID id, const StringView& name, AudioSystemRtpcData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterRtpc: invalid id or null data.");
        return false;
    }

    if (_rtpcs.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterRtpc: RTPC {0} already registered.", id);
        return false;
    }

    ATLRtpc* rtpc = New<ATLRtpc>();
    rtpc->Id      = id;
    rtpc->pData   = data;
    _rtpcs.Add(id, rtpc);

    if (name.HasChars())
        _rtpcNameMap.Add(HashName(name), id);

    return true;
}

bool AudioTranslationLayer::RegisterSwitch(AudioSystemDataID id, const StringView& name, AudioSystemSwitchData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterSwitch: invalid id or null data.");
        return false;
    }

    if (_switches.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterSwitch: switch {0} already registered.", id);
        return false;
    }

    ATLSwitch* sw = New<ATLSwitch>();
    sw->Id        = id;
    sw->pData     = data;
    _switches.Add(id, sw);

    if (name.HasChars())
        _switchNameMap.Add(HashName(name), id);

    return true;
}

bool AudioTranslationLayer::RegisterSwitchState(AudioSystemDataID id, const StringView& switchName, const StringView& stateName, AudioSystemSwitchStateData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterSwitchState: invalid id or null data.");
        return false;
    }

    if (switchName.IsEmpty() || stateName.IsEmpty())
    {
        LOG(Error, "[ATL] RegisterSwitchState: switchName and stateName are both required.");
        return false;
    }

    if (_switchStates.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterSwitchState: switch state {0} already registered.", id);
        return false;
    }

    const String            qualifiedName = BuildSwitchStateQualifiedName(switchName, stateName);
    const AudioSystemDataID nameHash      = HashName(qualifiedName);

    if (_switchStateNameMap.ContainsKey(nameHash))
    {
        LOG(Warning, "[ATL] RegisterSwitchState: qualified name '{0}' already mapped.", qualifiedName);
        return false;
    }

    ATLSwitchState* ss = New<ATLSwitchState>();
    ss->Id             = id;
    ss->pData          = data;
    _switchStates.Add(id, ss);
    _switchStateNameMap.Add(nameHash, id);

    return true;
}

bool AudioTranslationLayer::RegisterEnvironment(AudioSystemDataID id, const StringView& name, AudioSystemEnvironmentData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterEnvironment: invalid id or null data.");
        return false;
    }

    if (_environments.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterEnvironment: environment {0} already registered.", id);
        return false;
    }

    ATLEnvironment* env = New<ATLEnvironment>();
    env->Id             = id;
    env->pData          = data;
    _environments.Add(id, env);

    if (name.HasChars())
        _environmentNameMap.Add(HashName(name), id);

    return true;
}

bool AudioTranslationLayer::RegisterSoundBank(AudioSystemDataID id, const StringView& name, AudioSystemBankData* data)
{
    if (id == INVALID_AUDIO_SYSTEM_ID || data == nullptr)
    {
        LOG(Error, "[ATL] RegisterSoundBank: invalid id or null data.");
        return false;
    }

    if (_banks.ContainsKey(id))
    {
        LOG(Warning, "[ATL] RegisterSoundBank: bank {0} already registered.", id);
        return false;
    }

    ATLSoundBank* bank = New<ATLSoundBank>();
    bank->Id           = id;
    bank->pData        = data;
    _banks.Add(id, bank);

    if (name.HasChars())
        _bankNameMap.Add(HashName(name), id);

    return true;
}

// ============================================================================
//  Control unregistration
// ============================================================================

bool AudioTranslationLayer::UnregisterTrigger(AudioSystemDataID id)
{
    ATLTrigger* trigger = nullptr;
    if (!_triggers.TryGet(id, trigger) || trigger == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterTrigger: trigger {0} not found.", id);
        return false;
    }

    if (trigger->pData != nullptr && _middleware != nullptr)
        _middleware->DestroyTriggerData(trigger->pData);

    // Remove the reverse name-map entry that points to this ID.
    for (auto& kv : _triggerNameMap)
    {
        if (kv.Value == id)
        {
            _triggerNameMap.Remove(kv.Key);
            break;
        }
    }

    _triggers.Remove(id);
    Delete(trigger);
    return true;
}

bool AudioTranslationLayer::UnregisterRtpc(AudioSystemDataID id)
{
    ATLRtpc* rtpc = nullptr;
    if (!_rtpcs.TryGet(id, rtpc) || rtpc == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterRtpc: RTPC {0} not found.", id);
        return false;
    }

    if (rtpc->pData != nullptr && _middleware != nullptr)
        _middleware->DestroyRtpcData(rtpc->pData);

    for (auto& kv : _rtpcNameMap)
    {
        if (kv.Value == id)
        {
            _rtpcNameMap.Remove(kv.Key);
            break;
        }
    }

    _rtpcs.Remove(id);
    Delete(rtpc);
    return true;
}

bool AudioTranslationLayer::UnregisterSwitch(AudioSystemDataID id)
{
    ATLSwitch* sw = nullptr;
    if (!_switches.TryGet(id, sw) || sw == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterSwitch: switch {0} not found.", id);
        return false;
    }

    if (sw->pData != nullptr)
        Delete(sw->pData);

    for (auto& kv : _switchNameMap)
    {
        if (kv.Value == id)
        {
            _switchNameMap.Remove(kv.Key);
            break;
        }
    }

    _switches.Remove(id);
    Delete(sw);
    return true;
}

bool AudioTranslationLayer::UnregisterSwitchState(AudioSystemDataID id)
{
    ATLSwitchState* ss = nullptr;
    if (!_switchStates.TryGet(id, ss) || ss == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterSwitchState: switch state {0} not found.", id);
        return false;
    }

    if (ss->pData != nullptr && _middleware != nullptr)
        _middleware->DestroySwitchStateData(ss->pData);

    for (auto& kv : _switchStateNameMap)
    {
        if (kv.Value == id)
        {
            _switchStateNameMap.Remove(kv.Key);
            break;
        }
    }

    _switchStates.Remove(id);
    Delete(ss);
    return true;
}

bool AudioTranslationLayer::UnregisterEnvironment(AudioSystemDataID id)
{
    ATLEnvironment* env = nullptr;
    if (!_environments.TryGet(id, env) || env == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterEnvironment: environment {0} not found.", id);
        return false;
    }

    if (env->pData != nullptr && _middleware != nullptr)
        _middleware->DestroyEnvironmentData(env->pData);

    for (auto& kv : _environmentNameMap)
    {
        if (kv.Value == id)
        {
            _environmentNameMap.Remove(kv.Key);
            break;
        }
    }

    _environments.Remove(id);
    Delete(env);
    return true;
}

bool AudioTranslationLayer::UnregisterSoundBank(AudioSystemDataID id)
{
    ATLSoundBank* bank = nullptr;
    if (!_banks.TryGet(id, bank) || bank == nullptr)
    {
        LOG(Warning, "[ATL] UnregisterSoundBank: bank {0} not found.", id);
        return false;
    }

    if (bank->pData != nullptr && _middleware != nullptr)
        _middleware->DestroyBank(bank->pData);

    for (auto& kv : _bankNameMap)
    {
        if (kv.Value == id)
        {
            _bankNameMap.Remove(kv.Key);
            break;
        }
    }

    _banks.Remove(id);
    Delete(bank);
    return true;
}

// ============================================================================
//  Control ID lookup by hashed name
// ============================================================================

AudioSystemDataID AudioTranslationLayer::GetTriggerId(StringView name) const
{
    const uint32             nameHash = HashName(name);
    const AudioSystemDataID* id       = _triggerNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetRtpcId(StringView name) const
{
    const uint32             nameHash = HashName(name);
    const AudioSystemDataID* id       = _rtpcNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetSwitchId(StringView name) const
{
    const uint32             nameHash = HashName(name);
    const AudioSystemDataID* id       = _switchNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetSwitchStateId(StringView switchName, StringView stateName) const
{
    if (switchName.IsEmpty() || stateName.IsEmpty())
        return INVALID_AUDIO_SYSTEM_ID;

    const String             qualifiedName = BuildSwitchStateQualifiedName(switchName, stateName);
    const AudioSystemDataID* id            = _switchStateNameMap.TryGet(HashName(qualifiedName));
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetEnvironmentId(StringView name) const
{
    const uint32             nameHash = HashName(name);
    const AudioSystemDataID* id       = _environmentNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

AudioSystemDataID AudioTranslationLayer::GetBankId(StringView name) const
{
    const uint32             nameHash = HashName(name);
    const AudioSystemDataID* id       = _bankNameMap.TryGet(nameHash);
    return (id != nullptr) ? *id : INVALID_AUDIO_SYSTEM_ID;
}

// ============================================================================
//  Internal helpers
// ============================================================================

void AudioTranslationLayer::ClearAllMaps()
{
    // Free ATL wrapper objects only — does NOT call any middleware teardown.
    // This is the emergency path when _middleware is null during Shutdown().

    for (auto& kv : _banks)
        Delete(kv.Value);
    for (auto& kv : _events)
        Delete(kv.Value);
    for (auto& kv : _triggers)
        Delete(kv.Value);
    for (auto& kv : _rtpcs)
        Delete(kv.Value);
    for (auto& kv : _switches)
        Delete(kv.Value);
    for (auto& kv : _switchStates)
        Delete(kv.Value);
    for (auto& kv : _environments)
        Delete(kv.Value);
    for (auto& kv : _listeners)
        Delete(kv.Value);
    for (auto& kv : _entities)
        Delete(kv.Value);

    _banks.Clear();
    _events.Clear();
    _triggers.Clear();
    _rtpcs.Clear();
    _switches.Clear();
    _switchStates.Clear();
    _environments.Clear();
    _listeners.Clear();
    _entities.Clear();

    _triggerNameMap.Clear();
    _rtpcNameMap.Clear();
    _switchNameMap.Clear();
    _switchStateNameMap.Clear();
    _environmentNameMap.Clear();
    _bankNameMap.Clear();
}
