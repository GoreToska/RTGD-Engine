//
// Created by gorev on 23.03.2026.
//

#include "Scene/Scene.h"

#include <fstream>

#include "Components/UUIDComponent.h"
#include "Tools/Logger.h"

#include <nlohmann/json.hpp>

namespace RTGDEngine {
    Scene::Scene(flecs::world &world, const std::string &name)
        : m_name(name), m_world(&world) {
        m_root = m_world->entity(name.c_str());
        LogInfo("Scene created: '{}'", m_name);
    }

    flecs::entity Scene::CreateEntity(const std::string &name) {
        auto entity = m_world->entity(name.c_str()).child_of(m_root)
                .add<UUIDComponent>()
                .add<SceneEntity>();

        LogInfo("Scene '{}': entity created '{}'", m_name, name);
        return entity;
    }

    void Scene::DestroyEntity(const flecs::entity entity) {
        if (!entity.is_valid())
            return;
        LogInfo("Scene '{}': entity destroyed '{}'", m_name, entity.name().c_str());
        entity.destruct();
    }

    flecs::entity Scene::Find(const std::string &name) {
        return m_root.lookup(name.c_str());
    }

    flecs::entity Scene::GetRoot() const {
        return m_root;
    }

    const std::string &Scene::GetName() const {
        return m_name;
    }

    void Scene::SetName(const std::string &name) {
        m_name = name;
    }

    std::string Scene::Serialize() const {
        nlohmann::json json = nlohmann::json::array();

        m_world->query_builder<>().with(flecs::ChildOf, m_root).build()
                .each([&](const flecs::entity entity) {
                    nlohmann::json full = nlohmann::json::parse(entity.to_json().c_str());
                    nlohmann::json obj;
                    obj["Name"] = entity.name().c_str();
                    obj["Data"]["components"] = full["components"];
                    json.push_back(std::move(obj));
                });

        return json.dump(2);
    }

    void Scene::Deserialize(const std::string &json) {
        Clear();
        ApplyEntities(ParseScene(json));
    }

    void Scene::Clear() {
        m_world->delete_with(m_world->pair(flecs::ChildOf, m_root));
    }

    void Scene::SaveToFile(const std::string &absolutePath) const {
        auto json = Serialize();
        if (json == "[]" || json == "[\n]") {
            LogWarn("SaveToFile: scene '{}' empty, skip", m_name);
            return;
        }
        std::ofstream f(absolutePath.c_str());
        f << json;
        LogInfo("Saved to {}", absolutePath.c_str());
    }

    bool Scene::LoadFromFile(const std::string &absolutePath) {
        std::ifstream f(absolutePath.c_str());
        if (!f) {
            LogError("Scene not found '{}'", absolutePath.c_str());
            return false;
        }

        std::stringstream ss;
        ss << f.rdbuf();
        Deserialize(ss.str());
        return true;
    }

    std::vector<Scene::EntityData> Scene::ParseScene(const std::string &json) {
        std::vector<EntityData> entities;
        auto arr = nlohmann::json::parse(json);
        for (auto &obj: arr) {
            nlohmann::json data;
            data["components"] = obj["Data"]["components"];
            entities.push_back({obj["Name"].get<std::string>(), data.dump()});
        }

        return entities;
    }

    void Scene::ApplyEntities(const std::vector<EntityData> &entities) {
        for (auto &entity: entities) {
            auto e = m_world->entity();
            e.from_json(entity.data.c_str());
            e.child_of(m_root);
            e.set_name(entity.name.c_str());
            e.add<SceneEntity>();
        }
    }
} // RTGDEngine
