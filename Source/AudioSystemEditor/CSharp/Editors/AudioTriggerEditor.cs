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
        private Label _occlusionLabel;
        private Label _obstructionLabel;

        public override void Initialize(LayoutElementsContainer layout)
        {
            base.Initialize(layout);

            if (!Editor.Instance.StateMachine.IsPlayMode)
                return;

            var playbackGroup = layout.Group("Playback");
            playbackGroup.Panel.Open();

            _stateLabel = playbackGroup.Label("State: ---").Label;
            _stateLabel.AutoHeight = true;

            _occlusionLabel = playbackGroup.Label("Occlusion: ---").Label;
            _occlusionLabel.AutoHeight = true;

            _obstructionLabel = playbackGroup.Label("Obstruction: ---").Label;
            _obstructionLabel.AutoHeight = true;

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

            if (_stateLabel == null || _occlusionLabel == null || _obstructionLabel == null) return;

            var stateText = string.Empty;
            var occlusionText = string.Empty;
            var obstructionText = string.Empty;

            foreach (var value in Values)
            {
                if (value is AudioTriggerScript trigger)
                {
                    stateText += $"State: {trigger.GetTriggerState()}\n";
                    occlusionText += $"Occlusion: {trigger.GetOcclusion()}\n";
                    obstructionText += $"Obstruction: {trigger.GetObstruction()}\n";
                }
            }

            _stateLabel.Text = stateText.TrimEnd('\n');
            _occlusionLabel.Text = occlusionText.TrimEnd('\n');
            _obstructionLabel.Text = obstructionText.TrimEnd('\n');
        }
    }
}
