#include <Engine/Engine/Engine.h>
#include <Engine/Scripting/Scripting.h>

#include "AudioSystemPlugin.h"
#include "Core/AudioSystem.h"

bool AudioSystemPlugin::_initialized = false;

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

    if (_initialized)
        return;

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem == nullptr)
        return;

    // Do not call Startup() here — the middleware plugin has not registered yet.
    // The middleware plugin (e.g. AmplitudeAudioPlugin) is responsible for
    // calling RegisterMiddleware() followed by Startup().

    // Hook into the engine update loop so the audio system is ticked every frame.
    Engine::LateUpdate.Bind<AudioSystem, &AudioSystem::UpdateSound>(audioSystem);
    _initialized = true;
}

void AudioSystemPlugin::Deinitialize()
{
    if (!_initialized)
    {
        GamePlugin::Deinitialize();
        return;
    }

    AudioSystem* audioSystem = AudioSystem::Get();
    if (audioSystem != nullptr)
    {
        Engine::LateUpdate.Unbind<AudioSystem, &AudioSystem::UpdateSound>(audioSystem);
        audioSystem->Shutdown();
    }

    AudioSystem::Destroy();
    _initialized = false;

    GamePlugin::Deinitialize();
}
