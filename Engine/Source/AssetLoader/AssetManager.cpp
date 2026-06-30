//
// Created by gorev on 17.03.2026.
//

#include "AssetLoader/AssetManager.h"

#include <filesystem>
#include <fstream>

#include "AssetLoader/MeshImporter.h"
#include "AssetLoader/PathResolve.h"
#include "AssetLoader/TextureImporter.h"
#include "JobSystem/JobSystem.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Tools/Logger.h"

namespace RTGDEngine {
    MeshHandle AssetManager::GetMesh(const std::string &absolutePath, std::function<void(MeshHandle)> onComplete) {
        const std::string key = Normalize(absolutePath);

        MeshHandle handle;
        {
            std::lock_guard lock(m_registryMutex);

            if (auto it = m_meshByPath.find(key); it != m_meshByPath.end()) {
                if (RenderResourceManager::Instance().IsAlive(it->second))
                    return it->second;
                m_meshByPath.erase(it);
            }

            handle = RenderResourceManager::Instance().RegisterMesh(key, MeshData{});
            m_meshByPath[key] = handle;
            m_meshPathByHandle[handle] = key;
        }


        LogInfo("Async load queued '{}' - handle {}", key, handle);

        JobSystem::Instance().Submit([key, handle, onComplete]() {
            MeshImportData data = MeshImporter::Import(key);

            if (!data.Success) {
                LogError("Mesh import failed {}", key);
                return;
            }

            RenderResourceManager::Instance().QueueMeshUpload(
                handle,
                std::move(data.Vertices),
                std::move(data.Indices));

            if (onComplete)
                onComplete(handle);
        });

        return handle;
    }

    MeshHandle AssetManager::GetMeshSync(const std::string &absolutePath) {
        const std::string key = Normalize(absolutePath);

        MeshHandle handle;
        {
            std::lock_guard lock(m_registryMutex);

            if (auto it = m_meshByPath.find(key); it != m_meshByPath.end()) {
                if (RenderResourceManager::Instance().IsAlive(it->second))
                    return it->second;
                m_meshByPath.erase(it);
            }


            handle = RenderResourceManager::Instance().RegisterMesh(absolutePath, MeshData{});
            m_meshByPath[key] = handle;
            m_meshPathByHandle[handle] = key;
        }


        MeshImportData data = MeshImporter::Import(absolutePath);

        if (!data.Success)
            return INVALID_MESH_HANDLE;

        RenderResourceManager::Instance().QueueMeshUpload(
            handle,
            std::move(data.Vertices),
            std::move(data.Indices));

        LogInfo("AssetLoader: sync loaded '{}' → handle {}", absolutePath, handle);
        return handle;
    }

    TextureHandle AssetManager::GetTexture(const std::string &path,
                                           bool isSRGB, std::function<void(TextureHandle)> onComplete) {
        const std::string key = Normalize(path);

        TextureHandle handle;
        {
            std::lock_guard lock(m_registryMutex);

            if (auto it = m_textureByPath.find(key); it != m_textureByPath.end()) {
                if (RenderResourceManager::Instance().IsAlive(it->second))
                    return it->second;
                m_textureByPath.erase(it);
            }

            handle = RenderResourceManager::Instance().RegisterTexture(key, TextureData{});
            m_textureByPath[key] = handle;
            m_textureHandleByPath[handle] = key;
        }

        LogInfo("AssetLoader: async texture queued '{}' → handle {}", key, handle);

        JobSystem::Instance().Submit([key, handle, isSRGB, onComplete]() {
            TextureImportData data = TextureImporter::Import(key);

            if (!data.Success) {
                LogError("Texture import failed {}", key);
                return;
            }

            RenderResourceManager::Instance().QueueTextureUpload(
                handle, std::move(data.Pixels),
                data.Width, data.Height, data.Channels, isSRGB);

            if (onComplete)
                onComplete(handle);
        });

        return handle;
    }

