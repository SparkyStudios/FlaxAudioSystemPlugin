using AudioSystem;
using FlaxEditor;
using FlaxEditor.CustomEditors;
using FlaxEditor.CustomEditors.Editors;
using FlaxEditor.CustomEditors.Elements;
using FlaxEngine;
using FlaxEngine.GUI;

namespace AudioSystemEditor.Editors
{
    [CustomEditor(typeof(AudioListenerComponent)), DefaultEditor]
    public class AudioListenerEditor : GenericEditor
    {
        private Label _statusLabel;

        public override void Initialize(LayoutElementsContainer layout)
        {
            base.Initialize(layout);

            var infoGroup = layout.Group("Listener Info");
            infoGroup.Panel.Open();

            _statusLabel = infoGroup.Label("---").Label;
            _statusLabel.AutoHeight = true;

            UpdateStatusLabel();
        }

        public override void Refresh()
        {
            base.Refresh();
            UpdateStatusLabel();
        }

        private void UpdateStatusLabel()
        {
            if (_statusLabel == null) return;

            foreach (var value in Values)
            {
                if (value is AudioListenerComponent listener)
                {
                    _statusLabel.Text = listener.IsDefault
                        ? "This is the DEFAULT listener"
                        : "Not the default listener";
                    break;
                }
            }
        }
    }
}
