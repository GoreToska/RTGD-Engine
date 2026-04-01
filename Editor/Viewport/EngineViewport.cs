using System;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;
using System.Runtime.InteropServices;

namespace Editor
{
    public class EngineViewport : HwndHost
    {
        private IntPtr _hwndChild = IntPtr.Zero;
        private IntPtr _engineHandle = IntPtr.Zero;

        private int _width;
        private int _height;

        private DateTime _lastFrameTime = DateTime.Now;

        private System.Threading.Thread _renderThread;
        private volatile bool _running = false;

        public EngineViewport(int width, int height)
        {
            _width = width;
            _height = height;
        }

        protected override IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            if (msg == NativeInterop.WM_LBUTTONDOWN || msg == NativeInterop.WM_RBUTTONDOWN)
                NativeInterop.SetFocus(hwnd);

            switch (msg)
            {
                case NativeInterop.WM_SIZING:
                case NativeInterop.WM_SIZE:
                case NativeInterop.WM_MOVE:
                    UpdateViewportFromWindow();
                    break;
            }

            NativeInterop.Engine_HandleMessage(hwnd, (uint)msg, wParam, lParam);
            return NativeInterop.DefWindowProc(hwnd, (uint)msg, wParam, lParam);
        }

        private void UpdateViewportFromWindow()
        {
            
        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            _hwndChild = CreateChildWindow(hwndParent.Handle, _width, _height);

            if (_hwndChild == IntPtr.Zero)
            {
                int error = Marshal.GetLastWin32Error();
                throw new InvalidOperationException(
                    $"CreateWindowEx failed. Win32 error: {error}");
            }

            if (!NativeInterop.Engine_Initialize(_hwndChild))
                throw new InvalidOperationException("Engine_Initialize failed");

            MouseDown += (s, e) => NativeInterop.SetFocus(_hwndChild);

            _running = true;
            _renderThread = new System.Threading.Thread(RenderLoop)
            {
                IsBackground = true,
                Name = "EngineUpdateThread"
            };
            _renderThread.Start();

            return new HandleRef(this, _hwndChild);
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            _running = false;
            _renderThread?.Join(1000);

            NativeInterop.Engine_Shutdown();
            NativeInterop.DestroyWindow(hwnd.Handle);
        }

        private void RenderLoop()
        {
            var lastTime = DateTime.Now;

            while (_running)
            {
                var now = DateTime.Now;
                float deltaTime = (float)(now - lastTime).TotalSeconds;
                lastTime = now;

                NativeInterop.Engine_Update(deltaTime);
            }
        }

        private static string GetWin32ErrorMessage(int code)
        {
            return new System.ComponentModel.Win32Exception(code).Message;
        }

        private void OnRendering(object sender, EventArgs e)
        {
            var now = DateTime.Now;
            float deltaTime = (float)(now - _lastFrameTime).TotalSeconds;
            _lastFrameTime = now;

            NativeInterop.Engine_Update(deltaTime);
        }

        public void Resize(int width, int height)
        {
            _width = width;
            _height = height;

            if (_hwndChild != IntPtr.Zero)
                NativeInterop.SetWindowPos(_hwndChild, IntPtr.Zero, 0, 0, width, height,
                    NativeInterop.SWP_NOZORDER | NativeInterop.SWP_NOACTIVATE);

            NativeInterop.Engine_Resize(width, height);
        }

        private static bool _classRegistered = false;
        private const string ClassName = "DiligentChildWindow";

        private static IntPtr CreateChildWindow(IntPtr hwndParent, int width, int height)
        {
            if (!_classRegistered)
            {
                var wc = new NativeInterop.WNDCLASSEX
                {
                    cbSize = Marshal.SizeOf<NativeInterop.WNDCLASSEX>(),
                    style = NativeInterop.CS_HREDRAW | NativeInterop.CS_VREDRAW,
                    lpfnWndProc = Marshal.GetFunctionPointerForDelegate(NativeInterop._wndProcDelegate),
                    hInstance = NativeInterop.GetModuleHandle(null),
                    lpszClassName = ClassName,
                    hbrBackground = (IntPtr)(NativeInterop.COLOR_WINDOW + 1),
                };

                ushort atom = NativeInterop.RegisterClassEx(ref wc);
                if (atom == 0)
                {
                    int err = Marshal.GetLastWin32Error();
                    if (err != 1410)
                        throw new InvalidOperationException(
                            $"RegisterClassEx failed: {err}");
                }
                _classRegistered = true;
            }


            IntPtr hwnd = NativeInterop.CreateWindowEx(
                0,
                ClassName,
                "",
                NativeInterop.WS_CHILD | NativeInterop.WS_VISIBLE | NativeInterop.WS_CLIPCHILDREN | NativeInterop.WS_CLIPSIBLINGS,
                0, 0,
                width > 0 ? width : 100,
                height > 0 ? height : 100,
                hwndParent,
                IntPtr.Zero,
                NativeInterop.GetModuleHandle(null),
                IntPtr.Zero
            );

            return hwnd;
        }
    }
}