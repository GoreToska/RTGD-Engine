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
    };

    struct PendingGPUUpload
    {
        MeshHandle Handle;
        std::vector<VertexPNUV> Vertices;
        std::vector<uint32_t> Indices;
    };

    class RenderResourceManager
    {
    public:
        static RenderResourceManager& Instance();

        MeshHandle RegisterMesh(const std::string& name, MeshData data);

        MaterialHandle RegisterMaterial(const std::string& name, MaterialData data);

        [[nodiscard]] const MeshData& GetMesh(MeshHandle handle) const;

        [[nodiscard]] const MaterialData& GetMaterial(MaterialHandle handle) const;

        [[nodiscard]] MeshHandle GetMeshByName(const std::string& name) const;

        [[nodiscard]] MaterialHandle GetMaterialByName(const std::string& name) const;

        void QueueGPUUpload(MeshHandle handle, std::vector<VertexPNUV> vertices, std::vector<uint32_t> indices);

        void FlushGPUUploads(Diligent::IRenderDevice& device);

        void UpdateMesh(MeshHandle handle, MeshData data);

    private:
        RenderResourceManager() = default;

        std::vector<MeshData> m_meshes;
        std::vector<MaterialData> m_materials;

        std::unordered_map<std::string, MeshHandle> m_meshNames;
        std::unordered_map<std::string, MaterialHandle> m_materialNames;

        std::mutex m_uploadMutex;
        std::vector<PendingGPUUpload> m_pendingUploads;
    };
} // RTGDEngine
