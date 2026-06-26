using Editor.ViewModels.Panels;

namespace Editor.ViewModels;

public partial class EditorViewModel : ViewModelBase
{
    public HierarchyViewModel Hierarchy { get; } = new();
    public ViewportViewModel Viewport { get; } = new();
    public InspectorViewModel Inspector { get; } = new();
    public AssetBrowserViewModel AssetBrowser { get; } = new();
}
