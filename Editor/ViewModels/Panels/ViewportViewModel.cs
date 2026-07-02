using System;

namespace Editor.ViewModels.Panels;

public class ViewportViewModel : ViewModelBase
{
    public bool IsEngineReady { get; private set; }

    public event Action? EngineInitialized;
    public event Action? EngineShutdown;

    internal void NotifyEngineInitialized()
    {
        IsEngineReady = true;
        EngineInitialized?.Invoke();
    }

    internal void NotifyEngineShutdown()
    {
        IsEngineReady = false;
        EngineShutdown?.Invoke();
    }
}
