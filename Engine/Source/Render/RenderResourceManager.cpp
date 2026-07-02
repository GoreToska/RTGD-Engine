//
// Created by gorev on 12.03.2026.
//

#include "Render/RenderResourceManager.h"

#include "RenderDevice.h"
#include "AssetLoader/AssetManager.h"
#include "Event/EventBus.h"
#include "Event/Events.h"
#include "Render/RenderSystem.h"
#include "Render/Vertex.h"
#include "Tools/Logger.h"

namespace RTGDEngine {
    static const MeshData emptyMesh = {};
    static const MaterialData emptyMaterial = {};
    static const TextureData emptyTexture = {};

    void RenderResourceManager::Initialize(Diligent::IRenderDevice &device, Diligent::IDeviceContext &context) {
        using namespace Diligent;

        m_device = &device;
        m_context = &context;

        uint8_t white[] = {255, 255, 255, 255};

        TextureData data;

        TextureDesc texDesc;
        texDesc.Type = RESOURCE_DIM_TEX_2D;
        texDesc.Width = 1;
        texDesc.Height = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        texDesc.BindFlags = BIND_SHADER_RESOURCE;
        texDesc.Usage = USAGE_DEFAULT;

        TextureSubResData subRes;
        subRes.pData = white;
        subRes.Stride = 4;

        Diligent::TextureData texData;
        texData.pSubResources = &subRes;
        texData.NumSubresources = 1;

        device.CreateTexture(texDesc, &texData, &data.Texture);
        data.SRV = data.Texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        SamplerDesc samplerDesc;
        samplerDesc.MinFilter = FILTER_TYPE_LINEAR;
        samplerDesc.MagFilter = FILTER_TYPE_LINEAR;
        samplerDesc.MipFilter = FILTER_TYPE_LINEAR;
        device.CreateSampler(samplerDesc, &data.Sampler);

        m_defaultTexture = RegisterTexture("__default__", std::move(data));

        TextureData normalData;
        uint8_t flatNormal[] = {128, 128, 255, 255};

        TextureDesc normalTexDesc;
        normalTexDesc.Type = RESOURCE_DIM_TEX_2D;
        normalTexDesc.Width = 1;
        normalTexDesc.Height = 1;
        normalTexDesc.MipLevels = 1;
        normalTexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        normalTexDesc.BindFlags = BIND_SHADER_RESOURCE;
        normalTexDesc.Usage = USAGE_DEFAULT;

        TextureSubResData normalSubRes;
        normalSubRes.pData = flatNormal;
        normalSubRes.Stride = 4;

        Diligent::TextureData normalTexData;
        normalTexData.pSubResources = &normalSubRes;
        normalTexData.NumSubresources = 1;

        device.CreateTexture(normalTexDesc, &normalTexData, &normalData.Texture);
        normalData.SRV = normalData.Texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        SamplerDesc normalSamplerDesc;
        normalSamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
        normalSamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
        normalSamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
        device.CreateSampler(normalSamplerDesc, &normalData.Sampler);
        m_defaultNormalTexture = RegisterTexture("__default_normal__", std::move(normalData));

        LogInfo("Default white texture created → handle {}", m_defaultTexture);
        LogInfo("Default normal texture created → handle {}", m_defaultNormalTexture);
    }

    MeshHandle RenderResourceManager::RegisterMesh(const std::string &name, MeshData data, uint64_t assetID) {
        std::lock_guard lock(m_lifetimeMutex);
        uint32_t index;

        if (!m_meshes.FreeList.empty()) {
            index = m_meshes.FreeList.back();
            m_meshes.FreeList.pop_back();
            auto &slot = m_meshes[index];
            slot.data = std::move(data);
            slot.refCount = 0;
            slot.pendingDestroy = 0;
            slot.assetID = assetID;
        } else {
            index = m_meshes.size();
            m_meshes.Slots.push_back({std::move(data), 0, 0, 0, assetID});
        }

        return MeshHandle{index, m_meshes[index].generation};
    }

    MaterialHandle
    RenderResourceManager::RegisterMaterial(const std::string &name, MaterialData data, uint64_t assetID) {
        std::lock_guard lock(m_lifetimeMutex);
        uint32_t index;

        if (!m_materials.FreeList.empty()) {
            index = m_materials.FreeList.back();
            m_materials.FreeList.pop_back();
            auto &slot = m_materials[index];
            slot.data = std::move(data);
            slot.refCount = 0;
            slot.pendingDestroy = 0;
            slot.assetID = assetID;
        } else {
            index = m_materials.size();
            m_materials.Slots.push_back({std::move(data), 0, 0, 0, assetID});
        }

        return MaterialHandle{index, m_materials[index].generation};
    }

