using FlaxEditor;
using FlaxEditor.CustomEditors;
using FlaxEditor.CustomEditors.Editors;
using FlaxEditor.CustomEditors.Elements;
using FlaxEngine;

namespace AudioSystemEditor.Editors
{
    [CustomEditor(typeof(AudioListenerComponent)), DefaultEditor]
    public class AudioListenerEditor : GenericEditor
    {
        public override void Initialize(LayoutElementsContainer layout)
        {
            base.Initialize(layout);

            var infoGroup = layout.Group("Listener Info");
            infoGroup.Panel.Open();

            foreach (var value in Values)
            {
                if (value is AudioListenerComponent listener)
                {
                    var status = listener.IsDefault ? "This is the DEFAULT listener" : "Not the default listener";
                    var label = infoGroup.Label(status).Label;
                    label.AutoHeight = true;
                    break;
                }
            }
        }
    }
}
