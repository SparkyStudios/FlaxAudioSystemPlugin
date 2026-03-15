#pragma once

#include <Engine/Core/Collections/Array.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/ScriptingObject.h>

// ============================================================================
//  AudioSystemPreferences
//
//  Persisted editor settings for the AudioSystem plugin. Stored as a Flax
//  JSON settings asset at <ProjectFolder>/Settings/AudioSystem.json.
//
//  Call SyncSettings() to push values into the running AudioSystem instance.
//  Use Get() to access the global singleton instance.
// ============================================================================

/// \brief Editor preferences for the AudioSystem plugin.
///
/// Serialised to <ProjectFolder>/Settings/AudioSystem.json.
/// Provides master gain control and mute toggle synced to AudioSystem at runtime.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioSystemPreferences : public ScriptingObject
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSystemPreferences);

public:
    // ========================================================================
    //  Serialised fields
    // ========================================================================

    /// Master gain level applied to all audio output. Range [0.0, 1.0].
    API_FIELD(Attributes="Limit(0.0f, 1.0f), EditorOrder(0), Tooltip(\"Master gain level for all audio output. Range 0 to 1.\")")
    float MasterGain = 1.0f;

    /// When true, all audio output is silenced regardless of master gain.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Mute all audio output.\")")
    bool MuteAudio = false;

    /// List of bank directory paths to copy into the cooked output on build.
    /// Each entry should be an absolute or project-relative folder path.
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Bank directories to include in the cooked build.\")")
    Array<String> BankDirectories;

    // ========================================================================
    //  Runtime sync
    // ========================================================================

    /// Push the current preference values into the running AudioSystem instance.
    /// No-op if AudioSystem is not yet initialised.
    API_FUNCTION() void SyncSettings();

    // ========================================================================
    //  Persistence
    // ========================================================================

    /// Load preferences from the JSON asset, or create a default asset if absent.
    /// Returns true on success.
    static bool LoadOrCreate();

    /// Persist current values back to disk (JSON asset).
    void Save() const;

    /// \return The global singleton instance. May be null before LoadOrCreate() succeeds.
    static AudioSystemPreferences* Get();

private:
    /// Returns the absolute path to the settings JSON file.
    static String GetSettingsPath();

    static AudioSystemPreferences* _instance;
};