    TextureHandle RenderResourceManager::RegisterTexture(const std::string &name, TextureData data, uint64_t assetID) {
        std::lock_guard lock(m_lifetimeMutex);

        uint32_t index;

        if (!m_textures.FreeList.empty()) {
            index = m_textures.FreeList.back();
            m_textures.FreeList.pop_back();
            auto &slot = m_textures[index];
            slot.data = std::move(data);
            slot.refCount = 0;
            slot.pendingDestroy = 0;
            slot.assetID = assetID;
        } else {
            index = m_textures.size();
            m_textures.Slots.push_back({std::move(data), 0, 0, 0, assetID});
        }

        return TextureHandle{index, m_textures[index].generation};
    }

    TextureHandle RenderResourceManager::RegisterTexture(const std::string &name, uint8_t r, uint8_t g, uint8_t b,
                                                         uint8_t a, uint64_t assetID) {
        using namespace Diligent;

        if (auto it = m_textureNames.find(name); it != m_textureNames.end())
            return it->second;

        uint8_t pixels[] = {r, g, b, a};

        TextureData data;

        TextureDesc texDesc;
        texDesc.Type = RESOURCE_DIM_TEX_2D;
        texDesc.Width = 1;
        texDesc.Height = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        texDesc.BindFlags = BIND_SHADER_RESOURCE;
        texDesc.Usage = USAGE_DEFAULT;

        TextureSubResData subRes;
        subRes.pData = pixels;
        subRes.Stride = 4;

        Diligent::TextureData texData;
        texData.pSubResources = &subRes;
        texData.NumSubresources = 1;

        m_device->CreateTexture(texDesc, &texData, &data.Texture);
        data.SRV = data.Texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        SamplerDesc samplerDesc;
        samplerDesc.MinFilter = FILTER_TYPE_POINT;
        samplerDesc.MagFilter = FILTER_TYPE_POINT;
        samplerDesc.MipFilter = FILTER_TYPE_POINT;
        m_device->CreateSampler(samplerDesc, &data.Sampler);

        return RegisterTexture(name, std::move(data), assetID);
    }

    const TextureData &RenderResourceManager::GetTexture(TextureHandle handle) const {
        if (handle.Index() >= m_textures.size() || m_textures[handle.Index()].generation != handle.Generation()) {
            return emptyTexture;
        }

        return m_textures[handle.Index()].data;
    }

    void RenderResourceManager::QueueTextureUpload(TextureHandle handle, std::vector<uint8_t> pixels, uint32_t width,
                                                   uint32_t height, uint32_t channels, bool isSRGB) {
        std::lock_guard lock(m_textureUploadMutex);
        m_pendingTextureUploads.push_back({
            handle, std::move(pixels), width, height, channels, isSRGB
        });
    }

    void RenderResourceManager::QueueTextureBind(MaterialHandle mat, TextureHandle tex, ETextureSlot slot) {
        if (tex.Index() < m_textures.size() && m_textures[tex.Index()].data.SRV) {
            BindTextureToMaterial(mat, tex, slot);
            return;
        }

        std::lock_guard lock(m_bindMutex);
        m_pendingBinds.push_back({mat, tex, slot});
    }

