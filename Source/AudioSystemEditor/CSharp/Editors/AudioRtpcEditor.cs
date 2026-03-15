using AudioSystem;
using FlaxEditor;
using FlaxEditor.CustomEditors;
using FlaxEditor.CustomEditors.Editors;
using FlaxEditor.CustomEditors.Elements;
using FlaxEngine;
using FlaxEngine.GUI;

namespace AudioSystemEditor.Editors
{
    [CustomEditor(typeof(AudioRtpcComponent)), DefaultEditor]
    public class AudioRtpcEditor : GenericEditor
    {
        private Slider _slider;
        private Label _valueLabel;

        public override void Initialize(LayoutElementsContainer layout)
        {
            base.Initialize(layout);

            if (!Editor.Instance.StateMachine.IsPlayMode)
                return;

            var group = layout.Group("Runtime Value");
            group.Panel.Open();

            _valueLabel = group.Label("Value: 0.0").Label;
            _valueLabel.AutoHeight = true;

            var sliderElement = group.Slider();
            _slider = sliderElement.Slider;
            _slider.Minimum = 0.0f;
            _slider.Maximum = 100.0f;

            // Seed slider with the current live value
            foreach (var value in Values)
            {
                if (value is AudioRtpcComponent rtpc)
                {
                    _slider.Value = rtpc.GetValue();
                    _valueLabel.Text = $"Value: {rtpc.GetValue():F2}";
                    break;
                }
            }

            _slider.ValueChanged += OnSliderChanged;
        }

        private void OnSliderChanged()
        {
            if (_slider == null) return;

            foreach (var value in Values)
            {
                if (value is AudioRtpcComponent rtpc)
                {
                    rtpc.SetValue(_slider.Value);
                }
            }
        }

        public override void Refresh()
        {
            base.Refresh();

            if (_valueLabel == null) return;

            foreach (var value in Values)
            {
                if (value is AudioRtpcComponent rtpc)
                {
                    _valueLabel.Text = $"Value: {rtpc.GetValue():F2}";
                    break;
                }
            }
        }
    }
}
