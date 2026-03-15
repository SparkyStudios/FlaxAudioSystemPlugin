using Flax.Build;

public class AudioSystemTarget : GameProjectTarget
{
    /// <inheritdoc />
    public override void Init()
    {
        base.Init();

        // Reference the modules for game
        Modules.Add("AudioSystem");
    }
}
