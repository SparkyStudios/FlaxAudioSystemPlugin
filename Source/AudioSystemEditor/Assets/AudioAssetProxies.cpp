#include "AudioAssetProxies.h"

#include <Engine/Core/Log.h>
#include <Engine/Scripting/Scripting.h>

// ============================================================================
//  Scripting type registrations
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE(
    AudioBankAsset,
    ScriptingObject,
    "AudioSystemEditor.AudioBankAsset",
    nullptr,
    nullptr);

IMPLEMENT_SCRIPTING_TYPE(
    AudioTriggerAsset,
    ScriptingObject,
    "AudioSystemEditor.AudioTriggerAsset",
    nullptr,
    nullptr);

IMPLEMENT_SCRIPTING_TYPE(
    AudioRtpcAsset,
    ScriptingObject,
    "AudioSystemEditor.AudioRtpcAsset",
    nullptr,
    nullptr);

IMPLEMENT_SCRIPTING_TYPE(
    AudioSwitchStateAsset,
    ScriptingObject,
    "AudioSystemEditor.AudioSwitchStateAsset",
    nullptr,
    nullptr);

IMPLEMENT_SCRIPTING_TYPE(
    AudioEnvironmentAsset,
    ScriptingObject,
    "AudioSystemEditor.AudioEnvironmentAsset",
    nullptr,
    nullptr);

// ============================================================================
//  Constructors
// ============================================================================

AudioBankAsset::AudioBankAsset(const SpawnParams& params)
    : ScriptingObject(params)
{
}

AudioTriggerAsset::AudioTriggerAsset(const SpawnParams& params)
    : ScriptingObject(params)
{
}

AudioRtpcAsset::AudioRtpcAsset(const SpawnParams& params)
    : ScriptingObject(params)
{
}

AudioSwitchStateAsset::AudioSwitchStateAsset(const SpawnParams& params)
    : ScriptingObject(params)
{
}

AudioEnvironmentAsset::AudioEnvironmentAsset(const SpawnParams& params)
    : ScriptingObject(params)
{
}

// ============================================================================
//  AudioAssetProxies — registration
// ============================================================================

void AudioAssetProxies::Register()
{
    // TODO: Register custom file-extension proxies with the Flax Content Browser.
    //
    // The Flax AssetProxy system is primarily driven from C# in Flax <= 1.9.
    // To register proxies from C++, subclass ContentProxy and call:
    //
    //   ContentDatabase::RegisterProxy(New<AudioBankAssetProxy>());
    //   ContentDatabase::RegisterProxy(New<AudioTriggerAssetProxy>());
    //   ...
    //
    // Until the C++ proxy API is stabilised, create a C# EditorPlugin that
    // subclasses JsonAssetProxy for each type, overriding:
    //   - string FileExtension  → e.g. ".audiobankref"
    //   - bool CanCreate(...)   → true
    //   - void Create(...)      → write minimal JSON stub file
    //
    // See: AudioSystemAssetProxiesEditor.cs (to be created in C# editor scripts).
    //
    // Registered extensions (for documentation):
    //   AudioBankAsset         → .audiobankref
    //   AudioTriggerAsset      → .audiotrigger
    //   AudioRtpcAsset         → .audiortpc
    //   AudioSwitchStateAsset  → .audioswitchstate
    //   AudioEnvironmentAsset  → .audioenvironment

    LOG(Info, "[AudioAssetProxies] Asset proxy registration completed (C# binding pending — see TODO).");
}

void AudioAssetProxies::Unregister()
{
    // TODO: Unregister proxies added in Register() if the C++ proxy API is used.
    // When using C# proxies the teardown is handled by the EditorPlugin on the C# side.

    LOG(Info, "[AudioAssetProxies] Asset proxy unregistration completed.");
}
