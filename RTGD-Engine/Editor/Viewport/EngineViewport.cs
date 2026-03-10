using System;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Runtime.InteropServices;

namespace Editor
{
    public class EngineViewport : HwndHost
    {
        private IntPtr _hwnd;
        private bool _isInitialized = false;

        // === DependencyProperty для Background ===
        public static readonly DependencyProperty BackgroundProperty =
            DependencyProperty.Register(
                nameof(Background),
                typeof(Brush),
                typeof(EngineViewport),
                new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.AffectsRender));

        public Brush Background
        {
            get => (Brush)GetValue(BackgroundProperty);
            set => SetValue(BackgroundProperty, value);
        }

        // === Win32 API импорты ===
        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr CreateWindowEx(
            int dwExStyle, string lpClassName, string lpWindowName,
            int dwStyle, int x, int y, int nWidth, int nHeight,
            IntPtr hWndParent, IntPtr hMenu, IntPtr hInstance, int lpParam);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool DestroyWindow(IntPtr hwnd);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern int SetWindowLong(IntPtr hWnd, int nIndex, uint dwNewLong);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll")]
        private static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndNewParent);

        // === Константы стилей ===
        private const int GWL_STYLE = -16;
        private const int WS_CHILD = 0x40000000;
        private const int WS_VISIBLE = 0x10000000;
        private const int WS_CLIPCHILDREN = 0x02000000;
        private const int WS_CLIPSIBLINGS = 0x04000000;

        // === Обязательные переопределения HwndHost ===
        protected override int VisualChildrenCount => 1;

        protected override Visual GetVisualChild(int index)
        {
            if (index != 0) throw new ArgumentOutOfRangeException(nameof(index));
            return _visualWrapper ??= new HwndSourceVisualWrapper(_hwnd);
        }

        private HwndSourceVisualWrapper _visualWrapper;

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            // 1. Создаём нативное окно для рендеринга
            _hwnd = CreateWindowEx(
                0,                              // dwExStyle
                "STATIC",                       // lpClassName
                "",                             // lpWindowName
                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, // dwStyle
                0, 0, (int)ActualWidth, (int)ActualHeight,
                hwndParent.Handle,              // hWndParent
                IntPtr.Zero, IntPtr.Zero, 0
            );

            if (_hwnd == IntPtr.Zero)
            {
                Console.WriteLine("[Editor] Failed to create native window!");
                return new HandleRef(this, IntPtr.Zero);
            }

            // 2. 🔥 КЛЮЧЕВОЙ ШАГ: Явно устанавливаем стиль WS_CHILD через SetWindowLong
            // Это требование WPF для HwndHost
            int currentStyle = GetWindowLong(_hwnd, GWL_STYLE);
            int newStyle = currentStyle | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
            SetWindowLong(_hwnd, GWL_STYLE, (uint)newStyle);

            // 3. Привязываем окно к родителю WPF
            SetParent(_hwnd, hwndParent.Handle);

            // 4. Инициализируем движок с этим окном
            _isInitialized = NativeInterop.Engine_Initialize(_hwnd);
            if (_isInitialized)
            {
                Console.WriteLine("[Editor] Engine initialized successfully!");

                // Автозагрузка игры
                string gamePath = System.IO.Path.Combine(
                    AppDomain.CurrentDomain.BaseDirectory, "Game.dll");

                if (System.IO.File.Exists(gamePath))
                {
                    bool loaded = NativeInterop.Engine_LoadGameModule(gamePath);
                    Console.WriteLine(loaded ? "[Editor] Game module loaded!" : "[Editor] Failed to load Game!");
                }
            }

            return new HandleRef(this, _hwnd);
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            if (_isInitialized)
            {
                NativeInterop.Engine_Shutdown();
                Console.WriteLine("[Editor] Engine shutdown.");
                _isInitialized = false;
            }

            if (_hwnd != IntPtr.Zero)
            {
                DestroyWindow(_hwnd);
                _hwnd = IntPtr.Zero;
            }
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
            if (Background != null)
            {
                drawingContext.DrawRectangle(Background, null, new Rect(0, 0, ActualWidth, ActualHeight));
            }
            base.OnRender(drawingContext);
        }

        protected override Size MeasureOverride(Size constraint)
        {
            return new Size(ActualWidth, ActualHeight);
        }

        // === Вспомогательный класс для интеграции с визуальным деревом WPF ===
        private class HwndSourceVisualWrapper : Visual
        {
            private readonly HwndSource _source;

            public HwndSourceVisualWrapper(IntPtr hwnd)
            {
                if (hwnd != IntPtr.Zero)
                {
                    _source = new HwndSource(
                        new HwndSourceParameters
                        {
                            PositionX = 0,
                            PositionY = 0,
                            Width = 1,
                            Height = 1,
                            ParentWindow = hwnd,
                            WindowStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN
                        });
                    _source.AddHook(WndProc);
                }
            }

            private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
            {
                return IntPtr.Zero;
            }

            protected override int VisualChildrenCount => 0;
        }
    }
}