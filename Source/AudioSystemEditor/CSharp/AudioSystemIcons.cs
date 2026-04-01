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
        private const string IconsRoot = "Plugins/FlaxAudioSystemPlugin/Content/Editor/Icons/";

        // Script-based components: icon registration only.
        private static readonly (System.Type type, string iconName)[] ScriptIcons =
        {
            (typeof(AudioTriggerScript),        "AudioTriggerScript128.flax"),
            (typeof(AudioRtpcScript),           "AudioRtpcScript128.flax"),
            (typeof(AudioSwitchStateScript),    "AudioSwitchStateScript128.flax"),
            (typeof(AudioAnimationScript),      "AudioAnimationScript128.flax"),
        };

        // Actor-based components: icon registration + scene graph node type.
        private static readonly (System.Type type, string iconName, System.Type nodeType)[] ActorIcons =
        {
            (typeof(AudioProxyActor),             "AudioProxyActor128.flax",             typeof(AudioProxyNode)),
            (typeof(AudioListenerActor),          "AudioListenerActor128.flax",          typeof(AudioListenerNode)),
            (typeof(AudioBoxEnvironmentActor),    "AudioBoxEnvironmentActor128.flax",    typeof(AudioBoxEnvironmentNode)),
            (typeof(AudioSphereEnvironmentActor), "AudioSphereEnvironmentActor128.flax", typeof(AudioSphereEnvironmentNode)),
        };

        public void Register()
        {
            // Register script icons.
            foreach (var (type, iconName) in ScriptIcons)
            {
                var texture = Content.LoadAsync<Texture>(IconsRoot + iconName);
                if (texture == null)
                {
                    Debug.LogWarning($"[AudioSystemIcons] Icon not found: {IconsRoot}{iconName}");
                    continue;
                }
                ViewportIconsRenderer.AddCustomIcon(type, texture);
            }

            // Register actor icons + scene graph node types.
            foreach (var (type, iconName, nodeType) in ActorIcons)
            {
                var texture = Content.LoadAsync<Texture>(IconsRoot + iconName);
                if (texture == null)
                {
                    Debug.LogWarning($"[AudioSystemIcons] Icon not found: {IconsRoot}{iconName}");
                    continue;
                }
                ViewportIconsRenderer.AddCustomIcon(type, texture);
                SceneGraphFactory.CustomNodesTypes[type] = nodeType;
            }

            Debug.Log("[AudioSystemIcons] Viewport icons registered.");
        }

        public void Unregister()
        {
            foreach (var (type, _, _) in ActorIcons)
            {
                SceneGraphFactory.CustomNodesTypes.Remove(type);
            }
        }
    }

    // ========================================================================
    //  Custom scene graph nodes for Actor-based audio components
    // ========================================================================

    [HideInEditor]
    public sealed class AudioProxyNode : ActorNodeWithIcon
    {
        public AudioProxyNode(Actor actor) : base(actor) { }
    }

    [HideInEditor]
    public sealed class AudioListenerNode : ActorNodeWithIcon
    {
        public AudioListenerNode(Actor actor) : base(actor) { }
    }

    [HideInEditor]
    public sealed class AudioBoxEnvironmentNode : ActorNodeWithIcon
    {
        public AudioBoxEnvironmentNode(Actor actor) : base(actor) { }
    }

    [HideInEditor]
    public sealed class AudioSphereEnvironmentNode : ActorNodeWithIcon
    {
        public AudioSphereEnvironmentNode(Actor actor) : base(actor) { }
    }
}
