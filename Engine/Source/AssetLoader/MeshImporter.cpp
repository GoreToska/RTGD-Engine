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
                                                 aiProcess_FlipUVs |
                                                 aiProcess_JoinIdenticalVertices);

        if (!scene || !scene->HasMeshes())
        {
            result.ErrorMessage = importer.GetErrorString();
            LogError("MeshImporter: failed to load '{}': {}", absolutePath, result.ErrorMessage);
            return result;
        }

        // Unite all submeshes to one vertex and index buffers
        uint32_t vertexOffset = 0;
        for (uint32_t m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];

            for (uint32_t i = 0; i < mesh->mNumVertices; i++)
            {
                VertexPNUV vertex;

                vertex.Position = {
                    mesh->mVertices[i].x,
                    mesh->mVertices[i].y,
                    mesh->mVertices[i].z
                };

                vertex.Normal = mesh->HasNormals()
                                    ? Float3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z}
                                    : Float3{0.0f, 1.0f, 0.0f};

                vertex.UV = mesh->HasTextureCoords(0)
                                ? Float2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}
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
        }

        result.Success = true;

        LogInfo("MeshImporter: '{}' — {} vertices, {} indices",
                absolutePath, result.Vertices.size(), result.Indices.size());

        return result;
    }
} // RTGDEngine
