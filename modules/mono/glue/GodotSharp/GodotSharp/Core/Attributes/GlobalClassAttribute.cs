using System;

#nullable enable

namespace Godot
{
    /// <summary>
    /// Exposes the target class as a global script class to Godot Engine.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class)]
    public sealed class GlobalClassAttribute : Attribute
    {
        /// <summary>
        /// Optional file path to a custom icon for representing this class in the Godot Editor.
        /// </summary>
        public string? IconPath { get; }

        /// <summary>
        /// Constructs a new GlobalClassAttribute Instance.
        /// </summary>
        /// <param name="iconPath">
        /// An optional file path to a custom icon for representing this class in the Godot Editor.
        /// </param>
        public GlobalClassAttribute(string iconPath = "")
        {
            IconPath = iconPath;
        }
    }
}
