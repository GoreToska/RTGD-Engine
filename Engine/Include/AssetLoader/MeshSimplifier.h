//
// Created by ivan on 9/2/26.
//

#pragma once
#include "MeshImporter.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    class MeshSimplifier
    {
        DECLARE_SINGLETON(MeshSimplifier);

    public:
        MeshImportData Simplify(const MeshImportData& src, float ratio, float targetError = 0.01f);

        std::vector<Float3> SimplifyPoints(const std::vector<VertexPNTUV>& vertices, uint32_t targetVertexCount);
    };

    DECLARE_GLOBAL_SINGLETON(MeshSimplifier, GMeshSimplifier);
} // RTGDEngine
