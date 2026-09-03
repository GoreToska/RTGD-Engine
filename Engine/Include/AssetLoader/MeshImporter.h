//
// Created by gorev on 17.03.2026.
//

#pragma once
#include <string>
#include <vector>
#include <limits>

#include "Engine/EngineExport.h"
#include "Render/Vertex.h"
#include "Tools/Bounds.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    struct MeshImportData
    {
        std::vector<VertexPNTUV> Vertices = {};
        std::vector<uint32_t> Indices = {};
        uint32_t VertexCount = 0;
        bool Success = false;
        std::string ErrorMessage;
        AABB LocalBounds = {
            {
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()
            },
            {
                std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()
            }
        };
    };

    class ENGINE_API MeshImporter
    {
        DECLARE_SINGLETON(MeshImporter);

    public:
        MeshImportData Import(const std::string& absolutePath);
    };

    DECLARE_GLOBAL_SINGLETON(MeshImporter, GMeshImporter);
} // RTGDEngine
