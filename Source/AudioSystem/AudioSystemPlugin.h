#pragma once
#include <Engine/Scripting/Plugins/GamePlugin.h>

/// \brief Entry point for the AudioSystem game plugin.
API_CLASS() class AUDIOSYSTEM_API AudioSystemPlugin : public GamePlugin
{
    DECLARE_SCRIPTING_TYPE(AudioSystemPlugin);
public:
    void Initialize() override;
    void Deinitialize() override;

private:
    /// Guards against double initialization when multiple projects reference this plugin.
    static bool _initialized;
};
