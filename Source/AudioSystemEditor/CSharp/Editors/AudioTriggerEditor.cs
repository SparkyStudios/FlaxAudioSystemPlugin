using AudioSystem;
using FlaxEditor;
using FlaxEditor.CustomEditors;
using FlaxEditor.CustomEditors.Editors;
using FlaxEditor.CustomEditors.Elements;
using FlaxEngine;
using FlaxEngine.GUI;

namespace AudioSystemEditor.Editors
{
    [CustomEditor(typeof(AudioTriggerScript)), DefaultEditor]
    public class AudioTriggerEditor : GenericEditor
    {
        private Label _stateLabel;

        public override void Initialize(LayoutElementsContainer layout)
        {
            base.Initialize(layout);

            if (!Editor.Instance.StateMachine.IsPlayMode)
                return;

            var playbackGroup = layout.Group("Playback");
            playbackGroup.Panel.Open();

            _stateLabel = playbackGroup.Label("State: ---").Label;
            _stateLabel.AutoHeight = true;

            var grid = playbackGroup.UniformGrid();
            var gridControl = grid.CustomControl;
            gridControl.ClipChildren = false;
            gridControl.Height = Button.DefaultHeight;
            gridControl.SlotsHorizontally = 2;
            gridControl.SlotsVertically = 1;

            grid.Button("Play").Button.Clicked += () =>
            {
                foreach (var value in Values)
                    if (value is AudioTriggerScript trigger)
                        trigger.Play();
            };

            grid.Button("Stop").Button.Clicked += () =>
            {
                foreach (var value in Values)
                    if (value is AudioTriggerScript trigger)
                        trigger.Stop();
            };
        }

        public override void Refresh()
        {
            base.Refresh();

            if (_stateLabel == null) return;

            var text = string.Empty;
            foreach (var value in Values)
            {
                if (value is AudioTriggerScript trigger)
                {
                    text += $"State: {trigger.GetTriggerState()}\n";
                }
            }
            _stateLabel.Text = text.TrimEnd('\n');
        }
    }
}
