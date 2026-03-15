using FlaxEditor;
using FlaxEditor.Content;
using FlaxEngine;

namespace AudioSystemEditor
{
    /// <summary>
    /// Handles registration of AudioSystem asset proxies with the Flax Editor content database.
    /// </summary>
    public class AudioSystemAssetProxies
    {
        /// <summary>
        /// Adds AudioSystem asset proxies to the editor content database.
        /// </summary>
        public void Register()
        {
            Editor.Instance.ContentDatabase.AddProxy(new SpawnableJsonAssetProxy<AudioBankAsset>());
            Editor.Instance.ContentDatabase.AddProxy(new SpawnableJsonAssetProxy<AudioTriggerAsset>());
            Editor.Instance.ContentDatabase.AddProxy(new SpawnableJsonAssetProxy<AudioRtpcAsset>());
            Editor.Instance.ContentDatabase.AddProxy(new SpawnableJsonAssetProxy<AudioSwitchStateAsset>());
            Editor.Instance.ContentDatabase.AddProxy(new SpawnableJsonAssetProxy<AudioEnvironmentAsset>());

            Editor.Instance.ContentDatabase.Rebuild(true);

            Debug.Log("[AudioSystemAssetProxies] Asset proxies registered.");
        }

        /// <summary>
        /// Removes AudioSystem asset proxies from the editor content database.
        /// </summary>
        public void Unregister()
        {
            // Proxies are cleaned up automatically when the editor shuts down
        }
    }
}
