using Flax.Build;
using Flax.Build.NativeCpp;

public class AudioSystem : GameModule
{
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);
        BuildNativeCode = true;
        options.PublicDependencies.Add("Core");    // Actor, Script, Thread, CriticalSection, Semaphore
        options.PublicDependencies.Add("Engine");  // Engine update hook, scripting types
        options.PublicDependencies.Add("Physics"); // Physics::RayCast for obstruction/occlusion rays
    }
}
