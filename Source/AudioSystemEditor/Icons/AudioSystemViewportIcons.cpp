#include "AudioSystemViewportIcons.h"

#include <Engine/Core/Log.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Engine/Globals.h>
#include <Engine/Platform/File.h>

// ============================================================================
//  Internal helpers
// ============================================================================

String AudioSystemViewportIcons::ResolveIconPath(const Char* relativePath)
{
    // Icons live in the plugin's Content folder.
    // Globals::ProjectFolder points to the game project root; plugins are
    // typically under Plugins/<PluginName>/ relative to that.
    // TODO: Replace the plugin folder constant below with the actual
    //       plugin content folder path when it is known at runtime, e.g.
    //       via PluginManager::GetPlugin("AudioSystem")->GetContentFolder().
    return Globals::ProjectFolder / TEXT("Plugins/AudioSystem/Content") / relativePath;
}

void AudioSystemViewportIcons::ValidateIconFile(const Char* componentName, const String& absolutePath)
{
    if (!File::Exists(absolutePath))
    {
        LOG(Warning,
            "[AudioSystemViewportIcons] Icon file for {0} not found: {1}. "
            "Place a 128×128 PNG at that path to enable the viewport icon.",
            componentName,
            absolutePath);
    }
}

// ============================================================================
//  Lifecycle
// ============================================================================

void AudioSystemViewportIcons::Register()
{
    // Validate that expected icon assets are present on disk.
    // Missing assets produce clear log warnings rather than silent fallbacks.
    ValidateIconFile(TEXT("AudioProxyComponent"),           ResolveIconPath(k_IconProxy));
    ValidateIconFile(TEXT("AudioListenerComponent"),        ResolveIconPath(k_IconListener));
    ValidateIconFile(TEXT("AudioTriggerComponent"),         ResolveIconPath(k_IconTrigger));
    ValidateIconFile(TEXT("AudioRtpcComponent"),            ResolveIconPath(k_IconRtpc));
    ValidateIconFile(TEXT("AudioSwitchStateComponent"),     ResolveIconPath(k_IconSwitchState));
    ValidateIconFile(TEXT("AudioBoxEnvironmentComponent"),  ResolveIconPath(k_IconBoxEnvironment));
    ValidateIconFile(TEXT("AudioSphereEnvironmentComponent"), ResolveIconPath(k_IconSphereEnvironment));
    ValidateIconFile(TEXT("AudioAnimationComponent"),       ResolveIconPath(k_IconAnimation));

    // TODO: The C++ Editor API does not expose a direct icon-registration call
    // in Flax <= 1.9.  To bind icons to component types from C++, use:
    //
    //   #if defined(USE_EDITOR)
    //   auto* icons = Editor::Instance->GetIcons();
    //   icons->AddIcon(AudioProxyComponent::GetStaticClass(), iconTexture);
    //   #endif
    //
    // Until that API is available, create a C# editor script that performs:
    //
    //   var texture = Content.LoadAsync<Texture>(iconPath);
    //   Editor.Instance.Icons.AddIcon(typeof(AudioProxyComponent), texture);
    //
    // See: AudioSystemIconsEditor.cs (to be created in the C# editor scripts folder).

    LOG(Info, "[AudioSystemViewportIcons] Icon registration completed (C# binding pending — see TODO).");
}

void AudioSystemViewportIcons::Unregister()
{
    // Icon unregistration is handled by the C# editor script on teardown.
    // Nothing to do on the C++ side.
    LOG(Info, "[AudioSystemViewportIcons] Icon unregistration completed.");
}
