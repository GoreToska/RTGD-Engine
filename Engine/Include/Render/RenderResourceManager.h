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
#include "Vertex.h"
#include "AssetLoader/AssetType.h"
#include "Tools/RTGDMacros.h"


namespace Diligent {
    struct IRenderDevice;
}

namespace RTGDEngine {
    enum class ETextureSlot : uint8_t {
        Diffuse,
        Normal,
        MetallicRoughness,
        AO
    };

    struct MeshData {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
        uint32_t VertexCount = 0;
        uint32_t IndexCount = 0;
    };

    struct MaterialData {
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

    struct PendingGPUUpload {
        MeshHandle Handle;
        std::vector<VertexPNTUV> Vertices;
        std::vector<uint32_t> Indices;
    };

    struct TextureData {
        Diligent::RefCntAutoPtr<Diligent::ITexture> Texture;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> SRV;
        Diligent::RefCntAutoPtr<Diligent::ISampler> Sampler;
    };

    struct PendingTextureUpload {
        TextureHandle Handle;
        std::vector<uint8_t> Pixels;
        uint32_t Width;
        uint32_t Height;
        uint32_t Channels;
        bool IsSRGB = true;
    };

    struct PendingTextureBind {
        MaterialHandle MatHandle;
        TextureHandle TexHandle;
        ETextureSlot Slot = ETextureSlot::Diffuse;
    };

    class RenderResourceManager {
        DECLARE_SINGLETON(RenderResourceManager);

    public:
        void Initialize(Diligent::IRenderDevice &device, Diligent::IDeviceContext &context);

        MeshHandle RegisterMesh(const std::string &name, MeshData data, uint64_t assetID = 0);

        MaterialHandle RegisterMaterial(const std::string &name, MaterialData data, uint64_t assetID = 0);

        TextureHandle RegisterTexture(const std::string &name, TextureData data, uint64_t assetID = 0);

        TextureHandle RegisterTexture(const std::string &name, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                      uint64_t assetID = 0);

        void MarkMaterialLoaded(MaterialHandle handle, uint64_t assetID);

        [[nodiscard]] const MeshData &GetMesh(MeshHandle handle) const;

        [[nodiscard]] const MaterialData &GetMaterial(MaterialHandle handle) const;

        [[nodiscard]] const TextureData &GetTexture(TextureHandle handle) const;

        [[nodiscard]] MeshHandle GetMeshByName(const std::string &name) const;

        [[nodiscard]] MaterialHandle GetMaterialByName(const std::string &name) const;

        void QueueMeshUpload(MeshHandle handle, std::vector<VertexPNTUV> vertices, std::vector<uint32_t> indices);

        void QueueTextureUpload(TextureHandle handle, std::vector<uint8_t> pixels, uint32_t width,
                                uint32_t height,
                                uint32_t channels,
                                bool isSRGB = true);

        void QueueTextureBind(MaterialHandle mat, TextureHandle tex, ETextureSlot slot = ETextureSlot::Diffuse);

        void FlushMeshUploads(Diligent::IRenderDevice &device);

        void FlushTextureUploads(Diligent::IRenderDevice &device, Diligent::IDeviceContext &context);

        void UpdateMesh(MeshHandle handle, MeshData data);

        void UpdateTexture(TextureHandle handle, TextureData data);

        void BindTexturesToMaterial(MaterialHandle matHandle,
                                    TextureHandle albedo, TextureHandle normal,
                                    TextureHandle metallicRoughness, TextureHandle ao);

        void BindTextureToMaterial(MaterialHandle matHandle, TextureHandle texHandle,
                                   ETextureSlot slot = ETextureSlot::Diffuse);

        TextureHandle GetDefaultTextureHandle() const { return m_defaultTexture; }
        TextureHandle GetDefaultNormalTextureHandle() const { return m_defaultNormalTexture; }

        bool IsAlive(MeshHandle handle) const;

        bool IsAlive(MaterialHandle handle) const;

        bool IsAlive(TextureHandle handle) const;

        void AcquireAsset(MeshHandle handle);

        void ReleaseAsset(MeshHandle handle);

        void AcquireAsset(MaterialHandle handle);

        void ReleaseAsset(MaterialHandle handle);

        void AcquireAsset(TextureHandle handle);

        void ReleaseAsset(TextureHandle handle);

        void ProcessPendingDestroys();

        std::function<void(uint32_t, EAssetType)> OnAssetDestroyed = {};

    private:
        void RebindPendingMaterials(TextureHandle texHandle);

        struct DestroyedAsset { uint32_t handleValue; uint64_t assetId; EAssetType type; };

        std::vector<MeshData> m_meshes = {};
        std::vector<uint32_t> m_meshGenerations = {};
        std::vector<uint32_t> m_meshFreeList = {};
        std::vector<uint32_t> m_meshRefCounts = {};
        std::vector<uint8_t> m_meshPendingDestroy = {};
        std::vector<uint64_t> m_meshAssetIds = {};

        std::vector<MaterialData> m_materials = {};
        std::vector<uint32_t> m_materialGenerations = {};
        std::vector<uint32_t> m_materialFreeList = {};
        std::vector<uint32_t> m_materialRefCounts = {};
        std::vector<uint8_t> m_materialPendingDestroy = {};
        std::vector<uint64_t> m_materialAssetIds = {};

        std::vector<TextureData> m_textures = {};
        std::vector<uint32_t> m_textureGenerations = {};
        std::vector<uint32_t> m_textureFreeList = {};
        std::vector<uint32_t> m_textureRefCounts = {};
        std::vector<uint8_t> m_texturePendingDestroy = {};
        std::vector<uint64_t> m_texturesAssetIds = {};

        std::unordered_map<std::string, MeshHandle> m_meshNames = {};
        std::unordered_map<std::string, MaterialHandle> m_materialNames = {};
        std::unordered_map<std::string, TextureHandle> m_textureNames = {};

        std::mutex m_textureUploadMutex = {};
        std::mutex m_uploadMutex = {};
        std::mutex m_bindMutex = {};
        std::mutex m_lifetimeMutex = {};

        std::vector<PendingGPUUpload> m_pendingUploads = {};
        std::vector<PendingTextureUpload> m_pendingTextureUploads = {};
        std::vector<PendingTextureBind> m_pendingBinds = {};
        std::vector<uint32_t> m_pendingMeshDestroys = {};
        std::vector<uint32_t> m_pendingMaterialDestroys = {};
        std::vector<uint32_t> m_pendingTextureDestroys = {};

        Diligent::IRenderDevice *m_device = nullptr;
        Diligent::IDeviceContext *m_context = nullptr;

        TextureHandle m_defaultTexture = INVALID_TEXTURE_HANDLE;
        TextureHandle m_defaultNormalTexture = INVALID_TEXTURE_HANDLE;
    };
} // RTGDEngine
