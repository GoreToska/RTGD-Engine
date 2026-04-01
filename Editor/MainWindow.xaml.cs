using Editor.Scene;
using Microsoft.Win32;
using System;
using System.Windows;
using System.Windows.Media;

namespace Editor
{
    public partial class MainWindow : Window
    {
        private EngineViewport _viewport;

        public MainWindow()
        {
            InitializeComponent();
            Loaded += OnLoaded;
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            double dpiScaleX = VisualTreeHelper.GetDpi(this).DpiScaleX;
            double dpiScaleY = VisualTreeHelper.GetDpi(this).DpiScaleY;

            int width = (int)(ViewportBorder.ActualWidth * dpiScaleX);
            int height = (int)(ViewportBorder.ActualHeight * dpiScaleY);

            _viewport = new EngineViewport(width, height);
            ViewportBorder.Child = _viewport;

            ViewportBorder.SizeChanged += OnViewportSizeChanged;

            Dispatcher.BeginInvoke(
                System.Windows.Threading.DispatcherPriority.Background,
                () => HierarchyPanel.Refresh());
        }

        private void OnViewportSizeChanged(object sender, SizeChangedEventArgs e)
        {
            double dpiScaleX = VisualTreeHelper.GetDpi(this).DpiScaleX;
            double dpiScaleY = VisualTreeHelper.GetDpi(this).DpiScaleY;

            int width = (int)(e.NewSize.Width * dpiScaleX);
            int height = (int)(e.NewSize.Height * dpiScaleY);

            if (width <= 0 || height <= 0) return;

            _viewport?.Resize(width, height);
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (_viewport == null) return;

            int w = (int)ViewportBorder.ActualWidth;
            int h = (int)ViewportBorder.ActualHeight;
            _viewport.Resize(w, h);
        }

        public void OnEntitySelected(EntityItem item)
        {
            Console.WriteLine($"Выбрана сущность: {item.Name}");
        }
    }
}