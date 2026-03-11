#include "Render/RenderSystem.h"
#include <iostream>

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/StringView.h>
#include <Corrade/Utility/Assert.h>
#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Magnum.h>
#include <Magnum/Mesh.h>
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/VertexFormat.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Range.h>
#include <Magnum/ShaderTools/AbstractConverter.h>
#include <Magnum/Trade/AbstractImageConverter.h>
#include <Magnum/Vk/BufferCreateInfo.h>
#include <Magnum/Vk/CommandBuffer.h>
#include <Magnum/Vk/CommandPoolCreateInfo.h>
#include <Magnum/Vk/DeviceCreateInfo.h>
#include <Magnum/Vk/DeviceProperties.h>
#include <Magnum/Vk/Fence.h>
#include <Magnum/Vk/FramebufferCreateInfo.h>
#include <Magnum/Vk/ImageCreateInfo.h>
#include <Magnum/Vk/ImageViewCreateInfo.h>
#include <Magnum/Vk/InstanceCreateInfo.h>
#include <Magnum/Vk/Memory.h>
#include <Magnum/Vk/Mesh.h>
#include <Magnum/Vk/Pipeline.h>
#include <Magnum/Vk/PipelineLayoutCreateInfo.h>
#include <Magnum/Vk/Queue.h>
#include <Magnum/Vk/RasterizationPipelineCreateInfo.h>
#include <Magnum/Vk/RenderPassCreateInfo.h>
#include <Magnum/Vk/ShaderCreateInfo.h>
#include <Magnum/Vk/ShaderSet.h>


namespace RTGDEngine
{
    bool RTGDRenderSystem::Initialize(void* hwnd, int width, int height)
    {
        m_width = width;
        m_height = height;

        m_hdc = GetDC((HWND) hwnd);
        if (!m_hdc)
        {
            std::cerr << "[Renderer] Failed to get DC!" << std::endl;
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA,
            32, // Цветовые биты
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            24, // Z-буфер
            8, // Stencil
            0,
            PFD_MAIN_PLANE,
            0, 0, 0, 0
        };

        int format = ChoosePixelFormat(m_hdc, &pfd);
        if (!format)
        {
            std::cerr << "[Renderer] Failed to choose pixel format!" << std::endl;
            return false;
        }

        if (!SetPixelFormat(m_hdc, format, &pfd))
        {
            std::cerr << "[Renderer] Failed to set pixel format!" << std::endl;
            return false;
        }

        m_hglrc = wglCreateContext(m_hdc);
        if (!m_hglrc)
        {
            std::cerr << "[Renderer] Failed to create OpenGL context!" << std::endl;
            return false;
        }

        if (!wglMakeCurrent(m_hdc, m_hglrc))
        {
            std::cerr << "[Renderer] Failed to make context current!" << std::endl;
            return false;
        }

        m_glContext = new Magnum::GL::Context(m_hdc);


        // === 3. Настройка рендерера ===
        Magnum::GL::Renderer::enable(Magnum::GL::Renderer::Feature::DepthTest);
        Magnum::GL::Renderer::enable(Magnum::GL::Renderer::Feature::FaceCulling);
        Magnum::GL::Renderer::setClearColor(Magnum::Color4{0.1f, 0.2f, 0.4f, 1.0f}); // Синий фон

        std::cout << "[Renderer] Magnum initialized with FIXED camera!" << std::endl;
        m_initialized = true;
        return true;

        std::cout << "[Renderer] Initialized with FIXED camera!" << std::endl;
        return true;
    }

    void RTGDEngine::RTGDRenderSystem::CreateHelloTriangle()
    {
    }

    void RTGDRenderSystem::Resize(int width, int height)
    {
    }

    void RTGDRenderSystem::Render()
    {
    }

    void RTGDRenderSystem::Shutdown()
    {
    }
}
