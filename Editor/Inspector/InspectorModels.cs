using System.Collections.Generic;

namespace Editor.Inspector
{
    public class FieldViewModel
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public string Value { get; set; }

        /// <summary>
        /// Populated when this field is a struct (e.g. Float3, Quaternion).
        /// When non-empty, <see cref="Value"/> is ignored.
        /// </summary>
        public List<FieldViewModel> Children { get; set; } = new();

        public bool IsStruct => Children.Count > 0;
    }

    public class ComponentViewModel
    {
        public string Name { get; set; }
        public List<FieldViewModel> Fields { get; set; } = new();
    }
}