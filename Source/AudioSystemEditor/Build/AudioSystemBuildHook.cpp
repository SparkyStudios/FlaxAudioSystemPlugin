#include "AudioSystemBuildHook.h"

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Log.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Platform/File.h>
#include <Engine/Platform/FileSystem.h>
#include <Engine/Platform/StringUtils.h>

#include "../Preferences/AudioSystemPreferences.h"

// ============================================================================
//  Lifecycle
// ============================================================================

void AudioSystemBuildHook::Register()
{
    // TODO: Subscribe to GameCooker.DeployFiles.
    //
    // The GameCooker event API is declared in <Editor/Cooker/GameCooker.h>.
    // The expected registration pattern (pseudo-code):
    //
    //   #if defined(USE_EDITOR)
    //   GameCooker::DeployFiles.Bind<&AudioSystemBuildHook::OnDeployFiles>();
    //   #endif
    //
    // If the DeployFiles delegate signature passes a CookingData& parameter
    // rather than a plain String, adapt OnDeployFiles accordingly:
    //
    //   static void OnDeployFilesFromCooker(CookingData& data)
    //   {
    //       OnDeployFiles(data.DataOutputPath.ToString());
    //   }

    LOG(Info, "[AudioSystemBuildHook] Build deployment hook registered (stub — see TODO).");
}

void AudioSystemBuildHook::Unregister()
{
    // TODO: Unsubscribe from GameCooker.DeployFiles.
    //
    //   #if defined(USE_EDITOR)
    //   GameCooker::DeployFiles.Unbind<&AudioSystemBuildHook::OnDeployFiles>();
    //   #endif

    LOG(Info, "[AudioSystemBuildHook] Build deployment hook unregistered.");
}

// ============================================================================
//  DeployFiles handler
// ============================================================================

void AudioSystemBuildHook::OnDeployFiles(const String& outputPath)
{
    const AudioSystemPreferences* prefs = AudioSystemPreferences::Get();
    if (prefs == nullptr)
    {
        LOG(Warning, "[AudioSystemBuildHook] OnDeployFiles: preferences not loaded; skipping bank copy.");
        return;
    }

    if (prefs->BankDirectories.IsEmpty())
    {
        LOG(Info, "[AudioSystemBuildHook] No bank directories configured; skipping bank copy.");
        return;
    }

    // Destination root: <output>/Content/AudioBanks/
    const String banksDest = outputPath / TEXT("Content/AudioBanks");

    int32 totalCopied = 0;

    for (const String& bankDir : prefs->BankDirectories)
    {
        if (bankDir.IsEmpty())
            continue;

        if (!FileSystem::DirectoryExists(bankDir))
        {
            LOG(Warning, "[AudioSystemBuildHook] Bank directory not found, skipping: {0}", bankDir);
            continue;
        }

        // Use the last path component as the subdirectory name in the output.
        const String dirName = String(StringUtils::GetFileName(bankDir));
        const String destDir = banksDest / dirName;

        const int32 copied = CopyDirectoryRecursive(bankDir, destDir);
        if (copied < 0)
        {
            LOG(Error, "[AudioSystemBuildHook] Failed to copy bank directory: {0} to {1}", bankDir, destDir);
        }
        else
        {
            LOG(Info, "[AudioSystemBuildHook] Copied {0} bank file(s): {1} to {2}", copied, bankDir, destDir);
            totalCopied += copied;
        }
    }

    LOG(Info, "[AudioSystemBuildHook] Bank deployment complete. Total files copied: {0}", totalCopied);
}

// ============================================================================
//  Directory copy helper
// ============================================================================

int32 AudioSystemBuildHook::CopyDirectoryRecursive(const String& sourceDir, const String& destDir)
{
    if (!FileSystem::DirectoryExists(destDir))
    {
        if (FileSystem::CreateDirectory(destDir))
        {
            LOG(Error, "[AudioSystemBuildHook] Failed to create directory: {0}", destDir);
            return -1;
        }
    }

    // DirectoryGetFiles returns paths via HeapAllocation-backed array.
    Array<String, HeapAllocation> files;
    FileSystem::DirectoryGetFiles(files, sourceDir, TEXT("*"), DirectorySearchOption::AllDirectories);

    int32 copiedCount = 0;

    for (const String& srcFile : files)
    {
        // Reconstruct the destination path by replacing the source prefix with destDir.
        const String relativePart = srcFile.Substring(sourceDir.Length());
        const String destFile     = destDir + relativePart;

        // Ensure the parent sub-directory exists.
        const String destFileDir = String(StringUtils::GetDirectoryName(destFile));
        if (!FileSystem::DirectoryExists(destFileDir))
        {
            if (FileSystem::CreateDirectory(destFileDir))
            {
                LOG(Warning, "[AudioSystemBuildHook] Could not create directory: {0}", destFileDir);
                continue;
            }
        }

        if (FileSystem::CopyFile(destFile, srcFile))
        {
            LOG(Warning, "[AudioSystemBuildHook] Failed to copy file: {0} to {1}", srcFile, destFile);
            continue;
        }

        ++copiedCount;
    }

    return copiedCount;
}
