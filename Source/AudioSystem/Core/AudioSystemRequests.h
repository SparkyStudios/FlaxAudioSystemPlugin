#pragma once

#include <Engine/Core/Delegate.h>
#include <Engine/Core/Types/BaseTypes.h>

#include "AudioSystemData.h"

// ============================================================================
//  AudioRequestType
//
//  Enumerates every request kind that can flow through the audio request
//  queue. Used as a discriminator tag inside AudioRequest.
// ============================================================================

/// \brief Identifies the kind of audio request being submitted.
enum class AudioRequestType : uint8
{
    RegisterEntity,
    UnregisterEntity,
    UpdateEntityTransform,

    RegisterListener,
    UnregisterListener,
    UpdateListenerTransform,

    LoadTrigger,
    ActivateTrigger,
    StopEvent,
    StopAllEvents,
    UnloadTrigger,

    SetRtpcValue,
    ResetRtpcValue,
    SetSwitchState,

    SetObstructionOcclusion,
    SetEnvironmentAmount,

    LoadBank,
    UnloadBank,

    Shutdown,
};

// ============================================================================
//  AudioRequest — base for all queued audio requests
//
//  Every request carries at minimum a Type tag and an EntityId.
//  Specialized request structs extend this with additional payload fields.
//  An optional Callback is invoked on the main thread after processing.
// ============================================================================

/// \brief Base struct for all audio system requests.
struct AudioRequest
{
    AudioRequestType Type;
    AudioSystemDataID EntityId = INVALID_AUDIO_SYSTEM_ID;
    AudioSystemDataID ObjectId = INVALID_AUDIO_SYSTEM_ID;
    Function<void(bool success)> Callback;
};

// ============================================================================
//  Specialized request types
//
//  Each struct adds only the fields specific to that request kind.
//  Fields inherited from AudioRequest (Type, EntityId, ObjectId, Callback)
//  are reused where they apply.
// ============================================================================

/// \brief Request to create a new ATL entity and register it with the middleware.
struct RegisterEntityRequest : AudioRequest
{
    RegisterEntityRequest()
    {
        Type = AudioRequestType::RegisterEntity;
    }
};

/// \brief Request to remove an ATL entity from the middleware and destroy it.
struct UnregisterEntityRequest : AudioRequest
{
    UnregisterEntityRequest()
    {
        Type = AudioRequestType::UnregisterEntity;
    }
};

/// \brief Request to push a new world-space transform for an entity.
struct UpdateEntityTransformRequest : AudioRequest
{
    AudioSystemTransform Transform;

    UpdateEntityTransformRequest()
    {
        Type = AudioRequestType::UpdateEntityTransform;
    }
};

/// \brief Request to create a new ATL listener.
struct RegisterListenerRequest : AudioRequest
{
    RegisterListenerRequest()
    {
        Type = AudioRequestType::RegisterListener;
    }
};

/// \brief Request to remove an ATL listener.
struct UnregisterListenerRequest : AudioRequest
{
    UnregisterListenerRequest()
    {
        Type = AudioRequestType::UnregisterListener;
    }
};

/// \brief Request to push a new world-space transform for a listener.
struct UpdateListenerTransformRequest : AudioRequest
{
    AudioSystemTransform Transform;

    UpdateListenerTransformRequest()
    {
        Type = AudioRequestType::UpdateListenerTransform;
    }
};

/// \brief Request to load trigger data into the ATL.
struct LoadTriggerRequest : AudioRequest
{
    LoadTriggerRequest()
    {
        Type = AudioRequestType::LoadTrigger;
    }
};

/// \brief Request to fire a trigger on an entity.
struct ActivateTriggerRequest : AudioRequest
{
    ActivateTriggerRequest()
    {
        Type = AudioRequestType::ActivateTrigger;
    }
};

/// \brief Request to stop a specific event on an entity.
struct StopEventRequest : AudioRequest
{
    StopEventRequest()
    {
        Type = AudioRequestType::StopEvent;
    }
};

/// \brief Request to stop all events on an entity.
struct StopAllEventsRequest : AudioRequest
{
    StopAllEventsRequest()
    {
        Type = AudioRequestType::StopAllEvents;
    }
};

/// \brief Request to unload trigger data from the ATL.
struct UnloadTriggerRequest : AudioRequest
{
    UnloadTriggerRequest()
    {
        Type = AudioRequestType::UnloadTrigger;
    }
};

/// \brief Request to set an RTPC value on an entity.
struct SetRtpcValueRequest : AudioRequest
{
    float Value = 0.0f;

    SetRtpcValueRequest()
    {
        Type = AudioRequestType::SetRtpcValue;
    }
};

/// \brief Request to reset an RTPC to its default value on an entity.
struct ResetRtpcValueRequest : AudioRequest
{
    ResetRtpcValueRequest()
    {
        Type = AudioRequestType::ResetRtpcValue;
    }
};

/// \brief Request to set a switch state on an entity.
struct SetSwitchStateRequest : AudioRequest
{
    SetSwitchStateRequest()
    {
        Type = AudioRequestType::SetSwitchState;
    }
};

/// \brief Request to update obstruction/occlusion values for an entity.
struct SetObstructionOcclusionRequest : AudioRequest
{
    float Obstruction = 0.0f;
    float Occlusion = 0.0f;

    SetObstructionOcclusionRequest()
    {
        Type = AudioRequestType::SetObstructionOcclusion;
    }
};

/// \brief Request to set an environment effect amount on an entity.
struct SetEnvironmentAmountRequest : AudioRequest
{
    float Amount = 0.0f;

    SetEnvironmentAmountRequest()
    {
        Type = AudioRequestType::SetEnvironmentAmount;
    }
};

/// \brief Request to load a sound bank.
struct LoadBankRequest : AudioRequest
{
    LoadBankRequest()
    {
        Type = AudioRequestType::LoadBank;
    }
};

/// \brief Request to unload a sound bank.
struct UnloadBankRequest : AudioRequest
{
    UnloadBankRequest()
    {
        Type = AudioRequestType::UnloadBank;
    }
};

/// \brief Request to signal the audio thread to stop.
struct ShutdownRequest : AudioRequest
{
    ShutdownRequest()
    {
        Type = AudioRequestType::Shutdown;
    }
};
