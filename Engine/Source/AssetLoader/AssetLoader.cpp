//
// Created by gorev on 17.03.2026.
//

#include "AssetLoader/AssetLoader.h"

#include "AssetLoader/MeshImporter.h"
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
            MeshImportData data = MeshImporter::ImportMesh(absolutePath);

            if (!data.Success)
                return;

            RenderResourceManager::Instance().QueueGPUUpload(
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

        MeshImportData data = MeshImporter::ImportMesh(absolutePath);

        if (!data.Success)
            return INVALID_HANDLE;

        RenderResourceManager::Instance().QueueGPUUpload(
            handle,
            std::move(data.Vertices),
            std::move(data.Indices));

        LogInfo("AssetLoader: sync loaded '{}' → handle {}", absolutePath, handle);
        return handle;
    }
} // RTGDEngine
