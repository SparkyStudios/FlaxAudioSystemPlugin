#pragma once
#include <Engine/Plugins/GamePlugin.h>

/// \brief Entry point for the AudioSystem game plugin.
API_CLASS() class AUDIOSYSTEM_API AudioSystemPlugin : public GamePlugin
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSystemPlugin);
public:
    void Initialize() override;
    void Deinitialize() override;
};
