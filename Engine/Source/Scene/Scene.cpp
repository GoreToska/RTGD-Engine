//
// Created by gorev on 23.03.2026.
//

#include "Scene/Scene.h"

#include <fstream>

#include "Components/UUIDComponent.h"
#include "Tools/Alias.h"
#include "Tools/Logger.h"

#include <nlohmann/json.hpp>

#include "Components/GameRootTag.h"
#include "Scene/SceneManager.h"

namespace RTGDEngine
{
    Scene::Scene(flecs::world& world, const std::string& name)
        : m_name(name), m_world(&world)
    {
        m_root = m_world->entity(name.c_str());
        LogInfo("Scene created: '{}'", m_name);
    }

    flecs::entity Scene::GetOrCreateGameRoot()
    {
        if (m_gameRoot.is_alive())
            return m_gameRoot;

        m_gameRoot = GScene.CreateEntity("GameRoot", m_root).add<GameRootTag>();
        return m_gameRoot;
    }

    flecs::entity Scene::Find(const std::string& name)
    {
        return m_root.lookup(name.c_str());
    }

    flecs::entity Scene::GetRoot() const
    {
        return m_root;
    }

    const std::string& Scene::GetName() const
    {
        return m_name;
    }

    void Scene::SetName(const std::string& name)
    {
        m_name = name;
    }

    std::string Scene::Serialize() const
    {
        nlohmann::json json = nlohmann::json::array();

        std::function<void(Entity, const std::string&)> collect = [&](Entity parent, const std::string& parentName)
        {
            parent.children([&](Entity child)
            {
                nlohmann::json full = nlohmann::json::parse(child.to_json().c_str());
                nlohmann::json obj;
                obj["Name"] = child.name().c_str();
                if (!parentName.empty())
                    obj["Parent"] = parentName;

                obj["Data"]["components"] = full["components"];
                json.push_back(std::move(obj));
                collect(child, child.name().c_str());
            });
        };

        collect(m_root, "");

        return json.dump(2);
    }

    void Scene::Deserialize(const std::string& json)
    {
        Clear();
        ApplyEntities(ParseScene(json));
    }

    void Scene::Clear()
    {
        m_world->delete_with(m_world->pair(flecs::ChildOf, m_root));
    }

    void Scene::SaveToFile(const std::string& absolutePath) const
    {
        auto json = Serialize();
        if (json == "[]" || json == "[\n]")
        {
            LogWarn("SaveToFile: scene '{}' empty, skip", m_name);
            return;
        }
        std::ofstream f(absolutePath.c_str());
        f << json;
        LogInfo("Saved to {}", absolutePath.c_str());
    }

    bool Scene::LoadFromFile(const std::string& absolutePath)
    {
        std::ifstream f(absolutePath.c_str());
        if (!f)
        {
            LogError("Scene not found '{}'", absolutePath.c_str());
            return false;
        }

        m_lastLoadedPath = absolutePath;
        std::stringstream ss;
        ss << f.rdbuf();
        Deserialize(ss.str());
        return true;
    }

    bool Scene::Reload()
    {
        return m_lastLoadedPath.empty() ? false : LoadFromFile(m_lastLoadedPath);
    }

    std::vector<Scene::EntityData> Scene::ParseScene(const std::string& json)
    {
        std::vector<EntityData> entities;
        auto arr = nlohmann::json::parse(json);
        for (auto& obj: arr)
        {
            nlohmann::json data;
            data["components"] = obj["Data"]["components"];
            std::string parentName = obj.value("Parent", "");
            entities.push_back({obj["Name"].get<std::string>(), parentName, data.dump()});
        }

        return entities;
    }

    void Scene::ApplyEntities(const std::vector<EntityData>& entities)
    {
        std::vector<Entity> created = {};
        created.reserve(entities.size());

        for (auto& entity: entities)
        {
            auto e = m_world->entity();
            e.child_of(m_root);
            e.set_name(entity.name.c_str());
            e.add<SceneEntity>();
            created.push_back(e);
        }

        for (size_t i = 0; i < created.size(); ++i)
        {
            Entity parent = entities[i].parentName.empty() ? m_root : m_root.lookup(entities[i].parentName.c_str());

            if (!parent.is_valid())
            {
                LogError("Parent '{}' not found for {}", entities[i].parentName.c_str(), entities[i].name);
                parent = m_root;
            }

            created[i].child_of(parent);
        }

        for (size_t i = 0; i < entities.size(); ++i)
        {
            created[i].from_json(entities[i].data.c_str());
        }
    }
} // RTGDEngine