    MaterialHandle AssetManager::GetMaterial(const std::string &absolutePath) {
        const std::string key = Normalize(absolutePath);
        {
            std::lock_guard lock(m_registryMutex);
            if (auto it = m_materialByPath.find(key); it != m_materialByPath.end()) {
                if (RenderResourceManager::Instance().IsAlive(it->second))
                    return it->second;

                m_materialByPath.erase(it);
            }
        }

        std::ifstream f(key);
        if (!f) {
            LogError("Material not found '{}'", key);
            return INVALID_MATERIAL_HANDLE;
        }

        nlohmann::json j;
        try {
            f >> j;

            auto &rs = RTGDRenderSystem::Instance();

            MaterialHandle mat = PipelineFactory::CreateMeshPipeline(
                rs.GetDevice(), rs.GetSwapChain(), GetAbsolutePath("Shaders"));

            static const std::unordered_map<std::string, ETextureSlot> slots = {
                {"Diffuse", ETextureSlot::Diffuse},
                {"Normal", ETextureSlot::Normal},
                {"MetallicRoughness", ETextureSlot::MetallicRoughness},
                {"AO", ETextureSlot::AO}
            };

            if (j.contains("Textures")) {
                for (auto &[name, path]: j["Textures"].items()) {
                    auto it = slots.find(name);

                    if (it == slots.end())
                        continue;

                    const bool srgb = it->second == ETextureSlot::Diffuse;
                    AssignTexture(mat, it->second, GetAbsolutePath(path.get<std::string>()), srgb);
                }
            }

            {
                std::lock_guard lock(m_registryMutex);
                m_materialByPath[key] = mat;
                m_materialPathByHandle[mat] = key;
            }

            return mat;
        } catch (const nlohmann::json::exception &e) {
            LogError("Material parse error '{}': {}", key, e.what());
            return INVALID_MATERIAL_HANDLE;
        }
    }

    TextureHandle AssetManager::GetTextureSync(const std::string &absolutePath, bool isSRGB) {
        const std::string key = Normalize(absolutePath);

        TextureHandle handle;
        {
            std::lock_guard lock(m_registryMutex);

            if (auto it = m_textureByPath.find(key); it != m_textureByPath.end()) {
                if (RenderResourceManager::Instance().IsAlive(it->second))
                    return it->second;
                m_textureByPath.erase(it);
            }
            handle = RenderResourceManager::Instance().RegisterTexture(absolutePath, TextureData{});
            m_textureByPath[key] = handle;
            m_textureHandleByPath[handle] = key;
        }

        TextureImportData data = TextureImporter::Import(absolutePath);
        if (!data.Success)
            return INVALID_TEXTURE_HANDLE;

        RenderResourceManager::Instance().QueueTextureUpload(
            handle,
            std::move(data.Pixels),
            data.Width, data.Height, data.Channels,
            isSRGB);

        return handle;
    }

    void AssetManager::AssignTexture(MaterialHandle material, ETextureSlot slot, const std::string &meshAbsPath,
                                     bool srgb) {
        RenderResourceManager::Instance().QueueTextureBind(material, GetTexture(meshAbsPath, srgb), slot);
    }

    const std::string &AssetManager::GetMeshPath(MeshHandle mesh) const {
        static const std::string empty;
        std::lock_guard lock(m_registryMutex);

        auto it = m_meshPathByHandle.find(mesh);
        return it != m_meshPathByHandle.end() ? it->second : empty;
    }

    const std::string &AssetManager::GetTexturePath(TextureHandle texture) const {
        static const std::string empty;
        std::lock_guard lock(m_registryMutex);

        auto it = m_textureHandleByPath.find(texture);
        return it != m_textureHandleByPath.end() ? it->second : empty;
    }

    std::string AssetManager::Normalize(const std::string &path) {
        return std::filesystem::path(path).lexically_normal().generic_string();
    }
} // RTGDEngine
