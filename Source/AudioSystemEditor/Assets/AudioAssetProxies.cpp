#include "AudioAssetProxies.h"

// ============================================================================
//  Constructors
// ============================================================================

AudioBankAsset::AudioBankAsset(const SpawnParams& params)
    : SerializableScriptingObject(params)
{
}

AudioTriggerAsset::AudioTriggerAsset(const SpawnParams& params)
    : SerializableScriptingObject(params)
{
}

AudioRtpcAsset::AudioRtpcAsset(const SpawnParams& params)
    : SerializableScriptingObject(params)
{
}

AudioSwitchStateAsset::AudioSwitchStateAsset(const SpawnParams& params)
    : SerializableScriptingObject(params)
{
}

AudioEnvironmentAsset::AudioEnvironmentAsset(const SpawnParams& params)
    : SerializableScriptingObject(params)
{
}
