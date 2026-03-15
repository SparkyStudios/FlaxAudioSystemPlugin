#pragma once

#include <Engine/Core/Types/String.h>

// ============================================================================
//  Forward declarations
// ============================================================================

class AudioSystemPreferences;

// ============================================================================
//  AudioSystemActions
//
//  Manages all AudioSystem-related toolbar controls in the Flax Editor:
//
//    - Mute toggle button  — toggles MuteAudio in AudioSystemPreferences and
//                            immediately calls SyncSettings().
//    - Master volume slider — maps the integer slider value [0, 100] to the
//                             float gain [0.0, 1.0] stored in preferences.
//
//  The class is a static utility; do not instantiate it directly.
//  Call Register() once from AudioSystemEditorPlugin::InitializeEditor() and
//  Unregister() once from AudioSystemEditorPlugin::DeinitializeEditor().
//
//  NOTE: The Flax Editor C++ API for toolbar customisation is not fully
//  documented and may change between engine versions. The registration code
//  below targets the public EditorPlugin / ToolStrip API available in
//  Flax Engine >= 1.8. If that API is unavailable in the target version the
//  TODO markers below indicate what must be ported to a C# wrapper script.
// ============================================================================

class AUDIOSYSTEMEDITOR_API AudioSystemActions
{
public:
    // ========================================================================
    //  Lifecycle
    // ========================================================================

    /// Add the mute button and volume slider to the editor main toolbar.
    ///
    /// \note  TODO: If Editor::Instance->GetMainWindow()->GetToolStrip() is
    ///        not accessible from C++ in the target Flax version, this method
    ///        must be called from a C# EditorPlugin subclass instead.
    static void Register();

    /// Remove the toolbar controls added by Register().
    static void Unregister();

    // ========================================================================
    //  Action handlers (public so delegates can bind to them)
    // ========================================================================

    /// Toggle the mute state and push to AudioSystem.
    static void OnMuteClicked();

    /// Called when the volume slider value changes.
    /// \param value  Integer value in [0, 100].
    static void OnVolumeChanged(float value);

    // ========================================================================
    //  State helpers
    // ========================================================================

    /// \return The tooltip string for the mute button given the current state.
    static String GetMuteTooltip();

    /// \return The tooltip string for the volume slider given the current value.
    static String GetVolumeTooltip(int32 percentValue);

private:
    AudioSystemActions() = delete;
    ~AudioSystemActions() = delete;

    // Opaque handles to the controls added to the toolbar.
    // Stored as void* because the concrete toolbar control types are internal
    // to the Editor module and may not be forward-declarable here.

    /// Handle to the mute toggle button added to the toolbar.
    /// TODO: Replace void* with the correct ToolStripButton* type when available.
    static void* _muteButton;

    /// Handle to the master volume slider added to the toolbar.
    /// TODO: Replace void* with the correct ToolStripSlider* type when available.
    static void* _volumeSlider;
};
