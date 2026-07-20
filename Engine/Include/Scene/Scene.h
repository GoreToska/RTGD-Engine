//
// Created by gorev on 23.03.2026.
//

#pragma once

#include "Engine/EngineExport.h"
#include <flecs.h>
#include <string>
#include <vector>

#include "Event/EventBus.h"

namespace RTGDEngine {
    class ENGINE_API Scene {
    public:
        struct EntityData {
            std::string name;
            std::string data;
        };

        explicit Scene(flecs::world &world, const std::string &name);

        ~Scene() = default;

        flecs::entity Find(const std::string &name);

        [[nodiscard]] flecs::entity GetRoot() const;

        [[nodiscard]] const std::string &GetName() const;

        void SetName(const std::string &name);

        std::string Serialize() const;

        void Deserialize(const std::string &json);

        void Clear();

        void SaveToFile(const std::string &absolutePath) const;

        bool LoadFromFile(const std::string &absolutePath);

        static std::vector<EntityData> ParseScene(const std::string &json); // thread safe for async scene loading
        void ApplyEntities(const std::vector<EntityData> &entities);

        template<typename Func>
        void Each(Func &&func) {
            m_world->query_builder<>().with(flecs::ChildOf, m_root).build().each(std::forward<Func>(func));
        }

    private:
        std::string m_name;
        flecs::world *m_world;
        flecs::entity m_root;
    };
} // RTGDEngine
