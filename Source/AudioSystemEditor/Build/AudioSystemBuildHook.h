#pragma once

// ============================================================================
//  AudioSystemBuildHook
//
//  Subscribes to the GameCooker deployment event and delegates file
//  deployment to the loaded audio middleware. The hook itself has no
//  knowledge of middleware-specific assets or file layouts.
//
//  Lifecycle:
//    Register()   — called from AudioSystemEditorPlugin::Initialize()
//    Unregister() — called from AudioSystemEditorPlugin::Deinitialize()
// ============================================================================

class AUDIOSYSTEMEDITOR_API AudioSystemBuildHook
{
public:
    /// Subscribe to GameCooker.DeployFiles.
    static void Register();

    /// Unsubscribe from GameCooker.DeployFiles.
    static void Unregister();

private:
    AudioSystemBuildHook() = delete;
    ~AudioSystemBuildHook() = delete;

    /// Called by GameCooker during the deploy phase.
    /// Retrieves CookingData via GameCooker::GetCurrentData() and
    /// forwards to AudioSystem::DeployFiles().
    static void OnDeployFiles();
};
