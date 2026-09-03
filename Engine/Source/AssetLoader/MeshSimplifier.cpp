//
// Created by ivan on 9/2/26.
//

#include "AssetLoader/MeshSimplifier.h"

#include "meshoptimizer.h"

namespace RTGDEngine
{
    MeshImportData MeshSimplifier::Simplify(const MeshImportData& src, float ratio, float targetError)
    {
        size_t targetIndexCount = size_t(src.Indices.size() * ratio);
        std::vector<unsigned int> simplified(src.Indices.size());
        float resultError = 0.0f;

        size_t newCount = meshopt_simplify(simplified.data(), src.Indices.data(), src.Indices.size(),
                                           &src.Vertices[0].Position.x, src.Vertices.size(), sizeof(VertexPNTUV),
                                           targetIndexCount, targetError, 0, &resultError);

        simplified.resize(newCount);

        std::vector<unsigned int> remap(src.Vertices.size());
        size_t uniqueCount = meshopt_generateVertexRemap(remap.data(), simplified.data(), simplified.size(),
                                                         src.Vertices.data(), src.Vertices.size(), sizeof(VertexPNTUV));

        MeshImportData result;
        result.Indices.resize(simplified.size());
        for (size_t i = 0; i < simplified.size(); ++i)
            result.Indices[i] = remap[simplified[i]];

        result.Vertices.resize(uniqueCount);
        result.VertexCount = static_cast<uint32_t>(uniqueCount);
        meshopt_remapVertexBuffer(result.Vertices.data(), src.Vertices.data(), src.Vertices.size(), sizeof(VertexPNTUV),
                                  remap.data());
        result.Success = true;
        return result;
    }

    std::vector<Float3> MeshSimplifier::SimplifyPoints(const std::vector<VertexPNTUV>& vertices,
                                                       uint32_t targetVertexCount)
    {
        std::vector<unsigned int> indices(targetVertexCount);
        size_t count = meshopt_simplifyPoints(
            indices.data(),
            &vertices[0].Position.x, vertices.size(), sizeof(VertexPNTUV),
            nullptr, 0, 0.0f, targetVertexCount);

        std::vector<Float3> out(count);
        for (size_t i = 0; i < count; ++i)
            out[i] = vertices[indices[i]].Position;

        return out;
    }
} // RTGDEngine
