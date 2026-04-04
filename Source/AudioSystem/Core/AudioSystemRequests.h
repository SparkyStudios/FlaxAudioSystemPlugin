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

#include <Engine/Core/Delegate.h>
#include <Engine/Core/Types/BaseTypes.h>

#include "AudioSystemData.h"

/// <summary>
/// Enumerates every request kind that can flow through the audio request
/// queue. Used as a discriminator tag inside AudioRequest.
/// </summary>
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

/// <summary>
/// Flat tagged-union struct for all audio system requests.
///
/// All possible payload fields live directly in this struct. The Type enum
/// tag determines which fields are valid for a given request. This avoids
/// object slicing when stored by value in Array<AudioRequest>.
///
/// An optional Callback is invoked on the main thread after processing.
/// </summary>
struct AudioRequest
{
    /// <summary>
    /// Request type, required for all requests.
    /// </summary>
    AudioRequestType Type;

    /// <summary>
    /// Entity ID, used to specify the target entity for the request.
    /// </summary>
    AudioSystemDataID EntityId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// Listener ID, used to specify the target listener for the request.
    /// </summary>
    AudioSystemDataID ListenerId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// Object ID, used to specify the target object for the request.
    /// </summary>
    AudioSystemDataID ObjectId = INVALID_AUDIO_SYSTEM_ID;

    /// <summary>
    /// World-space transform payload (UpdateEntityTransform, UpdateListenerTransform).
    /// </summary>
    AudioSystemTransform Transform;

    /// <summary>
    /// Whether the listener is the default listener (RegisterListener).
    /// </summary>
    bool IsDefaultListener = false;

    /// <summary>
    /// RTPC value payload (SetRtpcValue).
    /// </summary>
    float Value = 0.0f;

    /// <summary>
    /// Obstruction value payload (SetObstructionOcclusion).
    /// </summary>
    float Obstruction = 0.0f;

    /// <summary>
    /// Occlusion value payload (SetObstructionOcclusion).
    /// </summary>
    float Occlusion = 0.0f;

    /// <summary>
    /// Environment send amount payload (SetEnvironmentAmount).
    /// </summary>
    float Amount = 0.0f;

    /// <summary>
    /// Whether the request was processed successfully. Set by the audio thread
    /// before the callback is queued for main-thread invocation.
    /// </summary>
    bool Success = false;

    /// <summary>
    /// Optional callback invoked on the main thread after the request is processed.
    /// </summary>
    Function<void(bool success)> Callback;
};
