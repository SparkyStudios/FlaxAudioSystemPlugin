using FlaxEditor;
using FlaxEditor.GUI;
using FlaxEngine;
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

            // NOTE: EditorIcons does not expose a dedicated Volume/Mute 64px icon.
            // AudioSettings128 is the closest built-in icon for audio-related actions.
            // If a custom mute icon is added via AudioSystemIcons, replace this reference.
            _muteButton = (ToolStripButton)toolstrip.AddButton(
                Editor.Instance.Icons.AudioSettings128
            );
            _muteButton.Tag = "AudioMute";
            _muteButton.LinkTooltip("Mute/Unmute Audio System");
            _muteButton.Clicked += OnMuteClicked;

            UpdateMuteButtonState();
        }

        /// <summary>
        /// Removes toolbar buttons and controls from the editor.
        /// </summary>
        public void Unregister()
        {
            if (_muteButton != null)
            {
                _muteButton.Clicked -= OnMuteClicked;
                _muteButton.Dispose();
                _muteButton = null;
            }
        }

        private void OnMuteClicked()
        {
            var prefs = AudioSystemPreferences.Get();
            if (prefs == null)
                return;

            prefs.MuteAudio = !prefs.MuteAudio;
            prefs.SyncSettings();
            UpdateMuteButtonState();
        }

        private void UpdateMuteButtonState()
        {
            if (_muteButton == null)
                return;

            var prefs = AudioSystemPreferences.Get();
            if (prefs == null)
                return;

            _muteButton.Checked = prefs.MuteAudio;
        }
    }
}
