#pragma once

#include <Engine/Scripting/Plugins/EditorPlugin.h>

// ============================================================================
//  AudioSystemEditorPlugin
//
//  Entry point for the AudioSystem editor plugin.
//
//  Initialize() bootstraps all editor sub-systems in this order:
//    1. Load (or create) AudioSystemPreferences.
//    2. Register component viewport icons.
//    3. Register toolbar actions.
//    4. Subscribe to play-mode events to keep settings in sync.
//    5. Subscribe to GameCooker.DeployFiles for bank copying.
//
//  Deinitialize() reverses all registrations in the opposite order.
// ============================================================================

/// \brief Entry point for the AudioSystem editor plugin.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioSystemEditorPlugin : public EditorPlugin
{
    DECLARE_SCRIPTING_TYPE(AudioSystemEditorPlugin);

public:
    void Initialize() override;
    void Deinitialize() override;

private:
    /// Called when the editor enters play mode. Pushes preference values
    /// into the running AudioSystem so in-editor play mirrors real settings.
    static void OnPlayModeBegin();

    /// Called when the editor exits play mode. Re-applies preference values
    /// to restore editor-mode audio state.
    static void OnPlayModeEnd();
};
