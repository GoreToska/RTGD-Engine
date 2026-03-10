using System;
using System.Windows;
using Microsoft.Win32;

namespace Editor
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            Console.WriteLine("[Editor] WPF Editor started!");
        }

        // ✅ ОБЯЗАТЕЛЬНО: Этот метод должен существовать для Button_LoadGame_Click
        private void Button_LoadGame_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog
            {
                Filter = "Game DLL|*.dll",
                InitialDirectory = AppDomain.CurrentDomain.BaseDirectory
            };

            if (dialog.ShowDialog() == true)
            {
                bool loaded = NativeInterop.Engine_LoadGameModule(dialog.FileName);
                Console.WriteLine($"[Editor] Manual load result: {loaded}");
            }
        }
    }
}