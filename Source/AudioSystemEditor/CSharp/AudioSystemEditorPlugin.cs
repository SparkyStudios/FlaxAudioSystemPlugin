using FlaxEditor;
using FlaxEngine;

namespace AudioSystemEditor
{
    /// <summary>
    /// Main C# entry point for the AudioSystem editor plugin.
    /// Orchestrates icons, asset proxies, toolbar, and menu registration,
    /// and wires up play-mode event callbacks.
    /// </summary>
    public class AudioSystemEditorPlugin : EditorPlugin
    {
        private AudioSystemToolbar _toolbar;
        private AudioSystemIcons _icons;
        private AudioSystemAssetProxies _assetProxies;
        private AudioSystemMenu _menu;

        /// <inheritdoc />
        public override void InitializeEditor()
        {
            base.InitializeEditor();

            _icons = new AudioSystemIcons();
            _icons.Register();

            _assetProxies = new AudioSystemAssetProxies();
            _assetProxies.Register();

            _toolbar = new AudioSystemToolbar();
            _toolbar.Register();

            _menu = new AudioSystemMenu();
            _menu.Register();

            Editor.Instance.StateMachine.PlayingState.ScenePlaying += OnPlayModeBegin;
            Editor.Instance.StateMachine.PlayingState.SceneStopPlay += OnPlayModeEnd;

            Debug.Log("[AudioSystemEditor] C# editor plugin initialized.");
        }

        /// <inheritdoc />
        public override void DeinitializeEditor()
        {
            Editor.Instance.StateMachine.PlayingState.ScenePlaying -= OnPlayModeBegin;
            Editor.Instance.StateMachine.PlayingState.SceneStopPlay -= OnPlayModeEnd;

            _menu?.Unregister();
            _menu = null;

            _toolbar?.Unregister();
            _toolbar = null;

            _assetProxies?.Unregister();
            _assetProxies = null;

            _icons?.Unregister();
            _icons = null;

            Debug.Log("[AudioSystemEditor] C# editor plugin deinitialized.");

            base.DeinitializeEditor();
        }

        private void OnPlayModeBegin()
        {
            var prefs = AudioSystemPreferences.Get();
            if (prefs != null)
                prefs.SyncSettings();
        }

        private void OnPlayModeEnd()
        {
            // Optionally reset audio state after exiting play mode.
        }
    }
}
