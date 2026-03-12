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

        public EngineViewport(int width, int height)
        {
            _width = width;
            _height = height;
        }


        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            _hwndChild = CreateChildWindow(hwndParent.Handle, _width, _height);

            if (_hwndChild == IntPtr.Zero)
            {
                int error = Marshal.GetLastWin32Error();
                throw new InvalidOperationException(
                    $"CreateWindowEx failed. Win32 error: {error} — {GetWin32ErrorMessage(error)}");
            }

            _engineHandle = NativeInterop.Engine_Create();
            if (_engineHandle == IntPtr.Zero)
                throw new InvalidOperationException("Engine_Create returned null");

            bool ok = NativeInterop.Engine_Initialize(_engineHandle, _hwndChild, _width, _height);
            if (!ok)
                throw new InvalidOperationException("Engine_Initialize failed");

            System.Windows.Media.CompositionTarget.Rendering += OnRendering;

            return new HandleRef(this, _hwndChild);
        }

        private static string GetWin32ErrorMessage(int code)
        {
            return new System.ComponentModel.Win32Exception(code).Message;
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            System.Windows.Media.CompositionTarget.Rendering -= OnRendering;

            if (_engineHandle != IntPtr.Zero)
            {
                NativeInterop.Engine_Shutdown(_engineHandle);
                NativeInterop.Engine_Destroy(_engineHandle);
                _engineHandle = IntPtr.Zero;
            }

            NativeInterop.DestroyWindow(hwnd.Handle);
        }

        private void OnRendering(object sender, EventArgs e)
        {
            if (_engineHandle != IntPtr.Zero)
                NativeInterop.Engine_Render(_engineHandle);
        }

        public void Resize(int width, int height)
        {
            _width = width;
            _height = height;

            if (_hwndChild != IntPtr.Zero)
                NativeInterop.SetWindowPos(_hwndChild, IntPtr.Zero, 0, 0, width, height,
                    NativeInterop.SWP_NOZORDER | NativeInterop.SWP_NOACTIVATE);

            if (_engineHandle != IntPtr.Zero)
                NativeInterop.Engine_Resize(_engineHandle, width, height);
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