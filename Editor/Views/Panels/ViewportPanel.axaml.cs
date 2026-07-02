using Avalonia.Controls;
using Editor.ViewModels.Panels;

namespace Editor.Views.Panels;

public partial class ViewportPanel : UserControl
{
    public ViewportPanel()
    {
        InitializeComponent();
        Loaded += OnLoaded;
    }

    private void OnLoaded(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
    {
        if (DataContext is not ViewportViewModel vm)
            return;

        ViewportHost.EngineInitialized += vm.NotifyEngineInitialized;
        ViewportHost.EngineShutdown += vm.NotifyEngineShutdown;
    }
}
