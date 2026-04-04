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
                    Debug.LogWarning($"[AudioSystem] Editor icon not found: {IconsRoot}{iconName}");
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
                    Debug.LogWarning($"[AudioSystem] Editor icon not found: {IconsRoot}{iconName}");
                    continue;
                }
                ViewportIconsRenderer.AddCustomIcon(type, texture);
                SceneGraphFactory.CustomNodesTypes[type] = nodeType;
            }

            Debug.Log("[AudioSystem] Viewport icons registered.");
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

    /// <summary>
    /// Custom scene graph node for AudioProxyActor.
    /// </summary>
    [HideInEditor]
    public sealed class AudioProxyNode : ActorNodeWithIcon
    {
        public AudioProxyNode(Actor actor) : base(actor) { }
    }

    /// <summary>
    /// Custom scene graph node for AudioListenerActor.
    /// </summary>
    [HideInEditor]
    public sealed class AudioListenerNode : ActorNodeWithIcon
    {
        public AudioListenerNode(Actor actor) : base(actor) { }
    }

    /// <summary>
    /// Custom scene graph node for AudioBoxEnvironmentActor.
    /// </summary>
    [HideInEditor]
    public sealed class AudioBoxEnvironmentNode : ActorNodeWithIcon
    {
        public AudioBoxEnvironmentNode(Actor actor) : base(actor) { }
    }

    /// <summary>
    /// Custom scene graph node for AudioSphereEnvironmentActor.
    /// </summary>
    [HideInEditor]
    public sealed class AudioSphereEnvironmentNode : ActorNodeWithIcon
    {
        public AudioSphereEnvironmentNode(Actor actor) : base(actor) { }
    }
}
