//
// Created by gorev on 12.03.2026.
//

#include "Render/SimpleMeshFactory.h"

#include "Render/RenderResourceManager.h"
#include "Render/Vertex.h"

namespace RTGDEngine
{
    static constexpr VertexPC triangleVerts[] = {
        {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    };

    MeshHandle SimpleMeshFactory::CreateTriangle(Diligent::IRenderDevice& device)
    {
        using namespace Diligent;
        MeshData data;
        data.VertexCount = 3;

        BufferDesc vbDesc;
        vbDesc.Name = "Triangle VB";
        vbDesc.Size = sizeof(triangleVerts);
        vbDesc.Usage = USAGE_IMMUTABLE;
        vbDesc.BindFlags = BIND_VERTEX_BUFFER;

        BufferData vbData;
        vbData.pData = triangleVerts;
        vbData.DataSize = sizeof(triangleVerts);

        device.CreateBuffer(vbDesc, &vbData, &data.VertexBuffer);

        return RenderResourceManager::Instance().RegisterMesh("Triangle", std::move(data));
    }
}
