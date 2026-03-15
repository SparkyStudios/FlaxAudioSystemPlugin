#include "AudioSystemPlugin.h"
#include "Engine/Scripting/Scripting.h"

IMPLEMENT_SCRIPTING_TYPE(AudioSystemPlugin, GamePlugin, "AudioSystem.AudioSystemPlugin", nullptr, nullptr);

AudioSystemPlugin::AudioSystemPlugin(const SpawnParams& params)
    : GamePlugin(params)
{
    _description.Name = TEXT("AudioSystem");
    _description.Category = TEXT("Audio");
    _description.Author = TEXT("Sparky Studios");
    _description.Description = TEXT("Middleware-agnostic audio system for Flax Engine.");
    _description.RepositoryUrl = TEXT("https://github.com/SparkyStudios/FlaxAudioSystemPlugin");
}

void AudioSystemPlugin::Initialize()
{
    GamePlugin::Initialize();
    // TODO: Call AudioSystem::Startup() (Phase 5)
}

void AudioSystemPlugin::Deinitialize()
{
    // TODO: Call AudioSystem::Shutdown() (Phase 5)
    GamePlugin::Deinitialize();
}
