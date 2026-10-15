//
// Created by ivan on 10/15/26.
//

#include "UI/UiRenderInterface.h"

#include "AssetLoader/PathResolve.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    bool UiRenderInterface::Initialize()
    {
        using namespace Diligent;

        auto& device = GRenderSystem().GetDevice();

        MaterialHandle material = PipelineFactory::CreateUIPipeline(device, GRenderSystem().GetSwapChain(),
                                                                    GetAbsolutePath("Shaders"));

        if (material == INVALID_MATERIAL_HANDLE)
        {
            LogError("Failed to create UI material");
            return false;
        }

        m_pso = GRenderResources().GetMaterial(material).PSO;

        BufferDesc cbDesc;
        cbDesc.Name = "UI Constants";
        cbDesc.Usage = USAGE_DYNAMIC;
        cbDesc.BindFlags = BIND_UNIFORM_BUFFER;
        cbDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        cbDesc.Size = sizeof(Matrix4);
        device.CreateBuffer(cbDesc, nullptr, &m_constants);
        m_pso->GetStaticVariableByName(SHADER_TYPE_VERTEX, "UIConstants")->Set(m_constants);

        constexpr uint32_t white = 0xFFFFFFFF;
        m_white = CreateTexture(&white, 1, 1);
        return m_white != 0;
    }

    void UiRenderInterface::BeginFrame(Diligent::IDeviceContext& context, int width, int height)
    {
        m_context = &context;
        m_width = width;
        m_height = height;
        m_projection = Matrix4::OrthoOffCenter(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 0.0f,
                                               1.0f, GRenderSystem().GetDevice().GetDeviceInfo().IsGLDevice());

        m_scissorEnabled = false;
        ApplyScissor();
    }

    Rml::CompiledGeometryHandle UiRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                                                                   Rml::Span<const int> indices)
    {
        using namespace Diligent;

        if (vertices.empty() || indices.empty())
            return 0;

        auto& device = GRenderSystem().GetDevice();
        Geometry geometry;

        BufferDesc vbDesc;
        vbDesc.Name = "UI VB";
        vbDesc.Usage = USAGE_IMMUTABLE;
        vbDesc.BindFlags = BIND_VERTEX_BUFFER;
        vbDesc.Size = vertices.size() * sizeof(Rml::Vertex);

        BufferData vbData{vertices.data(), vbDesc.Size};
        device.CreateBuffer(vbDesc, &vbData, &geometry.VB);

        BufferDesc ibDesc;
        ibDesc.Name = "UI IB";
        ibDesc.Usage = USAGE_IMMUTABLE;
        ibDesc.BindFlags = BIND_INDEX_BUFFER;
        ibDesc.Size = indices.size() * sizeof(int);

        BufferData ibData{indices.data(), ibDesc.Size};
        device.CreateBuffer(ibDesc, &ibData, &geometry.IB);

        if (!geometry.VB || !geometry.IB)
            return 0;

        geometry.NumIndices = static_cast<uint32_t>(indices.size());
        m_geometry.emplace(m_nextHandle, std::move(geometry));
        return m_nextHandle++;
    }

    void UiRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation,
                                           Rml::TextureHandle texture)
    {
        using namespace Diligent;

        auto geometryIt = m_geometry.find(handle);
        auto textureIt = m_textures.find(texture ? texture : m_white);

        if (!m_context || geometryIt == m_geometry.end() || textureIt == m_textures.end())
            return;

        const Matrix4 transform = Matrix4::Translation(translation.x, translation.y, 0.0f) * m_projection;
        void* mapped = nullptr;
        m_context->MapBuffer(m_constants, MAP_WRITE, MAP_FLAG_DISCARD, mapped);
        if (!mapped)
            return;

        *static_cast<Matrix4*>(mapped) = transform.Transpose();
        m_context->UnmapBuffer(m_constants, MAP_WRITE);

        const Geometry& geometry = geometryIt->second;
        m_context->SetPipelineState(m_pso);
        m_context->CommitShaderResources(textureIt->second.SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        IBuffer* vbs[] = {geometry.VB};
        Uint64 offsets[] = {0};
        m_context->SetVertexBuffers(0, 1, vbs, offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                    SET_VERTEX_BUFFERS_FLAG_RESET);
        m_context->SetIndexBuffer(geometry.IB, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs draw;
        draw.NumIndices = geometry.NumIndices;
        draw.IndexType = VT_UINT32;
        draw.Flags = DRAW_FLAG_VERIFY_ALL;
        m_context->DrawIndexed(draw);
    }

    void UiRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
    {
        m_geometry.erase(geometry);
    }

    Rml::TextureHandle UiRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
    {
        LogWarn("UI image loading not implemented: {}", source);
        return 0;
    }

    Rml::TextureHandle UiRenderInterface::GenerateTexture(Rml::Span<const unsigned char> source,
                                                          Rml::Vector2i source_dimensions)
    {
        return CreateTexture(source.data(), source_dimensions.x, source_dimensions.y);
    }

    void UiRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
    {
        m_textures.erase(texture);
    }

    void UiRenderInterface::EnableScissorRegion(bool enable)
    {
        m_scissorEnabled = enable;
        ApplyScissor();
    }

    void UiRenderInterface::SetScissorRegion(Rml::Rectanglei region)
    {
        m_scissor = region;
        ApplyScissor();
    }

    Rml::TextureHandle UiRenderInterface::CreateTexture(const void* rgba, int width, int height)
    {
        using namespace Diligent;

        TextureDesc desc;
        desc.Name = "UI Texture";
        desc.Type = RESOURCE_DIM_TEX_2D;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.Format = TEX_FORMAT_RGBA8_UNORM;
        desc.Usage = USAGE_IMMUTABLE;
        desc.BindFlags = BIND_SHADER_RESOURCE;

        TextureSubResData subres{rgba, static_cast<Uint64>(width) * 4};
        Diligent::TextureData data{&subres, 1};

        Texture texture;
        GRenderSystem().GetDevice().CreateTexture(desc, &data, &texture.Tex);
        if (!texture.Tex)
            return 0;

        m_pso->CreateShaderResourceBinding(&texture.SRB, true);
        texture.SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(
            texture.Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

        m_textures.emplace(m_nextHandle, std::move(texture));
        return m_nextHandle++;
    }

    void UiRenderInterface::ApplyScissor()
    {
        using namespace Diligent;

        if (!m_context)
            return;

        Rect rect = m_scissorEnabled
                        ? Rect{m_scissor.Left(), m_scissor.Top(), m_scissor.Right(), m_scissor.Bottom()}
                        : Rect{0, 0, m_width, m_height};
        m_context->SetScissorRects(1, &rect, m_width, m_height);
    }
} // RTGDEngine
