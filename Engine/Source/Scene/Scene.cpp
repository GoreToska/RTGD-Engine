//
// Created by gorev on 23.03.2026.
//

#include "Scene/Scene.h"

#include "Tools/Logger.h"

namespace RTGDEngine
{
    Scene::Scene(const std::string& name)
        : m_name(name)
    {
        LogInfo("Scene created: '{}'", m_name);
    }

    flecs::entity Scene::CreateEntity(const std::string& name)
    {
        auto entity = m_world.entity(name.c_str());
        
        LogInfo("Scene '{}': entity created '{}'", m_name, name);
        return entity;
    }

    void Scene::DestroyEntity(const flecs::entity entity)
    {
        if (!entity.is_valid())
            return;
        LogInfo("Scene '{}': entity destroyed '{}'", m_name, entity.name().c_str());
        entity.destruct();
    }

    flecs::entity Scene::Find(const std::string& name)
    {
        return m_world.lookup(name.c_str());
    }

    flecs::world& Scene::GetWorld()
    {
        return m_world;
    }

    const flecs::world& Scene::GetWorld() const
    {
        return m_world;
    }

    const std::string& Scene::GetName() const
    {
        return m_name;
    }

    void Scene::SetName(const std::string& name)
    {
        m_name = name;
    }
} // RTGDEngine
