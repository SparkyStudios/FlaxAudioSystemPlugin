#include "AudioSystemEditorPlugin.h"

#include <Engine/Core/Log.h>

#include "Build/AudioSystemBuildHook.h"
#include "Preferences/AudioSystemPreferences.h"

// ============================================================================
//  Static state
// ============================================================================

bool AudioSystemEditorPlugin::_initialized = false;

// ============================================================================
//  Constructor
// ============================================================================

AudioSystemEditorPlugin::AudioSystemEditorPlugin(const SpawnParams& params)
    : EditorPlugin(params)
{
    _description.Name        = TEXT("AudioSystemEditor");
    _description.Category    = TEXT("Audio");
    _description.Author      = TEXT("Sparky Studios");
    _description.Description = TEXT("Editor tools for the AudioSystem plugin.");
}

// ============================================================================
//  Initialize
// ============================================================================

void AudioSystemEditorPlugin::Initialize()
{
    EditorPlugin::Initialize();

    if (_initialized)
        return;

    // ------------------------------------------------------------------
    // Step 1 — Load or create preferences
    // ------------------------------------------------------------------
    if (!AudioSystemPreferences::LoadOrCreate())
    {
        LOG(Error, "[AudioSystemEditorPlugin] Failed to load/create AudioSystemPreferences. "
                   "Editor features may not function correctly.");
    }

    // ------------------------------------------------------------------
    // Step 2 — Subscribe to GameCooker.DeployFiles for bank copying
    // ------------------------------------------------------------------
    AudioSystemBuildHook::Register();

    _initialized = true;
    LOG(Info, "[AudioSystemEditorPlugin] Editor plugin initialised.");
}

// ============================================================================
//  Deinitialize
// ============================================================================

void AudioSystemEditorPlugin::Deinitialize()
{
    if (!_initialized)
    {
        EditorPlugin::Deinitialize();
        return;
    }

    AudioSystemBuildHook::Unregister();

    // Persist preferences one final time before teardown.
    if (AudioSystemPreferences* prefs = AudioSystemPreferences::Get())
    {
        prefs->Save();
    }

    AudioSystemPreferences::Destroy();

    _initialized = false;
    LOG(Info, "[AudioSystemEditorPlugin] Editor plugin deinitialised.");

    EditorPlugin::Deinitialize();
}
