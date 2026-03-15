#pragma once
#include <Engine/Plugins/EditorPlugin.h>

/// \brief Entry point for the AudioSystem editor plugin.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioSystemEditorPlugin : public EditorPlugin
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSystemEditorPlugin);
public:
    void InitializeEditor() override;
    void DeinitializeEditor() override;
};
