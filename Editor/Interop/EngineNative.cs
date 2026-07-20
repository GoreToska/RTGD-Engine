using System;
using System.Runtime.InteropServices;

namespace Editor.Interop;

internal static class EngineNative
{
    private const string LibName = "Engine";

    private static EntityCreatedDelegate? _onCreated;
    private static EntityDeletedDelegate? _onDestroyed;
    private static EntityRenamedDelegate? _onRenamed;
    private static EntityReparentedDelegate? _onReparented;

    public static event Action<string, ulong, ulong>? EntityCreated;
    public static event Action<ulong>? EntityDeleted;
    public static event Action<string, ulong>? EntityRenamed;
    public static event Action<ulong, ulong, ulong>? EntityReparented;

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate void EntityCallbackDelegate(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string name,
        long id,
        long parentId);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate void EntityCreatedDelegate(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string name,
        ulong id,
        ulong parentId);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate void EntityRenamedDelegate(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string name,
        ulong id);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate void EntityReparentedDelegate(ulong id, ulong oldParentId, ulong newParentId);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate void EntityDeletedDelegate(ulong id);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool Engine_Initialize(IntPtr nativeWindow, int width, int height);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Resize(int width, int height);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_Shutdown();

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_InjectKey(int key, bool down);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_InjectMouseButton(int button, bool down);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_InjectMouseMove(float dx, float dy);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_WarpCursorToCenter();

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_SetCursorVisible(bool visible);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_GetEntities(EntityCallbackDelegate callback);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern ulong Engine_PickEntity(int x, int y);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_RenameEntity(ulong id, string name);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_CreateEntity(string name);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_DeleteEntity(ulong id);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_ReparentEntity(ulong id, ulong parentId);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_SetEntityCreatedCallback(EntityCreatedDelegate context);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_SetEntityDestroyedCallback(EntityDeletedDelegate context);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_SetEntityRenamedCallback(EntityRenamedDelegate context);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void Engine_SetEntityReparentedCallback(EntityReparentedDelegate context);

    public static bool Initialize(IntPtr nativeWindow, int width, int height) =>
        Engine_Initialize(nativeWindow, width, height);

    public static void RegisterEntityCallbacks()
    {
        _onCreated = (name, id, parent) => EntityCreated?.Invoke(name, id, parent);
        _onDestroyed = id => EntityDeleted?.Invoke(id);
        _onRenamed = (name, id) => EntityRenamed?.Invoke(name, id);
        _onReparented = (id, oldP, newP) => EntityReparented?.Invoke(id, oldP, newP);

        Engine_SetEntityCreatedCallback(_onCreated);
        Engine_SetEntityDestroyedCallback(_onDestroyed);
        Engine_SetEntityRenamedCallback(_onRenamed);
        Engine_SetEntityReparentedCallback(_onReparented);
    }

    public static void Resize(int width, int height) =>
        Engine_Resize(width, height);

    public static void Shutdown() =>
        Engine_Shutdown();

    public static void InjectKey(int key, bool isDown) =>
        Engine_InjectKey(key, isDown);

    public static void InjectMouseButton(int button, bool isDown) =>
        Engine_InjectMouseButton(button, isDown);

    public static void InjectMouseMove(float dx, float dy) =>
        Engine_InjectMouseMove(dx, dy);

    public static void WarpCursorToCenter() =>
        Engine_WarpCursorToCenter();

    public static void SetCursorVisible(bool visible) =>
        Engine_SetCursorVisible(visible);

    public static void GetEntities(Action<string, long, long> callback) =>
        Engine_GetEntities((name, id, parentId) => callback(name, id, parentId));

    public static ulong PickEntity(int x, int y) => Engine_PickEntity(x, y);

    public static void RenameEntity(ulong id, string name) => Engine_RenameEntity(id, name);

    public static void CreateEntity(string name) => Engine_CreateEntity(name);
    public static void DeleteEntity(ulong id) => Engine_DeleteEntity(id);
    public static void ReparentEntity(ulong id, ulong parentId) => Engine_ReparentEntity(id, parentId);
}