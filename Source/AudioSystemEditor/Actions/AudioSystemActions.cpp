#include "AudioSystemActions.h"

#include <Engine/Core/Log.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Core/Math/Math.h>

// Editor headers — adjust include paths if the Flax Engine version differs.
// TODO: Verify these paths against the target Flax Engine version.
#if defined(USE_EDITOR)
#include <Editor/Editor.h>
#endif

#include "../Preferences/AudioSystemPreferences.h"

// ============================================================================
//  Static member definitions
// ============================================================================

void* AudioSystemActions::_muteButton   = nullptr;
void* AudioSystemActions::_volumeSlider = nullptr;

// ============================================================================
//  Lifecycle
// ============================================================================

void AudioSystemActions::Register()
{
    // TODO: The Flax Editor C++ toolbar API may not be available in all
    // versions.  If Editor::Instance->GetMainWindow()->GetToolStrip() is not
    // accessible, this registration block must be moved to a C# EditorPlugin
    // subclass (see documentation note in AudioSystemActions.h).
    //
    // Expected C++ registration pattern (pseudo-code):
    //
    //   #if defined(USE_EDITOR)
    //   auto* toolbar = Editor::Instance->GetMainWindow()->GetToolStrip();
    //
    //   // --- Mute toggle button -------------------------------------------
    //   auto* muteBtn = toolbar->AddButton(nullptr, TEXT("Mute/Unmute Audio System"));
    //   muteBtn->Clicked.Bind<&AudioSystemActions::OnMuteClicked>();
    //   _muteButton = muteBtn;
    //
    //   // --- Master volume slider ------------------------------------------
    //   auto* volSlider = toolbar->AddSlider(0.0f, 100.0f, TEXT("Audio Master Volume"));
    //   volSlider->ValueChanged.Bind<&AudioSystemActions::OnVolumeChanged>();
    //
    //   // Sync slider to current preference value.
    //   if (auto* prefs = AudioSystemPreferences::Get())
    //       volSlider->SetValue(prefs->MasterGain * 100.0f);
    //
    //   _volumeSlider = volSlider;
    //   #endif

    LOG(Info, "[AudioSystemActions] Toolbar actions registered (stub — see TODO).");
}

void AudioSystemActions::Unregister()
{
    // TODO: Remove the controls added in Register() using the toolbar API.
    // Expected pattern (pseudo-code):
    //
    //   #if defined(USE_EDITOR)
    //   if (_muteButton)
    //   {
    //       auto* muteBtn = static_cast<ToolStripButton*>(_muteButton);
    //       muteBtn->Clicked.Unbind<&AudioSystemActions::OnMuteClicked>();
    //       muteBtn->Dispose();
    //       _muteButton = nullptr;
    //   }
    //
    //   if (_volumeSlider)
    //   {
    //       auto* volSlider = static_cast<ToolStripSlider*>(_volumeSlider);
    //       volSlider->ValueChanged.Unbind<&AudioSystemActions::OnVolumeChanged>();
    //       volSlider->Dispose();
    //       _volumeSlider = nullptr;
    //   }
    //   #endif

    _muteButton   = nullptr;
    _volumeSlider = nullptr;

    LOG(Info, "[AudioSystemActions] Toolbar actions unregistered.");
}

// ============================================================================
//  Action handlers
// ============================================================================

void AudioSystemActions::OnMuteClicked()
{
    AudioSystemPreferences* prefs = AudioSystemPreferences::Get();
    if (prefs == nullptr)
    {
        LOG(Warning, "[AudioSystemActions] OnMuteClicked: preferences not loaded.");
        return;
    }

    prefs->MuteAudio = !prefs->MuteAudio;
    prefs->SyncSettings();
    prefs->Save();

    LOG(Info, "[AudioSystemActions] Audio mute toggled: {0}", prefs->MuteAudio);
}

void AudioSystemActions::OnVolumeChanged(float value)
{
    AudioSystemPreferences* prefs = AudioSystemPreferences::Get();
    if (prefs == nullptr)
    {
        LOG(Warning, "[AudioSystemActions] OnVolumeChanged: preferences not loaded.");
        return;
    }

    // Map [0, 100] integer percentage → [0.0, 1.0] float gain.
    const float clampedValue = Math::Clamp(value, 0.0f, 100.0f);
    prefs->MasterGain = clampedValue / 100.0f;
    prefs->SyncSettings();
    prefs->Save();
}

// ============================================================================
//  Tooltip helpers
// ============================================================================

String AudioSystemActions::GetMuteTooltip()
{
    const AudioSystemPreferences* prefs = AudioSystemPreferences::Get();
    if (prefs == nullptr)
        return TEXT("Mute/Unmute Audio System");

    return prefs->MuteAudio
        ? TEXT("Unmute Audio System")
        : TEXT("Mute Audio System");
}

String AudioSystemActions::GetVolumeTooltip(int32 percentValue)
{
    return String::Format(TEXT("Audio Master Volume: {0}%"), percentValue);
}
