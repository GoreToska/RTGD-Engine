using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor.Scene
{

    public class EntityItem
    {
        public string Name { get; set; }
        public ulong Id { get; set; }

        public override string ToString() => Name;
    }

    public partial class SceneHierarchyPanel : UserControl
    {
        public ObservableCollection<EntityItem> Entities { get; } = new();

        public event Action<EntityItem> EntitySelected;
        public EntityItem SelectedEntity { get; set; }


        public SceneHierarchyPanel()
        {
            InitializeComponent();
            DataContext = this; 
        }

        public void Refresh()
        {
            Entities.Clear();

            NativeInterop.Engine_GetEntities((name, id) =>
            {
                Dispatcher.Invoke(() =>
                {
                    Entities.Add(new EntityItem { Name = name, Id = id });
                });
            });
        }

        private void OnRefreshClick(object sender, RoutedEventArgs e)
        {
            Refresh();
        }

        private void OnEntitySelected(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0 && e.AddedItems[0] is EntityItem item)
                EntitySelected?.Invoke(item);
        }
    }
}
