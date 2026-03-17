//
// Created by gorev on 17.03.2026.
//

#pragma once
#include <string>
#include <vector>

#include "Engine/EngineExport.h"
#include "Render/Vertex.h"

namespace RTGDEngine
{
    struct MeshImportData
    {
        std::vector<VertexPNUV> Vertices;
        std::vector<uint32_t> Indices;
        uint32_t VertexCount;
        bool Success = false;
        std::string ErrorMessage;
    };

    class ENGINE_API MeshImporter
    {
    public:
        static MeshImportData ImportMesh(const std::string& absolutePath);
    };
} // RTGDEngine
