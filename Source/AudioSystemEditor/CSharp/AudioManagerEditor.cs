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

using FlaxEngine;
using FlaxEditor.Content.Settings;

namespace AudioSystemEditor
{
    /// <summary>
    /// Editor-only convenience properties for AudioSystem global state.
    /// Use <see cref="AudioSystem.AudioManager"/> for game-side audio operations.
    /// </summary>
    public static class AudioManagerEditor
    {
        /// <summary>
        /// Get or set the master volume (0.0 to 1.0).
        /// </summary>
        public static float MasterVolume
        {
            get
            {
                var settings = AudioSystemSettings.Get();
                return settings?.MasterGain ?? 1.0f;
            }
            set
            {
                var settings = AudioSystemSettings.Get();
                if (settings == null) return;
                settings.MasterGain = Mathf.Clamp(value, 0.0f, 1.0f);
                AudioSystemSettings.Save(settings);
                AudioSystemSettings.Apply();
            }
        }

        /// <summary>
        /// Get or set the global mute state.
        /// </summary>
        public static bool IsMuted
        {
            get
            {
                var settings = AudioSystemSettings.Get();
                return settings?.MuteAudio ?? false;
            }
            set
            {
                var settings = AudioSystemSettings.Get();
                if (settings == null) return;
                settings.MuteAudio = value;
                AudioSystemSettings.Save(settings);
                AudioSystemSettings.Apply();
            }
        }
    }
}
