using System.Collections.Generic;
using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using Editor.Models;

namespace Editor.ViewModels.Panels;

public partial class HierarchyViewModel : ViewModelBase
{
    public ObservableCollection<Entity> Entities { get; } = new();

    [ObservableProperty]
    private Entity? selectedEntity;

    public void SetEntities(IEnumerable<Entity> roots)
    {
        Entities.Clear();
        foreach (var entity in roots)
            Entities.Add(entity);

        if (SelectedEntity is not null && !Contains(Entities, SelectedEntity))
            SelectedEntity = null;
    }

    public void Clear()
    {
        Entities.Clear();
        SelectedEntity = null;
    }

    private static bool Contains(IEnumerable<Entity> nodes, Entity entity)
    {
        foreach (var node in nodes)
        {
            if (node.Id == entity.Id)
                return true;

            if (Contains(node.Children, entity))
                return true;
        }

        return false;
    }
}
