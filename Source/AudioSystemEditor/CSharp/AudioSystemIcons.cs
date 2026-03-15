using AudioSystem;
using FlaxEditor;
using FlaxEditor.SceneGraph;
using FlaxEngine;

namespace AudioSystemEditor
{
    /// <summary>
    /// Handles registration of AudioSystem custom icons in the Flax Editor viewport.
    /// </summary>
    public class AudioSystemIcons
    {
        // Path relative to the project root, pointing to the plugin's Content folder.
        // Icons must be imported via the Flax Editor to create .flax texture assets.
        private const string IconsRoot = "Plugins/FlaxAudioSystemPlugin/Content/Editor/Icons/";

        private static readonly (System.Type type, string iconName)[] ComponentIcons =
        {
            (typeof(AudioProxyComponent),             "AudioProxyComponent128.flax"),
            (typeof(AudioListenerComponent),          "AudioListenerComponent128.flax"),
            (typeof(AudioTriggerComponent),           "AudioTriggerComponent128.flax"),
            (typeof(AudioRtpcComponent),              "AudioRtpcComponent128.flax"),
            (typeof(AudioSwitchStateComponent),       "AudioSwitchStateComponent128.flax"),
            (typeof(AudioBoxEnvironmentComponent),    "AudioBoxEnvironmentComponent128.flax"),
            (typeof(AudioSphereEnvironmentComponent), "AudioSphereEnvironmentComponent128.flax"),
            (typeof(AudioAnimationComponent),         "AudioAnimationComponent128.flax"),
        };

        /// <summary>
        /// Loads and registers custom icons with the editor viewport.
        /// </summary>
        public void Register()
        {
            foreach (var (type, iconName) in ComponentIcons)
            {
                var texture = Content.LoadAsync<Texture>(IconsRoot + iconName);
                if (texture == null)
                {
                    Debug.LogWarning($"[AudioSystemIcons] Icon not found: {IconsRoot}{iconName}");
                    continue;
                }

                ViewportIconsRenderer.AddCustomIcon(type, texture);
            }

            Debug.Log("[AudioSystemIcons] Viewport icons registered.");
        }

        /// <summary>
        /// Unloads and unregisters custom icons from the editor viewport.
        /// ViewportIconsRenderer cleans up automatically on plugin deinit.
        /// </summary>
        public void Unregister()
        {
            // ViewportIconsRenderer cleans up automatically on plugin deinit
        }
    }
}
