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
using FlaxEditor.GUI;
using FlaxEngine;
using FlaxEditor.Content.Settings;
using FlaxEngine.GUI;

namespace AudioSystemEditor
{
    /// <summary>
    /// Manages AudioSystem toolbar contributions to the Flax Editor UI.
    /// Adds a mute toggle button to the main editor toolbar.
    /// </summary>
    public class AudioSystemToolbar
    {
        private ToolStripButton _muteButton;

        /// <summary>
        /// Registers toolbar buttons and controls in the editor.
        /// </summary>
        public void Register()
        {
            var toolstrip = Editor.Instance.UI.ToolStrip;

            toolstrip.AddSeparator();

            var muteIcon = Content.LoadAsync<Texture>(
                "Plugins/FlaxAudioSystemPlugin/Content/Editor/Icons/MuteButtonToggle50.flax"
            );
            _muteButton = new CustomIconToolStripButton(toolstrip.ItemsHeight, muteIcon)
            {
                Parent = toolstrip,
                Tag = "AudioMute",
            };
            _muteButton.LinkTooltip("Mute/Unmute Audio System");
            _muteButton.Clicked += OnMuteClicked;

            UpdateMuteButtonState();
        }

        /// <summary>
        /// Removes toolbar buttons and controls from the editor.
        /// </summary>
        public void Unregister()
        {
            if (_muteButton == null)
                return;

            _muteButton.Clicked -= OnMuteClicked;
            _muteButton.Dispose();
            _muteButton = null;
        }

        private void OnMuteClicked()
        {
            var settings = AudioSystemSettings.Get();
            if (settings == null)
                return;

            settings.MuteAudio = !settings.MuteAudio;

            AudioSystemSettings.Save(settings);
            AudioSystemSettings.Apply();

            UpdateMuteButtonState();
        }

        private void UpdateMuteButtonState()
        {
            if (_muteButton == null)
                return;

            var settings = AudioSystemSettings.Get();
            if (settings == null)
                return;

            _muteButton.Checked = settings.MuteAudio;
        }
    }

    /// <summary>
    /// Custom ToolStripButton to render a standalone Texture.
    /// </summary>
    public class CustomIconToolStripButton : ToolStripButton
    {
        private Texture _customIcon;

        /// <summary>
        /// Initializes a new instance of the <see cref="CustomIconToolStripButton"/> class.
        /// </summary>
        /// <param name="height">The height.</param>
        /// <param name="icon">The icon.</param>
        public CustomIconToolStripButton(float height, Texture icon)
            : base(height, ref SpriteHandle.Invalid)
        {
            _customIcon = icon;
        }

        /// <inheritdoc />
        public override void Draw()
        {
            base.Draw();

            if (_customIcon != null)
            {
                var style = Style.Current;
                float iconSize = Height - DefaultMargin;
                var iconRect = new Rectangle(DefaultMargin, DefaultMargin, iconSize, iconSize);
                bool enabled = EnabledInHierarchy;

                Render2D.DrawTexture(_customIcon, iconRect, enabled ? style.Foreground : style.ForegroundDisabled);
            }
        }

        /// <inheritdoc />
        public override void PerformLayout(bool force = false)
        {
            base.PerformLayout(force);

            var style = Style.Current;
            float iconSize = Height - DefaultMargin;
            float width = DefaultMargin * 2;

            if (_customIcon != null)
                width += iconSize;

            if (!string.IsNullOrEmpty(Text) && style.FontMedium != null)
                width += style.FontMedium.MeasureText(Text).X + (_customIcon != null ? DefaultMargin : 0);

            Width = width;
        }
    }
}
