using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Editor.Inspector
{
    public class FieldViewModel
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public string Value { get; set; }
    }

    public class ComponentViewModel
    {
        public string Name { get; set; }
        public List<FieldViewModel> Fields { get; set; } = new();
    }
}
