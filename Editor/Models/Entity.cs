using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;

namespace Editor.Models;

public partial class Entity : ObservableObject
{
    public required long Id { get; init; }

    [ObservableProperty]
    private string _name = string.Empty;

    public ObservableCollection<Entity> Children { get; } = new();
}
