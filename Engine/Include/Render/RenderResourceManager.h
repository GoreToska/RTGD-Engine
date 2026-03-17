//
// Created by gorev on 12.03.2026.
//

#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Buffer.h"
#include "PipelineState.h"
#include "RefCntAutoPtr.hpp"
#include "RenderHandle.h"
#include "Texture.h"


namespace Diligent
{
    struct IRenderDevice;
}

namespace RTGDEngine
{
    struct VertexPNUV;

    struct MeshData
    {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
    };

    struct MaterialData
    {
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> PSO;
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> SRB;

        TextureHandle DiffuseTexture = INVALID_TEXTURE_HANDLE;
    };

    struct PendingGPUUpload
    {
        MeshHandle Handle;
        std::vector<VertexPNUV> Vertices;
        std::vector<uint32_t> Indices;
    };

    struct TextureData
    {
        Diligent::RefCntAutoPtr<Diligent::ITexture> Texture;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> SRV;
        Diligent::RefCntAutoPtr<Diligent::ISampler> Sampler;
    };

    struct PendingTextureUpload
    {
        TextureHandle Handle;
        std::vector<uint8_t> Pixels;
        uint32_t Width;
        uint32_t Height;
        uint32_t Channels;
        bool IsSRGB = true;
    };

    struct PendingTextureBind
    {
        MaterialHandle MatHandle;
        TextureHandle  TexHandle;
    };

    class RenderResourceManager
    {
    public:
        static RenderResourceManager& Instance();

        MeshHandle RegisterMesh(const std::string& name, MeshData data);

        MaterialHandle RegisterMaterial(const std::string& name, MaterialData data);

        TextureHandle RegisterTexture(const std::string& name, TextureData data);

        [[nodiscard]] const MeshData& GetMesh(MeshHandle handle) const;

        [[nodiscard]] const MaterialData& GetMaterial(MaterialHandle handle) const;

        const TextureData& GetTexture(TextureHandle handle) const;

        [[nodiscard]] MeshHandle GetMeshByName(const std::string& name) const;

        [[nodiscard]] MaterialHandle GetMaterialByName(const std::string& name) const;

        void QueueMeshUpload(MeshHandle handle, std::vector<VertexPNUV> vertices, std::vector<uint32_t> indices);

        void QueueTextureUpload(TextureHandle handle, std::vector<uint8_t> pixels, uint32_t width,
                                uint32_t height,
                                uint32_t channels,
                                bool isSRGB = true);

        void QueueTextureBind(MaterialHandle mat, TextureHandle tex);

        void FlushMeshUploads(Diligent::IRenderDevice& device);

        void FlushTextureUploads(Diligent::IRenderDevice& device, Diligent::IDeviceContext& context);

        void UpdateMesh(MeshHandle handle, MeshData data);

        void UpdateTexture(TextureHandle handle, TextureData data);

        void BindTextureToMaterial(MaterialHandle matHandle, TextureHandle texHandle);

    private:
        RenderResourceManager() = default;

        void RebindPendingMaterials(TextureHandle texHandle);

        std::vector<MeshData> m_meshes;
        std::vector<MaterialData> m_materials;
        std::vector<TextureData> m_textures;

        std::unordered_map<std::string, MeshHandle> m_meshNames;
        std::unordered_map<std::string, MaterialHandle> m_materialNames;
        std::unordered_map<std::string, TextureHandle> m_textureNames;

        std::mutex m_textureUploadMutex;
        std::mutex m_uploadMutex;
        std::mutex                      m_bindMutex;

        std::vector<PendingGPUUpload> m_pendingUploads;
        std::vector<PendingTextureUpload> m_pendingTextureUploads;
        std::vector<PendingTextureBind> m_pendingBinds;
    };
} // RTGDEngine
