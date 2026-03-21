//
// Created by gorev on 17.03.2026.
//

#include "AssetLoader/AssetLoader.h"

#include "AssetLoader/MeshImporter.h"
#include "AssetLoader/TextureImporter.h"
#include "JobSystem/JobSystem.h"
#include "Render/RenderResourceManager.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    MeshHandle AssetLoader::LoadMeshAsync(const std::string& absolutePath, std::function<void(MeshHandle)> onComplete)
    {
        MeshHandle handle = RenderResourceManager::Instance()
                .RegisterMesh(absolutePath, MeshData{});

        LogInfo("AssetLoader: async queued '{}' → handle {}", absolutePath, handle);

        JobSystem::Instance().Submit([absolutePath, handle, onComplete]()
        {
            MeshImportData data = MeshImporter::Import(absolutePath);

            if (!data.Success)
                return;

            RenderResourceManager::Instance().QueueMeshUpload(
                handle,
                std::move(data.Vertices),
                std::move(data.Indices));

            if (onComplete)
                onComplete(handle);
        });

        return handle;
    }

    MeshHandle AssetLoader::LoadMeshSync(const std::string& absolutePath)
    {
        MeshHandle handle = RenderResourceManager::Instance()
                .RegisterMesh(absolutePath, MeshData{});

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

    TextureHandle AssetLoader::LoadTextureAsync(const std::string& path, MaterialHandle mat, ETextureSlot slot,
                                                bool isSRGB, std::function<void(TextureHandle)> onComplete)
    {
        TextureHandle handle = RenderResourceManager::Instance()
                .RegisterTexture(path, TextureData{});

        LogInfo("AssetLoader: async texture queued '{}' → handle {}", path, handle);

        JobSystem::Instance().Submit([path, handle, mat, slot, isSRGB, onComplete]()
        {
            TextureImportData data = TextureImporter::Import(path);
            if (!data.Success)
                return;

            RenderResourceManager::Instance().QueueTextureUpload(
                handle, std::move(data.Pixels),
                data.Width, data.Height, data.Channels, isSRGB);

            RenderResourceManager::Instance().QueueTextureBind(mat, handle, slot);

            if (onComplete)
                onComplete(handle);
        });

        return handle;
    }

    TextureHandle AssetLoader::LoadTextureSync(const std::string& absolutePath, bool isSRGB)
    {
        TextureHandle handle = RenderResourceManager::Instance()
                .RegisterTexture(absolutePath, TextureData{});

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
} // RTGDEngine
