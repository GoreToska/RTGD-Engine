using System.Runtime.InteropServices;

namespace Editor.Shared.EngineAPI;

public class EngineLibrary
{
    private const string EngineDLLName = "Engine.dll";

    [DllImport(EngineDLLName)]
    public static extern void Engine_Hello();

    [DllImport(EngineDLLName)]
    public static extern unsafe bool Engine_Initialize(void* nativeWindow, int width, int height);

    [DllImport(EngineDLLName)]
    public static extern void Engine_Update(float deltaTime);

    [DllImport(EngineDLLName)]
    public static extern void Engine_InjectKey(int key, bool down);

    [DllImport(EngineDLLName)]
    public static extern void Engine_InjectMouseButton(int button, bool down);

    [DllImport(EngineDLLName)]
    public static extern void Engine_InjectMousePosition(float normX, float normY);

    [DllImport(EngineDLLName)]
    public static extern void Engine_Resize(int w, int h);

    [DllImport(EngineDLLName)]
    public static extern void Engine_Shutdown();
}