#pragma once

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Context.h>
#include <Magnum/Math/Color.h>

#include <Windows.h>

namespace RTGDEngine
{
    class RTGDRenderSystem
    {
    public:
        bool Initialize(void* hwnd, int width, int height);

        void Shutdown();

        void Render();

        void Resize(int width, int height);

        void CreateHelloTriangle();

    private:
        int m_width = 0;
        int m_height = 0;
        bool m_initialized = false;

        HDC m_hdc = nullptr;
        HGLRC m_hglrc = nullptr;

        Magnum::GL::Context* m_glContext = nullptr;
    };
}
