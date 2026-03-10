using System;
using System.Runtime.InteropServices;

namespace Editor
{
    public static class NativeInterop
    {
        private const string EngineDll = "Engine.dll";

        [DllImport(EngineDll, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool Engine_Initialize(IntPtr hwnd);

        [DllImport(EngineDll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Engine_Shutdown();

        [DllImport(EngineDll, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool Engine_LoadGameModule([MarshalAs(UnmanagedType.LPStr)] string dllPath);

        [DllImport(EngineDll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Engine_Update(float deltaTime);

        [DllImport(EngineDll, CallingConvention = CallingConvention.Cdecl)]
        public static extern void Engine_Render();
    }
}