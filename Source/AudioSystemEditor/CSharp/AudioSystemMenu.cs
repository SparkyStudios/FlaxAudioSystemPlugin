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

using FlaxEditor;
using FlaxEditor.GUI.ContextMenu;
using FlaxEngine;
using FlaxEditor.Content.Settings;

namespace AudioSystemEditor
{
    /// <summary>
    /// Manages AudioSystem menu contributions to the Flax Editor main menu.
    /// </summary>
    public class AudioSystemMenu
    {
        private ContextMenuButton _openSettingsButton;
        private ContextMenuButton _muteButton;
        private ContextMenu _menu;

        /// <summary>
        /// Adds AudioSystem menu items to the editor main menu.
        /// </summary>
        public void Register()
        {
            var pluginsButton = Editor.Instance.UI.MainMenu.GetOrAddButton("Plugins");
            _menu = pluginsButton.ContextMenu.GetOrAddChildMenu("Audio System").ContextMenu;

            _openSettingsButton = _menu.AddButton("Open Settings...", OnOpenSettings);
            _muteButton = _menu.AddButton("Toggle Mute", OnToggleMute);
        }

        /// <summary>
        /// Removes AudioSystem menu items from the editor main menu.
        /// </summary>
        public void Unregister()
        {
            _openSettingsButton?.Dispose();
            _openSettingsButton = null;

            _muteButton?.Dispose();
            _muteButton = null;

            _menu?.Dispose();
            _menu = null;
        }

        private void OnOpenSettings()
        {
            var asset = GameSettings.LoadAsset<AudioSystemSettings>();

            if (asset != null)
                Editor.Instance.ContentEditing.Open(asset);
            else
                Debug.LogWarning("[AudioSystem] Settings asset not found. Please create it in Project Settings -> Custom Settings, or reload the plugin.");
        }

        private void OnToggleMute()
        {
            var settings = AudioSystemSettings.Get();
            if (settings == null) return;

            settings.MuteAudio = !settings.MuteAudio;
            AudioSystemSettings.Save(settings);
        }
    }
}
