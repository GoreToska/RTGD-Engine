//
// Created by gorev on 23.03.2026.
//

#include "Scene/Scene.h"

#include "Components/SceneEntity.h"
#include "Components/UUIDComponent.h"
#include "Tools/Logger.h"
#include "Tools/MetaTypes.h"

#include <nlohmann/json.hpp>

namespace RTGDEngine {
    Scene::Scene(const std::string &name)
        : m_name(name) {
        RegisterMetaTypes(m_world);

        LogInfo("Scene created: '{}'", m_name);
    }

    flecs::entity Scene::CreateEntity(const std::string &name) {
        auto entity = m_world.entity(name.c_str());
        entity.add<UUIDComponent>();
        entity.add<SceneEntity>();

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
        return m_world.lookup(name.c_str());
    }

    flecs::world &Scene::GetWorld() {
        return m_world;
    }

    const flecs::world &Scene::GetWorld() const {
        return m_world;
    }

    const std::string &Scene::GetName() const {
        return m_name;
    }

    void Scene::SetName(const std::string &name) {
        m_name = name;
    }

    std::string Scene::Serialize() const {
        nlohmann::json json = nlohmann::json::array();

        m_world.query_builder<>().with<SceneEntity>().build()
                .each([&](const flecs::entity entity) {
                    nlohmann::json obj;
                    obj["Name"] = entity.name().c_str();
                    obj["Data"] = nlohmann::json::parse(entity.to_json().c_str());
                    json.push_back(std::move(obj));
                });

        return json.dump(2);
    }

    void Scene::Deserialize(const std::string &json) {
        Clear();
        auto arr = nlohmann::json::parse(json);

        for (auto &obj: arr) {
            auto e = CreateEntity(obj["Name"].get<std::string>());
            e.from_json(obj["Data"].dump().c_str());
        }
    }

    void Scene::Clear() {
        m_world.delete_with(m_world.component<SceneEntity>());
    }
} // RTGDEngine
