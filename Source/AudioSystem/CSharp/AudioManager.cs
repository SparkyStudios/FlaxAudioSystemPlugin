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

using FlaxEngine;

namespace AudioSystem
{
    /// <summary>
    /// Static convenience API for common audio operations from C# game scripts.
    /// Wraps the component-based AudioSystem for simpler one-shot and global operations.
    /// </summary>
    public static class AudioManager
    {
        /// <summary>
        /// Play a trigger on a specific actor.
        /// The actor must be an AudioProxyActor or contain one as a child.
        /// </summary>
        public static void Play(Actor actor, string triggerName = null)
        {
            if (actor == null) return;

            var proxy = actor as AudioProxyActor ?? actor.GetChild<AudioProxyActor>();
            if (proxy == null)
            {
                Debug.LogWarning($"[AudioManager] No AudioProxyActor found on or under '{actor.Name}'.");
                return;
            }

            var triggers = proxy.GetScripts<AudioTriggerScript>();
            foreach (var trigger in triggers)
            {
                if (triggerName == null || trigger.PlayTriggerName == triggerName)
                {
                    trigger.Play();
                    return;
                }
            }

            Debug.LogWarning($"[AudioManager] No matching AudioTriggerScript found on '{proxy.Name}' for trigger '{triggerName}'.");
        }

        /// <summary>
        /// Stop audio on a specific actor.
        /// </summary>
        public static void Stop(Actor actor, string triggerName = null)
        {
            if (actor == null) return;

            var proxy = actor as AudioProxyActor ?? actor.GetChild<AudioProxyActor>();
            if (proxy == null) return;

            var triggers = proxy.GetScripts<AudioTriggerScript>();
            foreach (var trigger in triggers)
            {
                if (triggerName == null || trigger.PlayTriggerName == triggerName)
                {
                    trigger.Stop();
                    if (triggerName != null) return;
                }
            }
        }

        /// <summary>
        /// Set an RTPC value on a specific actor.
        /// </summary>
        public static void SetRtpc(Actor actor, string rtpcName, float value)
        {
            if (actor == null) return;

            var proxy = actor as AudioProxyActor ?? actor.GetChild<AudioProxyActor>();
            if (proxy == null)
            {
                Debug.LogWarning($"[AudioManager] No AudioProxyActor found on or under '{actor.Name}'.");
                return;
            }

            var rtpcs = proxy.GetScripts<AudioRtpcScript>();
            foreach (var rtpc in rtpcs)
            {
                if (rtpc.RtpcName == rtpcName)
                {
                    rtpc.SetValue(value);
                    return;
                }
            }

            Debug.LogWarning($"[AudioManager] No AudioRtpcScript with name '{rtpcName}' found on '{proxy.Name}'.");
        }

        /// <summary>
        /// Set a switch state on a specific actor using the first available
        /// AudioSwitchStateScript on that proxy.
        /// </summary>
        public static void SetSwitchState(Actor actor, string stateName)
        {
            if (actor == null) return;

            var proxy = actor as AudioProxyActor ?? actor.GetChild<AudioProxyActor>();
            if (proxy == null)
            {
                Debug.LogWarning($"[AudioManager] No AudioProxyActor found on or under {actor.Name}.");
                return;
            }

            var switches = proxy.GetScripts<AudioSwitchStateScript>();
            foreach (var sw in switches)
            {
                sw.SetState(stateName);
                return;
            }

            Debug.LogWarning($"[AudioManager] No AudioSwitchStateScript found on {proxy.Name}.");
        }

        /// <summary>
        /// Set a switch state on a specific actor for an explicit switch name.
        /// </summary>
        public static void SetSwitchState(Actor actor, string switchName, string stateName)
        {
            if (actor == null) return;

            var proxy = actor as AudioProxyActor ?? actor.GetChild<AudioProxyActor>();
            if (proxy == null)
            {
                Debug.LogWarning($"[AudioManager] No AudioProxyActor found on or under {actor.Name}.");
                return;
            }

            var switches = proxy.GetScripts<AudioSwitchStateScript>();
            AudioSwitchStateScript firstSwitch = null;

            foreach (var sw in switches)
            {
                if (firstSwitch == null)
                    firstSwitch = sw;

                if (sw.SwitchName == switchName)
                {
                    sw.SetStateForSwitch(switchName, stateName);
                    return;
                }
            }

            if (firstSwitch != null)
            {
                firstSwitch.SetStateForSwitch(switchName, stateName);
                return;
            }

            Debug.LogWarning($"[AudioManager] No AudioSwitchStateScript found on {proxy.Name}.");
        }
    }
}
