//
// Created by gorev on 11.03.2026.
//

#include <iostream>

#include "Engine/Engine.h"
#include "Engine/include/Render/RenderSystem.h"
#include "Tools/Logger.h"


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
            if (wParam != SIZE_MINIMIZED)
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                RTGDEngine::RTGDRenderSystem::Instance().Resize(width, height);
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
        0, className, "Triangle Test",
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
        LogError("Window Creation Failed!");
        return 1;
    }
    g_hwnd = hwnd;

    if (!RTGDEngine::Engine::Instance().Initialize(hwnd))
    {
        LogError("Failed to initialize engine!");
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

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

        if (g_running)
        {
            RTGDEngine::Engine::Instance().Render();
        }
    }


    RTGDEngine::Engine::Instance().Shutdown();

    DestroyWindow(hwnd);
    return 0;
}
