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
        private const string IconsRoot = "Editor/Icons/";

        private static readonly (System.Type type, string iconName)[] ComponentIcons =
        {
            (typeof(AudioProxyComponent),             "AudioProxyComponent128"),
            (typeof(AudioListenerComponent),          "AudioListenerComponent128"),
            (typeof(AudioTriggerComponent),           "AudioTriggerComponent128"),
            (typeof(AudioRtpcComponent),              "AudioRtpcComponent128"),
            (typeof(AudioSwitchStateComponent),       "AudioSwitchStateComponent128"),
            (typeof(AudioBoxEnvironmentComponent),    "AudioBoxEnvironmentComponent128"),
            (typeof(AudioSphereEnvironmentComponent), "AudioSphereEnvironmentComponent128"),
            (typeof(AudioAnimationComponent),         "AudioAnimationComponent128"),
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
