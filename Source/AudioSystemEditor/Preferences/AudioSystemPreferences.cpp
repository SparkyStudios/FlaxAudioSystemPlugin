#include "AudioSystemPreferences.h"

#include <Engine/Core/Log.h>
#include <Engine/Engine/Globals.h>
#include <Engine/Platform/File.h>
#include <Engine/Platform/FileSystem.h>
#include <Engine/Platform/StringUtils.h>
#include <Engine/Scripting/Scripting.h>
#include <Engine/Serialization/JsonSerializer.h>
#include <FlaxEngine.Gen.h>

#include "../../AudioSystem/Core/AudioSystem.h"

// ============================================================================
//  Static state
// ============================================================================

AudioSystemPreferences* AudioSystemPreferences::_instance = nullptr;

// ============================================================================
//  Constructor
// ============================================================================

AudioSystemPreferences::AudioSystemPreferences(const SpawnParams& params)
    : SerializableScriptingObject(params)
{
}

// ============================================================================
//  Singleton access
// ============================================================================

AudioSystemPreferences* AudioSystemPreferences::Get()
{
    return _instance;
}

// ============================================================================
//  Runtime sync
// ============================================================================

void AudioSystemPreferences::SyncSettings()
{
    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr || !audioSystem->IsInitialized())
    {
        // AudioSystem not yet started — nothing to sync.
        return;
    }

    audioSystem->SetMasterVolume(MasterGain);
    audioSystem->SetMasterMute(MuteAudio);
}

// ============================================================================
//  Persistence helpers
// ============================================================================

String AudioSystemPreferences::GetSettingsPath()
{
    // <ProjectFolder>/Settings/AudioSystem.json
    return Globals::ProjectFolder / TEXT("Settings/AudioSystem.json");
}

bool AudioSystemPreferences::LoadOrCreate()
{
    // Allocate the singleton instance if it does not already exist.
    if (_instance == nullptr)
    {
        SpawnParams spawnParams(Guid::New(), AudioSystemPreferences::TypeInitializer);
        _instance = New<AudioSystemPreferences>(spawnParams);
    }

    const String settingsPath = GetSettingsPath();

    if (FileSystem::FileExists(settingsPath))
    {
        // Read raw bytes from disk.
        Array<byte> fileData;
        if (File::ReadAllBytes(settingsPath, fileData))
        {
            LOG(Warning, "[AudioSystemPreferences] Failed to read settings from: {0}", settingsPath);
            // Continue with defaults rather than aborting.
            return true;
        }

        // Deserialise JSON into the instance fields.
        JsonSerializer::LoadFromBytes(_instance, fileData, FLAXENGINE_VERSION_BUILD);

        LOG(Info, "[AudioSystemPreferences] Loaded settings from: {0}", settingsPath);
    }
    else
    {
        // First run — persist default values to disk.
        LOG(Info,
            "[AudioSystemPreferences] Settings file not found; creating defaults at: {0}",
            settingsPath);
        _instance->Save();
    }

    return true;
}

void AudioSystemPreferences::Save() const
{
    const String settingsPath = GetSettingsPath();

    // Ensure the Settings/ directory exists.
    const String settingsDir = StringUtils::GetDirectoryName(settingsPath);
    if (!FileSystem::DirectoryExists(settingsDir))
    {
        FileSystem::CreateDirectory(settingsDir);
    }

    // Serialise the instance fields to raw JSON bytes.
    const Array<byte> data = JsonSerializer::SaveToBytes(
        const_cast<AudioSystemPreferences*>(this));

    if (File::WriteAllBytes(settingsPath, data))
    {
        LOG(Warning, "[AudioSystemPreferences] Failed to write settings to: {0}", settingsPath);
        return;
    }

    LOG(Info, "[AudioSystemPreferences] Settings saved to: {0}", settingsPath);
}
