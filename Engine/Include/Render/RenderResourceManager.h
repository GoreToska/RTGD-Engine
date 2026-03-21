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
    struct VertexPNTUV;

    enum class ETextureSlot : uint8_t
    {
        Diffuse,
        Normal,
        MetallicRoughness,
        AO
    };

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
        TextureHandle NormalTexture = INVALID_TEXTURE_HANDLE;
        TextureHandle MetallicRoughnessTexture = INVALID_TEXTURE_HANDLE;
        TextureHandle AOTexture = INVALID_TEXTURE_HANDLE;

        float Metallic = 0.0f;
        float Roughness = 0.5f;
        float AO = 1.0f;
    };

    struct PendingGPUUpload
    {
        MeshHandle Handle;
        std::vector<VertexPNTUV> Vertices;
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
        TextureHandle TexHandle;
        ETextureSlot Slot = ETextureSlot::Diffuse;
    };

    class RenderResourceManager
    {
    public:
        static RenderResourceManager& Instance();

        void Initialize(Diligent::IRenderDevice& device, Diligent::IDeviceContext& context);

        MeshHandle RegisterMesh(const std::string& name, MeshData data);

        MaterialHandle RegisterMaterial(const std::string& name, MaterialData data);

        TextureHandle RegisterTexture(const std::string& name, TextureData data);

        TextureHandle RegisterTexture(const std::string& name, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        [[nodiscard]] const MeshData& GetMesh(MeshHandle handle) const;

        [[nodiscard]] const MaterialData& GetMaterial(MaterialHandle handle) const;

        [[nodiscard]] const TextureData& GetTexture(TextureHandle handle) const;

        [[nodiscard]] MeshHandle GetMeshByName(const std::string& name) const;

        [[nodiscard]] MaterialHandle GetMaterialByName(const std::string& name) const;

        void QueueMeshUpload(MeshHandle handle, std::vector<VertexPNTUV> vertices, std::vector<uint32_t> indices);

        void QueueTextureUpload(TextureHandle handle, std::vector<uint8_t> pixels, uint32_t width,
                                uint32_t height,
                                uint32_t channels,
                                bool isSRGB = true);

        void QueueTextureBind(MaterialHandle mat, TextureHandle tex, ETextureSlot slot = ETextureSlot::Diffuse);

        void FlushMeshUploads(Diligent::IRenderDevice& device);

        void FlushTextureUploads(Diligent::IRenderDevice& device, Diligent::IDeviceContext& context);

        void UpdateMesh(MeshHandle handle, MeshData data);

        void UpdateTexture(TextureHandle handle, TextureData data);

        void BindTexturesToMaterial(MaterialHandle matHandle,
                                    TextureHandle albedo, TextureHandle normal,
                                    TextureHandle metallicRoughness, TextureHandle ao);

        void BindTextureToMaterial(MaterialHandle matHandle, TextureHandle texHandle,
                                   ETextureSlot slot = ETextureSlot::Diffuse);

        TextureHandle GetDefaultTextureHandle() const { return m_defaultTexture; }
        TextureHandle GetDefaultNormalTextureHandle() const { return m_defaultNormalTexture; }

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
        std::mutex m_bindMutex;

        std::vector<PendingGPUUpload> m_pendingUploads;
        std::vector<PendingTextureUpload> m_pendingTextureUploads;
        std::vector<PendingTextureBind> m_pendingBinds;

        Diligent::IRenderDevice* m_device = nullptr;
        Diligent::IDeviceContext* m_context = nullptr;

        TextureHandle m_defaultTexture = INVALID_TEXTURE_HANDLE;
        TextureHandle m_defaultNormalTexture = INVALID_TEXTURE_HANDLE;
    };
} // RTGDEngine
