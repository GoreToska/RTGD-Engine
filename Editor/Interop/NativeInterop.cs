using System;
using System.Runtime.InteropServices;

namespace Editor
{
    public static class NativeInterop
    {
        public const int WM_SIZING = 0x0214;
        public const int WM_SIZE = 0x0005;
        public const int WM_MOVE = 0x0003;
        public const uint WS_CHILD = 0x40000000;
        public const uint WS_VISIBLE = 0x10000000;
        public const uint SWP_NOZORDER = 0x0004;
        public const uint SWP_NOACTIVATE = 0x0010;
        public const uint WS_CLIPCHILDREN = 0x02000000;
        public const uint WS_CLIPSIBLINGS = 0x04000000;
        public const int CS_HREDRAW = 0x0002;
        public const int CS_VREDRAW = 0x0001;
        public const int COLOR_WINDOW = 5;
        public const uint WM_LBUTTONDOWN = 0x0201;
        public const uint WM_RBUTTONDOWN = 0x0204;

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void EntityCallback([MarshalAs(UnmanagedType.LPStr)] string name, ulong id);

        [DllImport("Engine.dll")] public static extern bool Engine_Initialize(IntPtr hwnd);
        [DllImport("Engine.dll")] public static extern void Engine_Update(float deltaTime);
        [DllImport("Engine.dll")] public static extern void Engine_HandleMessage(IntPtr hwnd, uint msg, IntPtr wParam, IntPtr lParam);
        [DllImport("Engine.dll")] public static extern void Engine_Resize(int w, int h);
        [DllImport("Engine.dll")] public static extern void Engine_Shutdown();
        [DllImport("Engine.dll")] public static extern void Engine_GetEntities(EntityCallback callback);
        [DllImport("Engine.dll")] public static extern void Engine_FreeString(IntPtr str);

        // Inspector component level
        [DllImport("Engine.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int Inspector_GetComponentCount(ulong entityId);

        [DllImport("Engine.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr Inspector_GetComponentName(ulong entityId, int index);

        // Inspector field level
        [DllImport("Engine.dll")] public static extern int Inspector_GetFieldCount(int compIndex);
        [DllImport("Engine.dll")] public static extern IntPtr Inspector_GetFieldName(int compIndex, int fieldIndex);
        [DllImport("Engine.dll")] public static extern IntPtr Inspector_GetFieldType(int compIndex, int fieldIndex);
        [DllImport("Engine.dll")] public static extern IntPtr Inspector_GetFieldValue(int compIndex, int fieldIndex);

        // Inspector sub-field level (Float3.x, Quaternion.w)
        [DllImport("Engine.dll")] public static extern int Inspector_GetSubFieldCount(int compIndex, int fieldIndex);
        [DllImport("Engine.dll")] public static extern IntPtr Inspector_GetSubFieldName(int compIndex, int fieldIndex, int subIndex);
        [DllImport("Engine.dll")] public static extern IntPtr Inspector_GetSubFieldType(int compIndex, int fieldIndex, int subIndex);
        [DllImport("Engine.dll")] public static extern IntPtr Inspector_GetSubFieldValue(int compIndex, int fieldIndex, int subIndex);

        // Inspector write-back 
        /// <returns>1 on success, 0 on parse/index error.</returns>
        [DllImport("Engine.dll")] public static extern int Inspector_SetFieldValue(int compIndex, int fieldIndex, [MarshalAs(UnmanagedType.LPStr)] string value);
        [DllImport("Engine.dll")] public static extern int Inspector_SetSubFieldValue(int compIndex, int fieldIndex, int subIndex, [MarshalAs(UnmanagedType.LPStr)] string value);

        // Inspector live refresh (engine → editor)
        /// Re-reads all cached values from ECS memory.
        /// Returns 0 if the cache is empty.
        [DllImport("Engine.dll")] public static extern int Inspector_RefreshValues();

        public static string GetComponentName(ulong entityId, int index)
            => Marshal.PtrToStringAnsi(Inspector_GetComponentName(entityId, index)) ?? "";

        public static string GetFieldName(int c, int f) => Marshal.PtrToStringAnsi(Inspector_GetFieldName(c, f)) ?? "";
        public static string GetFieldType(int c, int f) => Marshal.PtrToStringAnsi(Inspector_GetFieldType(c, f)) ?? "";
        public static string GetFieldValue(int c, int f) => Marshal.PtrToStringAnsi(Inspector_GetFieldValue(c, f)) ?? "";

        public static string GetSubFieldName(int c, int f, int s) => Marshal.PtrToStringAnsi(Inspector_GetSubFieldName(c, f, s)) ?? "";
        public static string GetSubFieldType(int c, int f, int s) => Marshal.PtrToStringAnsi(Inspector_GetSubFieldType(c, f, s)) ?? "";
        public static string GetSubFieldValue(int c, int f, int s) => Marshal.PtrToStringAnsi(Inspector_GetSubFieldValue(c, f, s)) ?? "";

        // ── Win32 ───────────────────────────────────────────────────────────────
        [DllImport("user32.dll")] public static extern IntPtr SetFocus(IntPtr hWnd);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern ushort RegisterClassEx(ref WNDCLASSEX lpwcx);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern IntPtr CreateWindowEx(uint dwExStyle, string lpClassName,
            string lpWindowName, uint dwStyle,
            int x, int y, int nWidth, int nHeight,
            IntPtr hWndParent, IntPtr hMenu, IntPtr hInstance, IntPtr lpParam);

        [DllImport("user32.dll")] public static extern bool DestroyWindow(IntPtr hwnd);

        [DllImport("user32.dll")]
        public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
            int X, int Y, int cx, int cy, uint uFlags);

        [DllImport("kernel32.dll")] public static extern IntPtr GetModuleHandle(string lpModuleName);
        [DllImport("kernel32.dll")] public static extern void SetLastError(uint dwErrCode);

        [DllImport("user32.dll")]
        public static extern IntPtr DefWindowProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

        public static WndProcDelegate _wndProcDelegate = CustomWndProc;
        public delegate IntPtr WndProcDelegate(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

        private static IntPtr CustomWndProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam)
        {
            Engine_HandleMessage(hWnd, msg, wParam, lParam);
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }

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