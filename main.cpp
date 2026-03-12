//
// Created by gorev on 11.03.2026.
//

#include <iostream>

#include "Engine/include/Render/RenderSystem.h"


static RTGDEngine::RTGDRenderSystem* g_renderSystem = nullptr;
static bool g_running = true;
static HWND g_hwnd = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
            g_running = false;
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            if (g_renderSystem && wParam != SIZE_MINIMIZED)
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                g_renderSystem->Resize(width, height);
            }
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

HWND CreateWindowHandle(HINSTANCE hInstance, int width, int height)
{
    const char* className = "TriangleTestClass";

    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .hInstance = hInstance,
        .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = (HBRUSH) (COLOR_WINDOW + 1),
        .lpszClassName = className,
        .hIconSm = LoadIcon(nullptr, IDI_APPLICATION)
    };

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return nullptr;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, style, FALSE);

    return CreateWindowEx(
        0, className, "Magnum Vulkan Triangle Test",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr
    );
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    const int width = 1280;
    const int height = 720;

    HWND hwnd = CreateWindowHandle(hInstance, width, height);
    if (!hwnd)
    {
        std::cerr << "Failed to create window!" << std::endl;
        return 1;
    }
    g_hwnd = hwnd;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    g_renderSystem = new RTGDEngine::RTGDRenderSystem();

    if (!g_renderSystem->Initialize((void*) hwnd, width, height))
    {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        delete g_renderSystem;
        return 1;
    }


    MSG msg = {};
    while (g_running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                g_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (g_running && g_renderSystem)
        {
            g_renderSystem->BeginFrame();
            g_renderSystem->EndFrame();
        }
    }

    if (g_renderSystem)
    {
        g_renderSystem->Shutdown();
        delete g_renderSystem;
        g_renderSystem = nullptr;
    }

    DestroyWindow(hwnd);
    return 0;
}
