// This code was auto-generated. Do not modify it.

#include "Engine/Scripting/BinaryModule.h"
#include "AudioSystem.Gen.h"

StaticallyLinkedBinaryModuleInitializer StaticallyLinkedBinaryModuleAudioSystem(GetBinaryModuleAudioSystem);

extern "C" BinaryModule* GetBinaryModuleAudioSystem()
{
    static NativeBinaryModule module("AudioSystem");
    return &module;
}
