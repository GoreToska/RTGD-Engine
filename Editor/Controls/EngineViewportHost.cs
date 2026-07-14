using System;
using System.Diagnostics;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Platform;
using Avalonia.Threading;
using Editor.Interop;

namespace Editor.Controls;

public class EngineViewportHost : NativeControlHost
{
    private IPlatformHandle? _controlHandle;
    private bool _initialized;
    private bool _viewportFocused;
    private bool _looking;
    private Point _lastPos;
    private DispatcherTimer? _timer;
    private readonly Stopwatch _stopwatch = new();

    public event Action? EngineInitialized;
    public event Action? EngineShutdown;

    public EngineViewportHost()
    {
        Focusable = true;
    }

    protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
    {
        base.OnAttachedToVisualTree(e);

        var top = TopLevel.GetTopLevel(this);
        top?.AddHandler(KeyDownEvent, OnTopLevelKeyDown,
            RoutingStrategies.Tunnel, handledEventsToo: true);
        top?.AddHandler(KeyUpEvent, OnTopLevelKeyUp,
            RoutingStrategies.Tunnel, handledEventsToo: true);

        top?.AddHandler(PointerPressedEvent, OnTopLevelPointerDown,
            RoutingStrategies.Tunnel, handledEventsToo: true);

        top?.AddHandler(PointerReleasedEvent, OnTopLevelPointerUp,
            RoutingStrategies.Tunnel, handledEventsToo: true);

        top?.AddHandler(PointerMovedEvent, OnTopLevelPointerMove,
            RoutingStrategies.Tunnel, handledEventsToo: true);
    }

    private bool IsInsideViewport(PointerEventArgs e)
    {
        var p = e.GetPosition(this);
        return p is { X: >= 0, Y: >= 0 } && p.X <= Bounds.Width && p.Y <= Bounds.Height;
    }

    private void OnTopLevelPointerUp(object? sender, PointerReleasedEventArgs e)
    {
        if (!_initialized) return;

        var kind = e.GetCurrentPoint(this).Properties.PointerUpdateKind;
        if (kind.TryMouseMap(out var button, out var isDown))
            EngineNative.InjectMouseButton((int)button, isDown);

        if (kind == PointerUpdateKind.RightButtonReleased)
            EndLook(e);
    }

    private void OnTopLevelPointerDown(object? sender, PointerPressedEventArgs e)
    {
        if (!_initialized) return;

        _viewportFocused = IsInsideViewport(e);
        if (!_viewportFocused) return;

        Focus();
        var kind = e.GetCurrentPoint(this).Properties.PointerUpdateKind;
        if (kind.TryMouseMap(out var button, out var isDown))
            EngineNative.InjectMouseButton((int)button, isDown);

        if (kind == PointerUpdateKind.RightButtonPressed)
            BeginLook(e);

        if (kind == PointerUpdateKind.LeftButtonPressed)
        {
            var scaling = TopLevel.GetTopLevel(this)?.RenderScaling ?? 1.0;
            var p = e.GetPosition(this);
            var id = EngineNative.PickEntity((int)(p.X * scaling), (int)(p.Y * scaling));
            Console.WriteLine("Picked entity: " + id);
        }
    }

    private void BeginLook(PointerEventArgs e)
    {
        if (_looking) return;

        _looking = true;
        e.Pointer.Capture(this);
        EngineNative.SetCursorVisible(false);
        EngineNative.WarpCursorToCenter();
        _lastPos = new Point(Bounds.Width / 2, Bounds.Height / 2);
    }

    private void EndLook(PointerEventArgs e)
    {
        if (!_looking) return;

        _looking = false;
        e.Pointer.Capture(null);
        EngineNative.SetCursorVisible(true);
    }

    private void OnTopLevelPointerMove(object? sender, PointerEventArgs e)
    {
        if (!_initialized || !_looking) return;

        var pos = e.GetPosition(this);
        var dx = pos.X - _lastPos.X;
        var dy = pos.Y - _lastPos.Y;
        _lastPos = pos;

        if (dx != 0 || dy != 0)
            EngineNative.InjectMouseMove((float)dx, (float)dy);

        var marginX = Bounds.Width / 4;
        var marginY = Bounds.Height / 4;
        if (pos.X < marginX || pos.X > Bounds.Width - marginX ||
            pos.Y < marginY || pos.Y > Bounds.Height - marginY)
        {
            EngineNative.WarpCursorToCenter();
            _lastPos = new Point(Bounds.Width / 2, Bounds.Height / 2);
        }
    }

    protected override void OnPointerCaptureLost(PointerCaptureLostEventArgs e)
    {
        base.OnPointerCaptureLost(e);

        if (!_looking) return;

        _looking = false;
        EngineNative.SetCursorVisible(true);
    }

    private void OnTopLevelKeyDown(object? sender, KeyEventArgs e)
    {
        if (!_initialized || !_viewportFocused) return;

        var key = e.Key.ToEngineKey();
        if (key == EngineKey.Unknown)
            return;

        EngineNative.InjectKey((int)key, isDown: true);
        e.Handled = true;
    }

    private void OnTopLevelKeyUp(object? sender, KeyEventArgs e)
    {
        if (!_initialized) return;

        var key = e.Key.ToEngineKey();
        if (key == EngineKey.Unknown)
            return;

        EngineNative.InjectKey((int)key, isDown: false);
    }

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
        EngineInitialized?.Invoke();
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
        EngineShutdown?.Invoke();
    }
}