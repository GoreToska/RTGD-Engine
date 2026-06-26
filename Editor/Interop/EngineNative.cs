using System;
using System.Runtime.InteropServices;

namespace Editor.Interop;

internal static class EngineNative
{
    private const string LibName = "Engine";

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool Engine_Initialize(IntPtr nativeWindow, int width, int height);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Update(float deltaTime);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Resize(int width, int height);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Shutdown();

    public static bool Initialize(IntPtr nativeWindow, int width, int height) =>
        Engine_Initialize(nativeWindow, width, height);

    public static void Update(float deltaTime) =>
        Engine_Update(deltaTime);

    public static void Resize(int width, int height) =>
        Engine_Resize(width, height);

    public static void Shutdown() =>
        Engine_Shutdown();
}
