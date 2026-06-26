using Avalonia.Controls;
using Editor.Interop;

namespace Editor.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        EngineNative.Hello();
    }
}