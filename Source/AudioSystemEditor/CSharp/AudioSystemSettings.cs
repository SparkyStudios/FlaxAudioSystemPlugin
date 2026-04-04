// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using FlaxEditor.Content.Settings;
using FlaxEngine;

namespace FlaxEditor.Content.Settings
{
    /// <summary>
    /// Editor preferences for the AudioSystem plugin.
    /// Provides master gain control and mute toggle synced to AudioSystem at runtime.
    /// </summary>
    public sealed partial class AudioSystemSettings : SettingsBase
    {
        /// <summary>
        /// Gets the instance of the settings asset (default value if missing).
        /// </summary>
        public static AudioSystemSettings Get()
        {
            return GameSettings.Load<AudioSystemSettings>();
        }

        /// <summary>
        /// Saves the settings to the disk.
        /// </summary>
        public static void Save(AudioSystemSettings settings)
        {
            var path = StringUtils.CombinePaths(Globals.ProjectContentFolder, "Settings", "AudioSystemSettings.json");
            if (Editor.SaveJsonAsset(path, settings)) return;
            var asset = FlaxEngine.Content.LoadAsync<JsonAsset>(path);
            GameSettings.SetCustomSettings("Audio System", asset);
        }

        [LibraryImport("AudioSystemEditor", EntryPoint = "AudioSystemSettingsInternal_Apply")]
        public static partial void Apply();
    }
}
