#pragma once

#include <Engine/Core/Types/String.h>

// ============================================================================
//  AudioSystemBuildHook
//
//  Subscribes to the GameCooker deployment event to copy middleware bank
//  directories into the cooked output folder.
//
//  Lifecycle:
//    Register()   — called from AudioSystemEditorPlugin::InitializeEditor()
//    Unregister() — called from AudioSystemEditorPlugin::DeinitializeEditor()
//
//  On the DeployFiles event:
//    1. Read the list of bank directory paths from AudioSystemPreferences.
//    2. For each path, recursively copy the directory into the cooked output.
//
//  NOTE: The GameCooker C++ event API may vary between Flax Engine versions.
//  TODO markers below indicate where the exact API call must be substituted.
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

    /// Called by GameCooker when it is ready to copy additional files.
    ///
    /// \param outputPath  Absolute path to the cooked output root folder.
    static void OnDeployFiles(const String& outputPath);

    /// Recursively copy \p sourceDir into \p destDir, creating the destination
    /// tree as needed. Returns the number of files copied, or -1 on error.
    static int32 CopyDirectoryRecursive(const String& sourceDir, const String& destDir);
};
