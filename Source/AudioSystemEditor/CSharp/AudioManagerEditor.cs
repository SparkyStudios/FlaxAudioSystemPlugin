using FlaxEngine;

namespace AudioSystemEditor
{
    /// <summary>
    /// Editor-only convenience properties for AudioSystem global state.
    /// Use <see cref="AudioSystem.AudioManager"/> for game-side audio operations.
    /// </summary>
    public static class AudioManagerEditor
    {
        /// <summary>
        /// Get or set the master volume (0.0 to 1.0).
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
    }
}
