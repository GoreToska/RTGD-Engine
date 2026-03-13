//
// Created by gorev on 12.03.2026.
//

#include "Render/RenderResourceManager.h"

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
} // RTGDEngine
