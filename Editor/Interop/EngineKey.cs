namespace Editor.Interop;

using Avalonia.Input;

internal enum EngineKey
{
    Unknown = -1,

    A = 0x41,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    D0 = 0x30,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,

    F1 = 1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    Escape = 0,
    Space = 0x20,

    Left = 97,
    Right,
    Up,
    Down,
    Insert,
    Home,
    Delete,
    End,
    PageUp,
    PageDown,

    Backspace = 125,
    Tab = 126,
    Enter = 127,
    ShiftLeft = 129,
    CtrlLeft = 130,
    AltLeft = 132,
    AltRight = 133,
    CtrlRight = 136,
    ShiftRight = 137,
}

internal enum EngineMouseButton
{
    Left = 0,
    Middle = 1,
    Right = 2,
    WheelUp = 3,
    WheelDown = 4,
    XButton1 = 5,
    XButton2 = 6,
}

internal static class EngineKeyMap
{
    public static EngineKey ToEngineKey(this Key key) => key switch
    {
        Key.A => EngineKey.A, Key.B => EngineKey.B, Key.C => EngineKey.C,
        Key.D => EngineKey.D, Key.E => EngineKey.E, Key.F => EngineKey.F,
        Key.G => EngineKey.G, Key.H => EngineKey.H, Key.I => EngineKey.I,
        Key.J => EngineKey.J, Key.K => EngineKey.K, Key.L => EngineKey.L,
        Key.M => EngineKey.M, Key.N => EngineKey.N, Key.O => EngineKey.O,
        Key.P => EngineKey.P, Key.Q => EngineKey.Q, Key.R => EngineKey.R,
        Key.S => EngineKey.S, Key.T => EngineKey.T, Key.U => EngineKey.U,
        Key.V => EngineKey.V, Key.W => EngineKey.W, Key.X => EngineKey.X,
        Key.Y => EngineKey.Y, Key.Z => EngineKey.Z,

        Key.D0 => EngineKey.D0, Key.D1 => EngineKey.D1, Key.D2 => EngineKey.D2,
        Key.D3 => EngineKey.D3, Key.D4 => EngineKey.D4, Key.D5 => EngineKey.D5,
        Key.D6 => EngineKey.D6, Key.D7 => EngineKey.D7, Key.D8 => EngineKey.D8,
        Key.D9 => EngineKey.D9,

        Key.F1 => EngineKey.F1, Key.F2 => EngineKey.F2, Key.F3 => EngineKey.F3,
        Key.F4 => EngineKey.F4, Key.F5 => EngineKey.F5, Key.F6 => EngineKey.F6,
        Key.F7 => EngineKey.F7, Key.F8 => EngineKey.F8, Key.F9 => EngineKey.F9,
        Key.F10 => EngineKey.F10, Key.F11 => EngineKey.F11, Key.F12 => EngineKey.F12,

        Key.LeftShift => EngineKey.ShiftLeft,
        Key.RightShift => EngineKey.ShiftRight,
        Key.LeftCtrl => EngineKey.CtrlLeft,
        Key.RightCtrl => EngineKey.CtrlRight,
        Key.LeftAlt => EngineKey.AltLeft,
        Key.RightAlt => EngineKey.AltRight,

        Key.Escape => EngineKey.Escape,
        Key.Tab => EngineKey.Tab,
        Key.Space => EngineKey.Space,
        Key.Enter => EngineKey.Enter,
        Key.Back => EngineKey.Backspace,

        Key.Left => EngineKey.Left,
        Key.Right => EngineKey.Right,
        Key.Up => EngineKey.Up,
        Key.Down => EngineKey.Down,

        Key.Insert => EngineKey.Insert,
        Key.Delete => EngineKey.Delete,
        Key.Home => EngineKey.Home,
        Key.End => EngineKey.End,
        Key.PageUp => EngineKey.PageUp,
        Key.PageDown => EngineKey.PageDown,

        _ => EngineKey.Unknown,
    };

    public static bool TryMouseMap(this PointerUpdateKind kind, out EngineMouseButton button, out bool isDown)
    {
        switch (kind)
        {
            case PointerUpdateKind.LeftButtonPressed:
                button = EngineMouseButton.Left;
                isDown = true;
                return true;
            case PointerUpdateKind.LeftButtonReleased:
                button = EngineMouseButton.Left;
                isDown = false;
                return true;
            case PointerUpdateKind.MiddleButtonPressed:
                button = EngineMouseButton.Middle;
                isDown = true;
                return true;
            case PointerUpdateKind.MiddleButtonReleased:
                button = EngineMouseButton.Middle;
                isDown = false;
                return true;
            case PointerUpdateKind.RightButtonPressed:
                button = EngineMouseButton.Right;
                isDown = true;
                return true;
            case PointerUpdateKind.RightButtonReleased:
                button = EngineMouseButton.Right;
                isDown = false;
                return true;
            case PointerUpdateKind.XButton1Pressed:
                button = EngineMouseButton.XButton1;
                isDown = true;
                return true;
            case PointerUpdateKind.XButton1Released:
                button = EngineMouseButton.XButton1;
                isDown = false;
                return true;
            case PointerUpdateKind.XButton2Pressed:
                button = EngineMouseButton.XButton2;
                isDown = true;
                return true;
            case PointerUpdateKind.XButton2Released:
                button = EngineMouseButton.XButton2;
                isDown = false;
                return true;
            default:
                button = default;
                isDown = false;
                return false;
        }
    }
};