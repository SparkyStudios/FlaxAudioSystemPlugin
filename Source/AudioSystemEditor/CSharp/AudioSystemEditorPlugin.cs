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
using FlaxEngine;
using FlaxEditor.Content.Settings;

namespace AudioSystemEditor
{
    /// <summary>
    /// C# editor plugin for the AudioSystem.
    /// Orchestrates icons, asset proxies, toolbar, and menu registration,
    /// and wires up play-mode event callbacks.
    /// </summary>
    public partial class AudioSystemEditorPlugin : EditorPlugin
    {
        private static bool _initialized;

        private AudioSystemToolbar _toolbar;
        private AudioSystemIcons _icons;
        private AudioSystemAssetProxies _assetProxies;
        private AudioSystemMenu _menu;

        /// <inheritdoc />
        public override void InitializeEditor()
        {
            base.InitializeEditor();

            if (_initialized)
                return;

            _icons = new AudioSystemIcons();
            _icons.Register();

            _assetProxies = new AudioSystemAssetProxies();
            _assetProxies.Register();

            _toolbar = new AudioSystemToolbar();
            _toolbar.Register();

            _menu = new AudioSystemMenu();
            _menu.Register();

            Editor.PlayModeBegin += OnPlayModeBegin;
            Editor.PlayModeEnd += OnPlayModeEnd;

            var asset = GameSettings.LoadAsset<AudioSystemSettings>();
            if (asset == null)
                AudioSystemSettings.Save(new AudioSystemSettings());

            _initialized = true;
            Debug.Log("[AudioSystem] C# editor plugin initialized.");
        }

        /// <inheritdoc />
        public override void DeinitializeEditor()
        {
            if (!_initialized)
            {
                base.DeinitializeEditor();
                return;
            }

            Editor.PlayModeBegin -= OnPlayModeBegin;
            Editor.PlayModeEnd -= OnPlayModeEnd;

            _menu?.Unregister();
            _menu = null;

            _toolbar?.Unregister();
            _toolbar = null;

            _assetProxies?.Unregister();
            _assetProxies = null;

            _icons?.Unregister();
            _icons = null;

            _initialized = false;
            Debug.Log("[AudioSystem] C# editor plugin deinitialized.");

            base.DeinitializeEditor();
        }

        private void OnPlayModeBegin()
        {
            var settings = AudioSystemSettings.Get();
            if (settings != null)
                GameSettings.Apply();
        }

        private void OnPlayModeEnd()
        {
            // Optionally reset audio state after exiting play mode.
        }
    }
}
