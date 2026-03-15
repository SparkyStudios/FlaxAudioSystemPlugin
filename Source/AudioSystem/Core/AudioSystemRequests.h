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
//  AudioRequest — flat tagged-union for all queued audio requests
//
//  All possible payload fields live directly in this struct. The Type enum
//  tag determines which fields are valid for a given request. This avoids
//  object slicing when stored by value in Array<AudioRequest>.
//
//  An optional Callback is invoked on the main thread after processing.
// ============================================================================

/// \brief Flat tagged-union struct for all audio system requests.
struct AudioRequest
{
    AudioRequestType Type;
    AudioSystemDataID EntityId = INVALID_AUDIO_SYSTEM_ID;
    AudioSystemDataID ObjectId = INVALID_AUDIO_SYSTEM_ID;

    /// World-space transform payload (UpdateEntityTransform, UpdateListenerTransform).
    AudioSystemTransform Transform;

    /// RTPC value payload (SetRtpcValue).
    float Value = 0.0f;

    /// Obstruction value payload (SetObstructionOcclusion).
    float Obstruction = 0.0f;

    /// Occlusion value payload (SetObstructionOcclusion).
    float Occlusion = 0.0f;

    /// Environment send amount payload (SetEnvironmentAmount).
    float Amount = 0.0f;

    /// Whether the request was processed successfully. Set by the audio thread
    /// before the callback is queued for main-thread invocation.
    bool Success = false;

    /// Optional callback invoked on the main thread after the request is processed.
    Function<void(bool success)> Callback;
};
