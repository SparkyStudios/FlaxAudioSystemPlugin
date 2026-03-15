using Flax.Build;
using Flax.Build.NativeCpp;

public class AudioSystemEditor : GameEditorModule
{
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);
        BuildNativeCode = true;
        options.PublicDependencies.Add("AudioSystem");
        options.PublicDependencies.Add("Editor");
    }
}
