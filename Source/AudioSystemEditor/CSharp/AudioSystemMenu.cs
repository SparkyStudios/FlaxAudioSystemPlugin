using FlaxEditor;
using FlaxEditor.GUI.ContextMenu;
using FlaxEngine;

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
            var settingsPath = "Settings/AudioSystem";
            var asset = Content.LoadAsync<JsonAsset>(settingsPath);
            if (asset != null)
            {
                Editor.Instance.ContentEditing.Open(asset);
            }
            else
            {
                Debug.LogWarning("[AudioSystemMenu] Settings asset not found at: " + settingsPath);
            }
        }

        private void OnToggleMute()
        {
            var prefs = AudioSystemPreferences.Get();
            if (prefs == null) return;

            prefs.MuteAudio = !prefs.MuteAudio;
            prefs.SyncSettings();
        }
    }
}
