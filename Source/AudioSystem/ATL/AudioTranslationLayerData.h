#pragma once

#include <Engine/Core/Collections/Dictionary.h>

#include "../Core/AudioSystemData.h"

// ============================================================================
//  Forward declarations
// ============================================================================

struct ATLEvent;

// ============================================================================
//  ATLControl — base wrapper for all ATL objects
//
//  Each ATL wrapper holds:
//    - A stable numeric ID (AudioSystemDataID) used for O(1) map lookups.
//    - A raw pointer to the opaque middleware data block. Lifetime of the
//      data block is managed by AudioTranslationLayer, not by this struct.
//
//  ATLControl is intentionally NOT a ScriptingObject — it lives entirely
//  on the audio thread and is never exposed to C#.
// ============================================================================

/// \brief Base for all ATL wrapper objects.
///
/// Subclasses add a typed \c pData pointer specific to the object kind.
struct ATLControl
{
    AudioSystemDataID Id = INVALID_AUDIO_SYSTEM_ID;

    virtual AudioSystemDataID GetId() const { return Id; }
    virtual ~ATLControl() = default;
};

// ============================================================================
//  Concrete ATL wrapper types
// ============================================================================

/// \brief ATL wrapper for a spatial audio entity (game object).
struct ATLEntity final : ATLControl
{
    AudioSystemEntityData* pData = nullptr;
};

/// \brief ATL wrapper for an audio listener ("ears" in the scene).
struct ATLListener : ATLControl
{
    AudioSystemListenerData* pData = nullptr;
};

/// \brief ATL wrapper for a trigger (event template / cue).
///
/// Tracks all currently-active event instances that were spawned by this
/// trigger so they can be stopped or queried individually.
struct ATLTrigger final : ATLControl
{
    AudioSystemTriggerData* pData = nullptr;

    /// Active event instances spawned by this trigger.
    /// Key: event instance ID.  Value: owning ATLEvent pointer (not owned here).
    Dictionary<AudioSystemDataID, ATLEvent*> Events;
};

/// \brief ATL wrapper for a single running event instance.
struct ATLEvent final : ATLControl
{
    AudioSystemEventData* pData = nullptr;
};

/// \brief ATL wrapper for a Real-Time Parameter Control (RTPC / parameter).
struct ATLRtpc final : ATLControl
{
    AudioSystemRtpcData* pData = nullptr;
};

/// \brief ATL wrapper for a discrete switch-state value.
struct ATLSwitchState final : ATLControl
{
    AudioSystemSwitchStateData* pData = nullptr;
};

/// \brief ATL wrapper for an environment (aux bus / reverb send).
struct ATLEnvironment final : ATLControl
{
    AudioSystemEnvironmentData* pData = nullptr;
};

/// \brief ATL wrapper for a sound bank.
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
