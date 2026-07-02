using System.Collections.Generic;
using CommunityToolkit.Mvvm.ComponentModel;
using Editor.Interop;
using Editor.Models;
using Editor.ViewModels.Panels;

namespace Editor.ViewModels;

public partial class EditorViewModel : ViewModelBase
{
    public HierarchyViewModel Hierarchy { get; } = new();
    public ViewportViewModel Viewport { get; } = new();
    public InspectorViewModel Inspector { get; } = new();
    public AssetBrowserViewModel AssetBrowser { get; } = new();

    [ObservableProperty]
    private string? activeScenePath;

    public EditorViewModel()
    {
        Viewport.EngineInitialized += SyncHierarchy;
        Viewport.EngineShutdown += Hierarchy.Clear;

        Hierarchy.SetEntities(EntityTree.CreateSample());
    }

    partial void OnActiveScenePathChanged(string? value) => SyncHierarchy();

    private void SyncHierarchy()
    {
#if DEBUG
                Hierarchy.SetEntities(EntityTree.CreateSample());
                return;
#endif
        if (!Viewport.IsEngineReady)
            return;

        Hierarchy.SetEntities(QueryActiveSceneEntities());
    }

    private static IReadOnlyList<Entity> QueryActiveSceneEntities()
    {
        var nodes = new List<EntityNode>();
        EngineNative.GetEntities((name, id, parentId) =>
            nodes.Add(new EntityNode(name, id, parentId)));
        return EntityTree.Build(nodes);
    }
}
