using FlaxEngine;

#if FLAX_EDITOR
using AudioSystemEditor;
#endif

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
        /// </summary>
        public static void Play(Actor actor, string triggerName = null)
        {
            if (actor == null) return;

            var triggers = actor.GetScripts<AudioTriggerComponent>();
            foreach (var trigger in triggers)
            {
                if (triggerName == null || trigger.PlayTriggerName == triggerName)
                {
                    trigger.Play();
                    return;
                }
            }

            Debug.LogWarning($"[AudioManager] No matching AudioTriggerComponent found on '{actor.Name}' for trigger '{triggerName}'.");
        }

        /// <summary>
        /// Stop audio on a specific actor.
        /// </summary>
        public static void Stop(Actor actor, string triggerName = null)
        {
            if (actor == null) return;

            var triggers = actor.GetScripts<AudioTriggerComponent>();
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

            var rtpcs = actor.GetScripts<AudioRtpcComponent>();
            foreach (var rtpc in rtpcs)
            {
                if (rtpc.RtpcName == rtpcName)
                {
                    rtpc.SetValue(value);
                    return;
                }
            }

            Debug.LogWarning($"[AudioManager] No AudioRtpcComponent with name '{rtpcName}' found on '{actor.Name}'.");
        }

        /// <summary>
        /// Set a switch state on a specific actor.
        /// </summary>
        public static void SetSwitchState(Actor actor, string stateName)
        {
            if (actor == null) return;

            var switches = actor.GetScripts<AudioSwitchStateComponent>();
            foreach (var sw in switches)
            {
                sw.SetState(stateName);
                return;
            }

            Debug.LogWarning($"[AudioManager] No AudioSwitchStateComponent found on '{actor.Name}'.");
        }

#if FLAX_EDITOR
        /// <summary>
        /// Get or set the master volume (0.0 to 1.0).
        /// Only available in editor builds. AudioSystemPreferences is an editor-only type.
        /// </summary>
        public static float MasterVolume
        {
            get
            {
                var prefs = AudioSystemPreferences.Get();
                return prefs?.MasterGain ?? 1.0f;
            }
            set
            {
                var prefs = AudioSystemPreferences.Get();
                if (prefs == null) return;
                prefs.MasterGain = Mathf.Clamp(value, 0.0f, 1.0f);
                prefs.SyncSettings();
            }
        }

        /// <summary>
        /// Get or set the global mute state.
        /// Only available in editor builds. AudioSystemPreferences is an editor-only type.
        /// </summary>
        public static bool IsMuted
        {
            get
            {
                var prefs = AudioSystemPreferences.Get();
                return prefs?.MuteAudio ?? false;
            }
            set
            {
                var prefs = AudioSystemPreferences.Get();
                if (prefs == null) return;
                prefs.MuteAudio = value;
                prefs.SyncSettings();
            }
        }
#endif
    }
}