    void RenderResourceManager::FlushTextureUploads(Diligent::IRenderDevice &device,
                                                    Diligent::IDeviceContext &context) {
        using namespace Diligent;

        std::vector<PendingTextureUpload> uploads;
        {
            std::lock_guard lock(m_textureUploadMutex);
            uploads = std::move(m_pendingTextureUploads);
            m_pendingTextureUploads.clear();
        }

        if (uploads.empty())
            return;

        for (auto &upload: uploads) {
            TextureData data;

            TextureDesc texDesc;
            texDesc.Type = RESOURCE_DIM_TEX_2D;
            texDesc.Width = upload.Width;
            texDesc.Height = upload.Height;
            texDesc.MipLevels = 1;
            texDesc.Format = upload.IsSRGB
                                 ? TEX_FORMAT_RGBA8_UNORM_SRGB
                                 : TEX_FORMAT_RGBA8_UNORM;
            texDesc.BindFlags = BIND_SHADER_RESOURCE;
            texDesc.Usage = USAGE_DEFAULT;

            TextureSubResData subRes;
            subRes.pData = upload.Pixels.data();
            subRes.Stride = upload.Width * upload.Channels;

            Diligent::TextureData texData;
            texData.pSubResources = &subRes;
            texData.NumSubresources = 1;

            device.CreateTexture(texDesc, &texData, &data.Texture);

            if (!data.Texture) {
                LogError("FlushTextureUploads: failed to create texture for handle {}",
                         upload.Handle);
                continue;
            }

            data.SRV = data.Texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

            SamplerDesc samplerDesc;
            samplerDesc.MinFilter = FILTER_TYPE_LINEAR;
            samplerDesc.MagFilter = FILTER_TYPE_LINEAR;
            samplerDesc.MipFilter = FILTER_TYPE_LINEAR;
            samplerDesc.AddressU = TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = TEXTURE_ADDRESS_WRAP;
            samplerDesc.MaxAnisotropy = 8;
            device.CreateSampler(samplerDesc, &data.Sampler);

            UpdateTexture(upload.Handle, std::move(data));

            RebindPendingMaterials(upload.Handle);

            if (IsAlive(upload.Handle))
                EventBus::Instance().Emit(Events::OnAssetLoaded,
                                          {m_textures[upload.Handle.Index()].assetID, EAssetType::Texture}, {});

            LogInfo("FlushTextureUploads: done → handle {}, {}x{}",
                    upload.Handle, upload.Width, upload.Height);
        }
    }

    void RenderResourceManager::UpdateTexture(TextureHandle handle, TextureData data) {
        if (handle == INVALID_TEXTURE_HANDLE || handle.Index() >= m_textures.size() || m_textures[handle.Index()].
            generation != handle.Generation()) {
            LogError("Invalid handle {}", handle);
            return;
        }

        m_textures[handle.Index()].data = std::move(data);
    }

    void RenderResourceManager::BindTexturesToMaterial(MaterialHandle matHandle, TextureHandle albedo,
                                                       TextureHandle normal, TextureHandle metallicRoughness,
                                                       TextureHandle ao) {
        if (matHandle == INVALID_MATERIAL_HANDLE || matHandle.Index() >= m_materials.size() || m_materials[
                matHandle.Index()].generation != matHandle.Generation()) {
            LogError("Invalid handle {}", matHandle);
            return;
        }

        auto &mat = m_materials[matHandle.Index()].data;

        mat.DiffuseTexture = albedo;
        mat.NormalTexture = normal;
        mat.MetallicRoughnessTexture = metallicRoughness;
        mat.AOTexture = ao;
    }

    void RenderResourceManager::BindTextureToMaterial(MaterialHandle matHandle, TextureHandle texHandle,
                                                      ETextureSlot slot) {
        if (matHandle == INVALID_MATERIAL_HANDLE || matHandle.Index() >= m_materials.size() || m_materials[
                matHandle.Index()].generation != matHandle.Generation()) {
            LogError("Invalid handle {}", matHandle);
            return;
        }
        if (texHandle == INVALID_TEXTURE_HANDLE || texHandle.Index() >= m_textures.size() || m_textures[
                texHandle.Index()].generation != texHandle.Generation()) {
            LogError("Invalid handle {}", texHandle);
            return;
        }

        auto &mat = m_materials[matHandle.Index()].data;

        switch (slot) {
            case ETextureSlot::Diffuse:
                mat.DiffuseTexture = texHandle;
                break;
            case ETextureSlot::Normal:
                mat.NormalTexture = texHandle;
                break;
            case ETextureSlot::MetallicRoughness:
                mat.MetallicRoughnessTexture = texHandle;
                break;
            case ETextureSlot::AO:
                mat.AOTexture = texHandle;
                break;
        }

        LogInfo("BindTextureToMaterial: mat={} slot={} tex={}",
                matHandle, static_cast<int>(slot), texHandle);
    }

    bool RenderResourceManager::IsAlive(MeshHandle handle) const {
        return handle.Index() < m_meshes.size() && m_meshes[handle.Index()].generation == handle.Generation();
    }

    bool RenderResourceManager::IsAlive(MaterialHandle handle) const {
        return handle.Index() < m_materials.size() && m_materials[handle.Index()].generation == handle.Generation();
    }

    bool RenderResourceManager::IsAlive(TextureHandle handle) const {
        return handle.Index() < m_textures.size() && m_textures[handle.Index()].generation == handle.Generation();
    }

