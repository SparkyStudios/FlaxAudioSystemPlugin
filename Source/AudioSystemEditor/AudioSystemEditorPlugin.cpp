#include "AudioSystemEditorPlugin.h"

#include <Engine/Core/Log.h>
#include <Engine/Scripting/Scripting.h>

// Editor sub-system headers — available only when the Editor module is present.
// TODO: Verify play-mode event API against the target Flax Engine version.
#if defined(USE_EDITOR)
#include <Editor/Editor.h>
#endif

#include "Actions/AudioSystemActions.h"
#include "Assets/AudioAssetProxies.h"
#include "Build/AudioSystemBuildHook.h"
#include "Icons/AudioSystemViewportIcons.h"
#include "Preferences/AudioSystemPreferences.h"

// ============================================================================
//  Scripting type registration
// ============================================================================

IMPLEMENT_SCRIPTING_TYPE(
    AudioSystemEditorPlugin,
    EditorPlugin,
    AudioSystemEditor,
    "AudioSystemEditor.AudioSystemEditorPlugin",
    nullptr,
    nullptr);

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
//  InitializeEditor
// ============================================================================

void AudioSystemEditorPlugin::InitializeEditor()
{
    EditorPlugin::InitializeEditor();

    // ------------------------------------------------------------------
    // Step 1 — Load or create preferences
    // ------------------------------------------------------------------
    if (!AudioSystemPreferences::LoadOrCreate())
    {
        LOG(Error, "[AudioSystemEditorPlugin] Failed to load/create AudioSystemPreferences. "
                   "Editor features may not function correctly.");
    }

    // ------------------------------------------------------------------
    // Step 2 — Register component viewport icons
    // ------------------------------------------------------------------
    AudioSystemViewportIcons::Register();

    // ------------------------------------------------------------------
    // Step 3 — Add toolbar actions (mute toggle + volume slider)
    // ------------------------------------------------------------------
    AudioSystemActions::Register();

    // ------------------------------------------------------------------
    // Step 4 — Subscribe to play-mode events
    // ------------------------------------------------------------------
    // TODO: Replace with the correct Flax Editor play-mode event delegates
    //       once the exact API is confirmed for the target engine version.
    //       Expected pattern (pseudo-code):
    //
    //   #if defined(USE_EDITOR)
    //   Editor::Instance->StateMachine->PlayingState->SceneRestored
    //       .Bind<&AudioSystemEditorPlugin::OnPlayModeEnd>();
    //   Editor::Instance->StateMachine->PlayingState->SceneSaving
    //       .Bind<&AudioSystemEditorPlugin::OnPlayModeBegin>();
    //   #endif

    LOG(Info, "[AudioSystemEditorPlugin] Play-mode event subscription pending (see TODO).");

    // ------------------------------------------------------------------
    // Step 5 — Subscribe to GameCooker.DeployFiles for bank copying
    // ------------------------------------------------------------------
    AudioSystemBuildHook::Register();

    // ------------------------------------------------------------------
    // Step 6 — Register asset proxies
    // ------------------------------------------------------------------
    AudioAssetProxies::Register();

    LOG(Info, "[AudioSystemEditorPlugin] Editor plugin initialised.");
}

// ============================================================================
//  DeinitializeEditor
// ============================================================================

void AudioSystemEditorPlugin::DeinitializeEditor()
{
    // Reverse the registration order from InitializeEditor().

    AudioAssetProxies::Unregister();
    AudioSystemBuildHook::Unregister();

    // TODO: Unsubscribe play-mode events (mirror the subscription TODO above).

    AudioSystemActions::Unregister();
    AudioSystemViewportIcons::Unregister();

    // Persist preferences one final time before teardown.
    if (AudioSystemPreferences* prefs = AudioSystemPreferences::Get())
    {
        prefs->Save();
    }

    LOG(Info, "[AudioSystemEditorPlugin] Editor plugin deinitialised.");

    EditorPlugin::DeinitializeEditor();
}

// ============================================================================
//  Play-mode event handlers
// ============================================================================

void AudioSystemEditorPlugin::OnPlayModeBegin()
{
    // Push current preference values into AudioSystem when play mode starts.
    if (AudioSystemPreferences* prefs = AudioSystemPreferences::Get())
    {
        prefs->SyncSettings();
    }
}

void AudioSystemEditorPlugin::OnPlayModeEnd()
{
    // Re-apply preferences when returning to editor mode so the audio state
    // is consistent with what the user last configured.
    if (AudioSystemPreferences* prefs = AudioSystemPreferences::Get())
    {
        prefs->SyncSettings();
    }
}
