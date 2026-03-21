//
// Created by gorev on 17.03.2026.
//

#include "AssetLoader/MeshImporter.h"
#include "Tools/Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Render/Vertex.h"
#include "Tools/Alias.h"

namespace RTGDEngine
{
    MeshImportData MeshImporter::Import(const std::string& absolutePath)
    {
        MeshImportData result;
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(absolutePath,
                                                 aiProcess_Triangulate |
                                                 aiProcess_GenSmoothNormals |
                                                 aiProcess_CalcTangentSpace |
                                                 aiProcess_ConvertToLeftHanded |
                                                 /*aiProcess_MakeLeftHanded |
                                                 aiProcess_FlipWindingOrder |*/
                                                 aiProcess_JoinIdenticalVertices);

        if (!scene || !scene->HasMeshes())
        {
            result.ErrorMessage = importer.GetErrorString();
            LogError("MeshImporter: failed to load '{}': {}", absolutePath, result.ErrorMessage);
            return result;
        }

        uint32_t vertexOffset = 0;
        for (uint32_t m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];

            for (uint32_t i = 0; i < mesh->mNumVertices; i++)
            {
                VertexPNTUV vertex;

                vertex.Position = {
                    mesh->mVertices[i].x,
                    mesh->mVertices[i].y,
                    mesh->mVertices[i].z
                };

                vertex.Normal = mesh->HasNormals()
                                    ? Float3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z}
                                    : Float3{0.0f, 1.0f, 0.0f};

                if (mesh->HasTangentsAndBitangents())
                {
                    Float3 T = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
                    Float3 B = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
                    Float3 N = vertex.Normal;

                    Float3 crossNT = {
                        N.y * T.z - N.z * T.y,
                        N.z * T.x - N.x * T.z,
                        N.x * T.y - N.y * T.x
                    };

                    float dotResult = crossNT.x * B.x + crossNT.y * B.y + crossNT.z * B.z;
                    float sign = (dotResult < 0.0f) ? -1.0f : 1.0f;

                    vertex.Tangent = {T.x, T.y, T.z, sign};

                }
                else
                {
                    vertex.Tangent = {1.0f, 0.0f, 0.0f, 1.0f};
                }

                vertex.UV = mesh->HasTextureCoords(0)
                                ? Float2{
                                    mesh->mTextureCoords[0][i].x,
                                    mesh->mTextureCoords[0][i].y
                                }
                                : Float2{0.0f, 0.0f};

                result.Vertices.push_back(vertex);
            }

            for (uint32_t f = 0; f < mesh->mNumFaces; f++)
            {
                const aiFace& face = mesh->mFaces[f];
                for (uint32_t i = 0; i < face.mNumIndices; i++)
                    result.Indices.push_back(vertexOffset + face.mIndices[i]);
            }

            vertexOffset += mesh->mNumVertices;

            if (m == 0)
            {
                LogInfo("=== UV debug for: {}", absolutePath);
                LogInfo("ConvertToLeftHanded: {}", "YES");
                LogInfo("JoinIdenticalVertices: {}", "YES");
                for (uint32_t i = 0; i < std::min(mesh->mNumVertices, 4u); i++)
                {
                    auto& v = result.Vertices[result.Vertices.size() - mesh->mNumVertices + i];
                    LogInfo("  v[{}] pos=({:.2f},{:.2f},{:.2f}) uv=({:.4f},{:.4f})",
                            i, v.Position.x, v.Position.y, v.Position.z, v.UV.x, v.UV.y);
                }
            }
        }

        result.Success = true;

        LogInfo("MeshImporter: '{}' — {} vertices, {} indices",
                absolutePath, result.Vertices.size(), result.Indices.size());

        return result;
    }
} // RTGDEngine
