#include <Engine/Engine/Engine.h>
#include <Engine/Scripting/Scripting.h>

#include "AudioSystemPlugin.h"
#include "Core/AudioSystem.h"

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

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr)
    {
        return;
    }

    audioSystem->Startup();

    // Hook into the engine update loop so the audio system is ticked every frame.
    Engine::LateUpdate.Bind<AudioSystem, &AudioSystem::UpdateSound>(audioSystem);
}

void AudioSystemPlugin::Deinitialize()
{
    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem != nullptr)
    {
        Engine::LateUpdate.Unbind<AudioSystem, &AudioSystem::UpdateSound>(audioSystem);
        audioSystem->Shutdown();
    }

    AudioSystem::Destroy();

    GamePlugin::Deinitialize();
}
