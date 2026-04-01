#pragma once

#include <Engine/Scripting/Plugins/EditorPlugin.h>

// ============================================================================
//  AudioSystemEditorPlugin
//
//  Entry point for the AudioSystem editor plugin (C++ side).
//
//  Initialize() bootstraps the C++ sub-systems in this order:
//    1. Load (or create) AudioSystemPreferences.
//    2. Subscribe to GameCooker.DeployFiles for bank copying.
//
//  All other editor features (toolbar, icons, asset proxies, play-mode events)
//  are handled by the C# EditorPlugin layer.
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
    static bool _initialized;
};