    void RenderResourceManager::AcquireAsset(MeshHandle handle) {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle)) return;

        ++m_meshes[handle.Index()].refCount;
    }

    void RenderResourceManager::ReleaseAsset(MeshHandle handle) {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle)) return;

        auto &slot = m_meshes[handle.Index()];

        if (slot.refCount == 0) return;
        if (--slot.refCount == 0 && !slot.pendingDestroy) {
            slot.pendingDestroy = 1;
            m_meshes.PendingDestroys.push_back(handle.Index());
        }
    }

    void RenderResourceManager::AcquireAsset(MaterialHandle handle) {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle)) return;

        ++m_materials[handle.Index()].refCount;
    }

    void RenderResourceManager::ReleaseAsset(MaterialHandle handle) {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle)) return;

        auto &slot = m_materials[handle.Index()];

        if (slot.refCount == 0) return;
        if (--slot.refCount == 0 && !slot.pendingDestroy) {
            slot.pendingDestroy = 1;
            m_materials.PendingDestroys.push_back(handle.Index());
        }
    }

    void RenderResourceManager::AcquireAsset(TextureHandle handle) {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle)) return;

        ++m_textures[handle.Index()].refCount;
    }

    void RenderResourceManager::ReleaseAsset(TextureHandle handle) {
        std::lock_guard lock(m_lifetimeMutex);

        if (!IsAlive(handle)) return;

        auto &slot = m_textures[handle.Index()];

        if (slot.refCount == 0) return;
        if (--slot.refCount == 0 && !slot.pendingDestroy) {
            slot.pendingDestroy = 1;
            m_textures.PendingDestroys.push_back(handle.Index());
        }
    }

    void RenderResourceManager::RebindPendingMaterials(TextureHandle texHandle) {
        std::vector<PendingTextureBind> toProcess;
        {
            std::lock_guard lock(m_bindMutex);
            for (auto it = m_pendingBinds.begin(); it != m_pendingBinds.end();) {
                if (it->TexHandle == texHandle) {
                    toProcess.push_back(*it);
                    it = m_pendingBinds.erase(it);
                } else
                    ++it;
            }
        }

        for (auto &bind: toProcess)
            BindTextureToMaterial(bind.MatHandle, bind.TexHandle, bind.Slot);
    }

    void RenderResourceManager::ProcessPendingDestroys() {
        std::vector<DestroyedAsset> destroyed = {};
        {
            std::lock_guard lock(m_lifetimeMutex);

            for (uint32_t i: m_meshes.PendingDestroys) {
                auto &slot = m_meshes[i];
                slot.pendingDestroy = 0;
                if (slot.refCount != 0) continue;

                destroyed.push_back({MeshHandle{i, slot.generation}.value, slot.assetID, EAssetType::Mesh});
                slot.data = {};
                ++slot.generation;
                m_meshes.FreeList.push_back(i);
            }

            m_meshes.PendingDestroys.clear();
        }

        {
            std::lock_guard lock(m_lifetimeMutex);

            for (uint32_t i: m_materials.PendingDestroys) {
                auto &slot = m_materials[i];
                slot.pendingDestroy = 0;
                if (slot.refCount != 0) continue;

                destroyed.push_back({
                    MaterialHandle{i, slot.generation}.value, slot.assetID, EAssetType::Material
                });
                slot.data = {};
                ++slot.generation;
                m_materials.FreeList.push_back(i);
            }

            m_materials.PendingDestroys.clear();
        }

        {
            std::lock_guard lock(m_lifetimeMutex);

            for (uint32_t i: m_textures.PendingDestroys) {
                auto &slot = m_textures[i];
                slot.pendingDestroy = 0;
                if (slot.refCount != 0) continue;

                destroyed.push_back({
                    TextureHandle{i, slot.generation}.value, slot.assetID, EAssetType::Texture
                });
                slot.data = {};
                ++slot.generation;
                m_textures.FreeList.push_back(i);
            }

            m_textures.PendingDestroys.clear();
        }
        for (auto &d: destroyed) {
            if (OnAssetDestroyed)
                OnAssetDestroyed(d.handleValue, d.type);
            if (d.assetId)
                EventBus::Instance().Emit(Events::OnAssetUnloaded, {d.assetId, d.type}, {});
        }
    }

    void RenderResourceManager::MarkMaterialLoaded(MaterialHandle handle, uint64_t assetID) {
        if (!IsAlive(handle)) {
            LogError("Invalid handle");
            return;
        }

        m_materials[handle.Index()].assetID = assetID;
        EventBus::Instance().Emit(Events::OnAssetLoaded, {assetID, EAssetType::Material}, {});
    }

    const MeshData &RenderResourceManager::GetMesh(MeshHandle handle) const {
        if (handle.Index() >= m_meshes.size() || m_meshes[handle.Index()].generation != handle.Generation()) {
            LogError("Mesh handle generation missmatch.");
            return emptyMesh;
        }

        return m_meshes[handle.Index()].data;
    }

    const MaterialData &RenderResourceManager::GetMaterial(MaterialHandle handle) const {
        if (handle.Index() >= m_materials.size() || m_materials[handle.Index()].generation != handle.Generation()) {
            LogError("Material handle generation missmatch.");
            return emptyMaterial;
        }

        return m_materials[handle.Index()].data;
    }

    MeshHandle RenderResourceManager::GetMeshByName(const std::string &name) const {
        auto it = m_meshNames.find(name);
        return it != m_meshNames.end() ? it->second : INVALID_MESH_HANDLE;
    }

    MaterialHandle RenderResourceManager::GetMaterialByName(const std::string &name) const {
        auto it = m_materialNames.find(name);
        return it != m_materialNames.end() ? it->second : INVALID_MATERIAL_HANDLE;
    }

    void RenderResourceManager::QueueMeshUpload(MeshHandle handle, std::vector<VertexPNTUV> vertices,
                                                std::vector<uint32_t> indices) {
        std::lock_guard lock(m_uploadMutex);
        m_pendingUploads.push_back({
            handle,
            std::move(vertices),
            std::move(indices)
        });

        LogInfo("RenderResourceManager: queued GPU upload - handle {}", handle);
    }

    void RenderResourceManager::FlushMeshUploads(Diligent::IRenderDevice &device) {
        using namespace Diligent;

        std::vector<PendingGPUUpload> uploads;
        {
            std::lock_guard lock(m_uploadMutex);
            uploads = std::move(m_pendingUploads);
            m_pendingUploads.clear();
        }

        if (uploads.empty())
            return;

        for (auto &upload: uploads) {
            if (upload.Vertices.empty()) {
                LogWarn("RenderResourceManager: empty vertices for handle {}", upload.Handle);
                continue;
            }

            MeshData data;
            data.VertexCount = static_cast<uint32_t>(upload.Vertices.size());

            // Vertex Buffer
            BufferDesc vbDesc;
            vbDesc.Name = "Mesh VB";
            vbDesc.Size = upload.Vertices.size() * sizeof(VertexPNTUV);
            vbDesc.Usage = USAGE_IMMUTABLE;
            vbDesc.BindFlags = BIND_VERTEX_BUFFER;

            BufferData vbData;
            vbData.pData = upload.Vertices.data();
            vbData.DataSize = vbDesc.Size;
            device.CreateBuffer(vbDesc, &vbData, &data.VertexBuffer);

            // Index Buffer
            if (!upload.Indices.empty()) {
                data.IndexCount = static_cast<uint32_t>(upload.Indices.size());

                BufferDesc ibDesc;
                ibDesc.Name = "Mesh IB";
                ibDesc.Size = upload.Indices.size() * sizeof(uint32_t);
                ibDesc.Usage = USAGE_IMMUTABLE;
                ibDesc.BindFlags = BIND_INDEX_BUFFER;

                BufferData ibData;
                ibData.pData = upload.Indices.data();
                ibData.DataSize = ibDesc.Size;
                device.CreateBuffer(ibDesc, &ibData, &data.IndexBuffer);
            }

            UpdateMesh(upload.Handle, std::move(data));

            if (IsAlive(upload.Handle))
                EventBus::Instance().Emit(Events::OnAssetLoaded,
                                          {m_meshes[upload.Handle.Index()].assetID, EAssetType::Mesh}, {});

            LogInfo("RenderResourceManager: GPU upload done → handle {}, {} vertices, {} indices",
                    upload.Handle,
                    upload.Vertices.size(),
                    upload.Indices.size());
        }
    }

    void RenderResourceManager::UpdateMesh(MeshHandle handle, MeshData data) {
        if (handle == INVALID_MESH_HANDLE || handle.Index() >= m_meshes.size() || m_meshes[handle.Index()].generation
            != handle.Generation()) {
            LogError("Invalid handle {}", handle);
            return;
        }

        m_meshes[handle.Index()].data = std::move(data);
    }
} // RTGDEngine
