#pragma warning disable CS0169

namespace Godot.SourceGenerators.Sample
{
    [GlobalClass]
    partial class Global : GodotObject
    {
    }

    [GlobalClass]
    partial class GlobalResource : Resource
    {
    }

    partial class GlobalOuter : GodotObject
    {
        [GlobalClass]
        partial class GlobalInner : GodotObject
        {
        }
    }

    /* Designed to throw errors. Not sure if these should be here as they throw errors when run. Big surprise.
    // Global again but with generic parameters
    [GlobalClass]
    partial class Global<T, R> : GodotObject
    {
    }

    // Global tool
    [Tool]
    [GlobalClass]
    partial class GlobalTool : GodotObject
    {
    }

    // Global non-Godot class
    [GlobalClass]
    partial class GlobalGodont // I'm so funny :')
    {
    }

    // Non-global parent class
    partial class NonGlobalParent : GodotObject
    {
    }

    // Global child class with non-global parent
    [GlobalClass]
    partial class GlobalChild : NonGlobalParent
    {
    }
    */
}
