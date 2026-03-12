using System;
using System.Runtime.InteropServices;

namespace Editor
{
    public static class NativeInterop
    {
        public const uint WS_CHILD = 0x40000000;
        public const uint WS_VISIBLE = 0x10000000;
        public const uint SWP_NOZORDER = 0x0004;
        public const uint SWP_NOACTIVATE = 0x0010;
        public const uint WS_CLIPCHILDREN = 0x02000000;
        public const uint WS_CLIPSIBLINGS = 0x04000000;
        public const int CS_HREDRAW = 0x0002;
        public const int CS_VREDRAW = 0x0001;
        public const int COLOR_WINDOW = 5;

        [DllImport("Engine.dll")] public static extern IntPtr Engine_Create();
        [DllImport("Engine.dll")] public static extern void Engine_Destroy(IntPtr engine);
        [DllImport("Engine.dll")] public static extern bool Engine_Initialize(IntPtr engine, IntPtr hwnd, int w, int h);
        [DllImport("Engine.dll")] public static extern void Engine_Render(IntPtr engine);
        [DllImport("Engine.dll")] public static extern void Engine_Resize(IntPtr engine, int w, int h);
        [DllImport("Engine.dll")] public static extern void Engine_Shutdown(IntPtr engine);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern ushort RegisterClassEx(ref WNDCLASSEX lpwcx);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr CreateWindowEx(uint dwExStyle, string lpClassName,
            string lpWindowName, uint dwStyle,
            int x, int y, int nWidth, int nHeight,
            IntPtr hWndParent, IntPtr hMenu, IntPtr hInstance, IntPtr lpParam);

        [DllImport("user32.dll")]
        public static extern bool DestroyWindow(IntPtr hwnd);

        [DllImport("user32.dll")]
        public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
            int X, int Y, int cx, int cy, uint uFlags);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll")]
        public static extern void SetLastError(uint dwErrCode);

        [DllImport("user32.dll")]
        public static extern IntPtr DefWindowProc(IntPtr hWnd, uint msg,
            IntPtr wParam, IntPtr lParam);

        public static WndProcDelegate _wndProcDelegate = DefWindowProc;
        public delegate IntPtr WndProcDelegate(IntPtr hWnd, uint msg,
            IntPtr wParam, IntPtr lParam);

        public struct WNDCLASSEX
        {
            public int cbSize;
            public int style;
            public IntPtr lpfnWndProc;
            public int cbClsExtra, cbWndExtra;
            public IntPtr hInstance, hIcon, hCursor, hbrBackground;
            public string lpszMenuName, lpszClassName;
            public IntPtr hIconSm;
        }
    }
}