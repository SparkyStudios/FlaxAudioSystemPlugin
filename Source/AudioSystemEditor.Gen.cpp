// This code was auto-generated. Do not modify it.

#include "Engine/Scripting/BinaryModule.h"
#include "AudioSystemEditor.Gen.h"

StaticallyLinkedBinaryModuleInitializer StaticallyLinkedBinaryModuleAudioSystemEditor(GetBinaryModuleAudioSystemEditor);

extern "C" BinaryModule* GetBinaryModuleAudioSystemEditor()
{
    static NativeBinaryModule module("AudioSystemEditor");
    return &module;
}
