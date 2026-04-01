#include "AudioSystemBuildHook.h"

#include <Editor/Cooker/GameCooker.h>
#include <Engine/Core/Log.h>

#include "../../AudioSystem/Core/AudioSystem.h"

// ============================================================================
//  Lifecycle
// ============================================================================

void AudioSystemBuildHook::Register()
{
    GameCooker::DeployFiles.Bind<&AudioSystemBuildHook::OnDeployFiles>();
    LOG(Info, "[AudioSystemBuildHook] Build deployment hook registered.");
}

void AudioSystemBuildHook::Unregister()
{
    GameCooker::DeployFiles.Unbind<&AudioSystemBuildHook::OnDeployFiles>();
    LOG(Info, "[AudioSystemBuildHook] Build deployment hook unregistered.");
}

// ============================================================================
//  DeployFiles handler
// ============================================================================

void AudioSystemBuildHook::OnDeployFiles()
{
    AudioSystem* system = AudioSystem::Get();
    if (system == nullptr)
    {
        LOG(Warning, "[AudioSystemBuildHook] OnDeployFiles: AudioSystem not available.");
        return;
    }

    const CookingData* data = GameCooker::GetCurrentData();
    if (data == nullptr)
    {
        LOG(Warning, "[AudioSystemBuildHook] OnDeployFiles: no active CookingData.");
        return;
    }

    const String outputPath = data->DataOutputPath;
    if (!system->DeployFiles(outputPath))
    {
        LOG(Error, "[AudioSystemBuildHook] Middleware file deployment failed.");
    }
}
