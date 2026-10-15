//
// Created by ivan on 10/15/26.
//

#pragma once
#include <RmlUi/Core/RenderInterface.h>

#include "DeviceContext.h"
#include "RefCntAutoPtr.hpp"
#include "Tools/Alias.h"

namespace RTGDEngine
{
    class UiRenderInterface final : public Rml::RenderInterface
    {
    public:
        bool Initialize();

        void BeginFrame(Diligent::IDeviceContext& context, int width, int height);

        Rml::CompiledGeometryHandle
        CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;

        void RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation,
                            Rml::TextureHandle texture) override;

        void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

        Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;

        Rml::TextureHandle
        GenerateTexture(Rml::Span<const unsigned char> source, Rml::Vector2i source_dimensions) override;

        void ReleaseTexture(Rml::TextureHandle texture) override;

        void EnableScissorRegion(bool enable) override;

        void SetScissorRegion(Rml::Rectanglei region) override;

    private:
        struct Geometry
        {
            Diligent::RefCntAutoPtr<Diligent::IBuffer> VB;
            Diligent::RefCntAutoPtr<Diligent::IBuffer> IB;
            uint32_t NumIndices = 0;
        };

        struct Texture
        {
            Diligent::RefCntAutoPtr<Diligent::ITexture> Tex;
            Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> SRB;
        };

        Rml::TextureHandle CreateTexture(const void* rgba, int width, int height);

        void ApplyScissor();

        Diligent::IDeviceContext* m_context = nullptr;
        Diligent::IPipelineState* m_pso = nullptr;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_constants;

        std::unordered_map<uintptr_t, Geometry> m_geometry = {};
        std::unordered_map<uintptr_t, Texture> m_textures = {};
        uintptr_t m_nextHandle = 1;
        Rml::TextureHandle m_white = 0;

        Matrix4 m_projection;
        int m_width = 0;
        int m_height = 0;
        bool m_scissorEnabled = false;
        Rml::Rectanglei m_scissor;
    };
} // RTGDEngine
