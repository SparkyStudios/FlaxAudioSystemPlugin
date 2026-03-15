#include "AudioSystemEditorPlugin.h"
#include "Engine/Scripting/Scripting.h"

IMPLEMENT_SCRIPTING_TYPE(AudioSystemEditorPlugin, EditorPlugin, "AudioSystemEditor.AudioSystemEditorPlugin", nullptr, nullptr);

AudioSystemEditorPlugin::AudioSystemEditorPlugin(const SpawnParams& params)
    : EditorPlugin(params)
{
    _description.Name = TEXT("AudioSystemEditor");
    _description.Category = TEXT("Audio");
    _description.Author = TEXT("Sparky Studios");
    _description.Description = TEXT("Editor tools for the AudioSystem plugin.");
}

void AudioSystemEditorPlugin::InitializeEditor()
{
    EditorPlugin::InitializeEditor();
    // TODO: Initialize editor components (Phase 8)
}

void AudioSystemEditorPlugin::DeinitializeEditor()
{
    // TODO: Cleanup editor components (Phase 8)
    EditorPlugin::DeinitializeEditor();
}
