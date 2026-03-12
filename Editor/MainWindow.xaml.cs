using System;
using System.Windows;
using Microsoft.Win32;

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
            ViewportBorder.SizeChanged += OnViewportSizeChanged;

            int w = (int)ViewportBorder.ActualWidth;
            int h = (int)ViewportBorder.ActualHeight;

            if (w <= 0) w = 800;
            if (h <= 0) h = 600;

            _viewport = new EngineViewport(w, h);
            ViewportBorder.Child = _viewport;
        }

        private void OnViewportSizeChanged(object sender, SizeChangedEventArgs e)
        {
            _viewport?.Resize((int)e.NewSize.Width, (int)e.NewSize.Height);
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (_viewport == null) return;

            int w = (int)ViewportBorder.ActualWidth;
            int h = (int)ViewportBorder.ActualHeight;
            _viewport.Resize(w, h);
        }
    }
}