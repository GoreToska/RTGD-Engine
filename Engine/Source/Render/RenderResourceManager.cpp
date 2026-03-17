//
// Created by gorev on 12.03.2026.
//

#include "Render/RenderResourceManager.h"

#include "RenderDevice.h"
#include "Render/Vertex.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    RenderResourceManager& RenderResourceManager::Instance()
    {
        static RenderResourceManager instance;
        return instance;
    }

    MeshHandle RenderResourceManager::RegisterMesh(const std::string& name, MeshData data)
    {
        MeshHandle handle = static_cast<MeshHandle>(m_meshes.size());
        m_meshes.push_back(std::move(data));
        m_meshNames[name] = handle;
        return handle;
    }

    MaterialHandle RenderResourceManager::RegisterMaterial(const std::string& name, MaterialData data)
    {
        MaterialHandle handle = static_cast<MaterialHandle>(m_materials.size());
        m_materials.push_back(std::move(data));
        m_materialNames[name] = handle;
        return handle;
    }

    const MeshData& RenderResourceManager::GetMesh(MeshHandle handle) const
    {
        return m_meshes[handle];
    }

    const MaterialData& RenderResourceManager::GetMaterial(MaterialHandle handle) const
    {
        return m_materials[handle];
    }

    MeshHandle RenderResourceManager::GetMeshByName(const std::string& name) const
    {
        auto it = m_meshNames.find(name);
        return it != m_meshNames.end() ? it->second : INVALID_HANDLE;
    }

    MaterialHandle RenderResourceManager::GetMaterialByName(const std::string& name) const
    {
        auto it = m_materialNames.find(name);
        return it != m_materialNames.end() ? it->second : INVALID_HANDLE;
    }

    void RenderResourceManager::QueueGPUUpload(MeshHandle handle, std::vector<VertexPNUV> vertices,
                                               std::vector<uint32_t> indices)
    {
        std::lock_guard lock(m_uploadMutex);
        m_pendingUploads.push_back({
            handle,
            std::move(vertices),
            std::move(indices)
        });

        LogInfo("RenderResourceManager: queued GPU upload → handle {}", handle);
    }

    void RenderResourceManager::FlushGPUUploads(Diligent::IRenderDevice& device)
    {
        using namespace Diligent;

        std::vector<PendingGPUUpload> uploads; {
            std::lock_guard lock(m_uploadMutex);
            uploads = std::move(m_pendingUploads);
            m_pendingUploads.clear();
        }

        if (uploads.empty())
            return;

        for (auto& upload: uploads)
        {
            if (upload.Vertices.empty())
            {
                LogWarn("RenderResourceManager: empty vertices for handle {}", upload.Handle);
                continue;
            }

            MeshData data;
            data.VertexCount = static_cast<uint32_t>(upload.Vertices.size());

            // Vertex Buffer
            BufferDesc vbDesc;
            vbDesc.Name = "Mesh VB";
            vbDesc.Size = upload.Vertices.size() * sizeof(VertexPNUV);
            vbDesc.Usage = USAGE_IMMUTABLE;
            vbDesc.BindFlags = BIND_VERTEX_BUFFER;

            BufferData vbData;
            vbData.pData = upload.Vertices.data();
            vbData.DataSize = vbDesc.Size;
            device.CreateBuffer(vbDesc, &vbData, &data.VertexBuffer);

            // Index Buffer
            if (!upload.Indices.empty())
            {
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

            LogInfo("RenderResourceManager: GPU upload done → handle {}, {} vertices, {} indices",
                    upload.Handle,
                    upload.Vertices.size(),
                    upload.Indices.size());
        }
    }

    void RenderResourceManager::UpdateMesh(MeshHandle handle, MeshData data)
    {
        if (handle == INVALID_HANDLE || handle >= m_meshes.size())
        {
            LogError("RenderResourceManager::UpdateMesh — invalid handle {}", handle);
            return;
        }

        m_meshes[handle] = std::move(data);
    }
} // RTGDEngine
