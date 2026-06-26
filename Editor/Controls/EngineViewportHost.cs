using System;
using System.Diagnostics;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Platform;
using Avalonia.Threading;
using Editor.Interop;

namespace Editor.Controls;

public class EngineViewportHost : NativeControlHost
{
    private IPlatformHandle? _controlHandle;
    private bool _initialized;
    private DispatcherTimer? _timer;
    private readonly Stopwatch _stopwatch = new();

    protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
    {
        _controlHandle = base.CreateNativeControlCore(parent);
        Dispatcher.UIThread.Post(TryInitializeEngine, DispatcherPriority.Loaded);
        return _controlHandle;
    }

    protected override void DestroyNativeControlCore(IPlatformHandle control)
    {
        StopRenderLoop();
        ShutdownEngine();
        base.DestroyNativeControlCore(control);
        _controlHandle = null;
    }

    protected override void OnSizeChanged(SizeChangedEventArgs e)
    {
        base.OnSizeChanged(e);

        if (!_initialized)
            return;

        var dpi = TopLevel.GetTopLevel(this)?.RenderScaling;
        var w = Math.Max(1, (int)(dpi * e.NewSize.Width)!);
        var h = Math.Max(1, (int)(dpi * e.NewSize.Height)!);
        EngineNative.Resize(w, h);
    }

    private void TryInitializeEngine()
    {
        if (_initialized || _controlHandle is null)
            return;

        if (!OperatingSystem.IsWindows())
        {
            Console.WriteLine("EngineViewportHost: embedded engine is supported on Windows only.");
            return;
        }

        var w = Math.Max(1, (int)Bounds.Width);
        var h = Math.Max(1, (int)Bounds.Height);
        if (w <= 1 || h <= 1)
            return;

        try
        {
            if (!EngineNative.Initialize(_controlHandle.Handle, w, h))
            {
                Console.WriteLine("EngineViewportHost: Engine_Initialize returned false.");
                return;
            }
        }
        catch (DllNotFoundException ex)
        {
            Console.WriteLine($"EngineViewportHost: {ex.Message}");
            return;
        }

        _initialized = true;
        StartRenderLoop();
    }

    private void StartRenderLoop()
    {
        StopRenderLoop();
        _stopwatch.Restart();
        _timer = new DispatcherTimer
        {
            Interval = TimeSpan.FromMilliseconds(16),
        };
        _timer.Tick += OnRenderTick;
        _timer.Start();
    }

    private void OnRenderTick(object? sender, EventArgs e)
    {
        if (!_initialized)
            return;

        var dt = (float)_stopwatch.Elapsed.TotalSeconds;
        _stopwatch.Restart();
        EngineNative.Update(dt);
    }

    private void StopRenderLoop()
    {
        if (_timer is null)
            return;

        _timer.Tick -= OnRenderTick;
        _timer.Stop();
        _timer = null;
    }

    private void ShutdownEngine()
    {
        if (!_initialized)
            return;

        EngineNative.Shutdown();
        _initialized = false;
    }
}
