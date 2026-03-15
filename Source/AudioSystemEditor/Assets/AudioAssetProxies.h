#pragma once

#include <Engine/Core/Types/BaseTypes.h>
#include <Engine/Core/Types/String.h>
#include <Engine/Scripting/SerializableScriptingObject.h>

// ============================================================================
//  Audio content asset types
//
//  Each class represents a lightweight JSON content asset that stores a named
//  reference string (trigger name, RTPC name, etc.).  Assets are serialised to
//  text files with the extensions listed below and can be created through the
//  Flax Content Browser.
//
//  Extension map:
//    AudioBankAsset         → .audiobankref
//    AudioTriggerAsset      → .audiotrigger
//    AudioRtpcAsset         → .audiortpc
//    AudioSwitchStateAsset  → .audioswitchstate
//    AudioEnvironmentAsset  → .audioenvironment
//
//  Registration notes:
//    Flax's AssetProxy system is primarily driven from C#. To register a custom
//    file-extension proxy in C++ you would subclass ContentProxy and override
//    GetFileExtension() / CanCreate() / Create().  However, this API is largely
//    internal to the Editor module and not fully exposed via the public C++ SDK.
//
//    The recommended approach for a C++ plugin is:
//      1. Define the data classes here (done below).
//      2. Register the extensions in a C# EditorPlugin by subclassing
//         CustomSettingsProxy or JsonAssetProxy for each type.
//
//    See AudioSystemAssetProxiesEditor.cs (TODO: create in C# scripts folder).
// ============================================================================

// ----------------------------------------------------------------------------
//  AudioBankAsset  (.audiobankref)
//
//  Stores the file-system path to a middleware sound bank directory.
// ----------------------------------------------------------------------------

/// \brief Content asset that references a middleware sound bank directory.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioBankAsset : public SerializableScriptingObject
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioBankAsset);

public:
    /// Expected file extension registered in the Content Browser.
    static constexpr const Char* FileExtension = TEXT(".audiobankref");

    /// Absolute or project-relative path to the middleware bank directory.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Path to the middleware bank directory.\")")
    String BankPath;
};

// ----------------------------------------------------------------------------
//  AudioTriggerAsset  (.audiotrigger)
//
//  Stores a named trigger reference that maps to a middleware event.
// ----------------------------------------------------------------------------

/// \brief Content asset that holds a named audio trigger reference.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioTriggerAsset : public SerializableScriptingObject
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioTriggerAsset);

public:
    static constexpr const Char* FileExtension = TEXT(".audiotrigger");

    /// Middleware trigger / event name.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Middleware trigger or event name.\")")
    String TriggerName;
};

// ----------------------------------------------------------------------------
//  AudioRtpcAsset  (.audiortpc)
//
//  Stores a named Real-Time Parameter Control reference plus its valid range.
// ----------------------------------------------------------------------------

/// \brief Content asset that holds a named RTPC reference and value range.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioRtpcAsset : public SerializableScriptingObject
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioRtpcAsset);

public:
    static constexpr const Char* FileExtension = TEXT(".audiortpc");

    /// Middleware RTPC / parameter name.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Middleware RTPC or parameter name.\")")
    String RtpcName;

    /// Minimum value of the RTPC range.
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"Minimum value of the RTPC range.\")")
    float MinValue = 0.0f;

    /// Maximum value of the RTPC range.
    API_FIELD(Attributes="EditorOrder(2), Tooltip(\"Maximum value of the RTPC range.\")")
    float MaxValue = 1.0f;

    /// Default value used when the RTPC is not explicitly set.
    API_FIELD(Attributes="EditorOrder(3), Tooltip(\"Default RTPC value.\")")
    float DefaultValue = 0.0f;
};

// ----------------------------------------------------------------------------
//  AudioSwitchStateAsset  (.audioswitchstate)
//
//  Stores a switch group name and one of its discrete state names.
// ----------------------------------------------------------------------------

/// \brief Content asset that identifies a specific state within a switch group.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioSwitchStateAsset : public SerializableScriptingObject
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioSwitchStateAsset);

public:
    static constexpr const Char* FileExtension = TEXT(".audioswitchstate");

    /// Name of the switch group (e.g. "WeaponType").
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Switch group name (e.g. WeaponType).\")")
    String SwitchGroupName;

    /// Name of the state within the group (e.g. "Rifle").
    API_FIELD(Attributes="EditorOrder(1), Tooltip(\"State name within the switch group (e.g. Rifle).\")")
    String StateName;
};

// ----------------------------------------------------------------------------
//  AudioEnvironmentAsset  (.audioenvironment)
//
//  Stores a named environment (aux-bus / reverb-send) reference.
// ----------------------------------------------------------------------------

/// \brief Content asset that holds a named audio environment reference.
API_CLASS() class AUDIOSYSTEMEDITOR_API AudioEnvironmentAsset : public SerializableScriptingObject
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE(AudioEnvironmentAsset);

public:
    static constexpr const Char* FileExtension = TEXT(".audioenvironment");

    /// Middleware environment / aux-bus name.
    API_FIELD(Attributes="EditorOrder(0), Tooltip(\"Middleware environment or aux-bus name.\")")
    String EnvironmentName;
};

// ============================================================================
//  AudioAssetProxies — registration helper
//
//  Call Register() from InitializeEditor() and Unregister() from
//  DeinitializeEditor() to register the custom asset types with the Flax
//  Content Browser.
// ============================================================================

class AUDIOSYSTEMEDITOR_API AudioAssetProxies
{
public:
    /// Register all AudioSystem content asset types with the editor.
    static void Register();

    /// Unregister all AudioSystem content asset types from the editor.
    static void Unregister();

private:
    AudioAssetProxies() = delete;
    ~AudioAssetProxies() = delete;
};
