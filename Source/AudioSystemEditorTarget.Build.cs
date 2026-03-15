using Flax.Build;

public class AudioSystemEditorTarget : GameProjectEditorTarget
{
    /// <inheritdoc />
    public override void Init()
    {
        base.Init();

        // Reference the modules for editor
        Modules.Add("AudioSystem");
        Modules.Add("AudioSystemEditor");
    }
}
