using System;
using System.Runtime.InteropServices;

namespace Editor.Interop;

internal static class EngineNative
{
    private const string LibName = "Engine";

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate void EntityCallbackDelegate(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string name,
        long id,
        long parentId);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool Engine_Initialize(IntPtr nativeWindow, int width, int height);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Update(float deltaTime);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Resize(int width, int height);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Shutdown();

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_InjectKey(int key, bool down);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_InjectMouseButton(int button, bool down);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_GetEntities(EntityCallbackDelegate callback);

    public static bool Initialize(IntPtr nativeWindow, int width, int height) =>
        Engine_Initialize(nativeWindow, width, height);

    public static void Update(float deltaTime) =>
        Engine_Update(deltaTime);

    public static void Resize(int width, int height) =>
        Engine_Resize(width, height);

    public static void Shutdown() =>
        Engine_Shutdown();

    public static void InjectKey(int key, bool isDown) =>
        Engine_InjectKey(key, isDown);

    public static void InjectMouseButton(int button, bool isDown) =>
        Engine_InjectMouseButton(button, isDown);

    public static void GetEntities(Action<string, long, long> callback) =>
        Engine_GetEntities((name, id, parentId) => callback(name, id, parentId));
}