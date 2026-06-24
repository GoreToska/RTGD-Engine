namespace Editor.ViewModels;

public partial class MainViewModel : ViewModelBase
{
    public EditorViewModel EditorPage { get; } = new();
}
