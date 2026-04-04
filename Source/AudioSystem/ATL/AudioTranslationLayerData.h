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

#include <Engine/Core/Collections/Dictionary.h>

#include "../Core/AudioSystemData.h"

// ============================================================================
//  Forward declarations
// ============================================================================

struct ATLEvent;

/// <summary>
/// Base for all ATL wrapper objects.
///
/// Subclasses add a typed \c pData pointer specific to the object kind.
///
/// ATLControl is intentionally NOT a ScriptingObject — it lives entirely
/// on the audio thread and is never exposed to C#.
/// </summary>
struct ATLControl
{
    AudioSystemDataID Id = INVALID_AUDIO_SYSTEM_ID;

    virtual AudioSystemDataID GetId() const
    {
        return Id;
    }

    virtual ~ATLControl() = default;
};

// ============================================================================
//  Concrete ATL wrapper types
// ============================================================================

/// <summary>
/// ATL wrapper for a spatial audio entity (game object).
/// </summary>
struct ATLEntity final : ATLControl
{
    AudioSystemEntityData* pData = nullptr;
};

/// <summary>
/// ATL wrapper for an audio listener ("ears" in the scene).
/// </summary>
struct ATLListener : ATLControl
{
    AudioSystemListenerData* pData = nullptr;
};

/// <summary>
/// ATL wrapper for a trigger (event template / cue).
///
/// Tracks all currently-active event instances that were spawned by this
/// trigger so they can be stopped or queried individually.
/// </summary>
struct ATLTrigger final : ATLControl
{
    AudioSystemTriggerData* pData = nullptr;

    /// <summary>
    /// Active event instances spawned by this trigger.
    /// Key: event instance ID.  Value: owning ATLEvent pointer (not owned here).
    /// </summary>
    Dictionary<AudioSystemDataID, ATLEvent*> Events;
};

/// <summary>
/// ATL wrapper for a single running event instance.
/// </summary>
struct ATLEvent final : ATLControl
{
    AudioSystemEventData* pData = nullptr;
};

/// <summary>
/// ATL wrapper for a Real-Time Parameter Control (RTPC / parameter).
/// </summary>
struct ATLRtpc final : ATLControl
{
    AudioSystemRtpcData* pData = nullptr;
};

/// <summary>
/// ATL wrapper for a discrete switch-state value.
/// </summary>
struct ATLSwitchState final : ATLControl
{
    AudioSystemSwitchStateData* pData = nullptr;
};

/// <summary>
/// ATL wrapper for an environment (aux bus / reverb send).
/// </summary>
struct ATLEnvironment final : ATLControl
{
    AudioSystemEnvironmentData* pData = nullptr;
};

/// <summary>
/// ATL wrapper for a sound bank.
/// </summary>
struct ATLSoundBank final : ATLControl
{
    AudioSystemBankData* pData = nullptr;
};

// ============================================================================
//  Typed lookup map aliases
//
//  AudioTranslationLayer stores one map per ATL object type.  Using explicit
//  aliases keeps the member declarations readable and consistent.
// ============================================================================

using ATLEntityMap      = Dictionary<AudioSystemDataID, ATLEntity*>;
using ATLListenerMap    = Dictionary<AudioSystemDataID, ATLListener*>;
using ATLTriggerMap     = Dictionary<AudioSystemDataID, ATLTrigger*>;
using ATLEventMap       = Dictionary<AudioSystemDataID, ATLEvent*>;
using ATLRtpcMap        = Dictionary<AudioSystemDataID, ATLRtpc*>;
using ATLSwitchStateMap = Dictionary<AudioSystemDataID, ATLSwitchState*>;
using ATLEnvironmentMap = Dictionary<AudioSystemDataID, ATLEnvironment*>;
using ATLSoundBankMap   = Dictionary<AudioSystemDataID, ATLSoundBank*>;
