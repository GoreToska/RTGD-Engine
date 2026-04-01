//
// Created by gorev on 23.03.2026.
//

#pragma once

#include "Engine/EngineExport.h"
#include <flecs.h>
#include <string>

namespace RTGDEngine
{
    class ENGINE_API Scene
    {
    public:
        explicit Scene(const std::string& name);

        ~Scene() = default;

        flecs::entity CreateEntity(const std::string& name);

        void DestroyEntity(flecs::entity entity);

        flecs::entity Find(const std::string& name);

        flecs::world& GetWorld();

        [[nodiscard]] const flecs::world& GetWorld() const;

        [[nodiscard]] const std::string& GetName() const;

        void SetName(const std::string& name);

        template<typename Func>
        void Each(Func&& func) { m_world.each(std::forward<Func>(func)); }

    private:
        flecs::world m_world;
        std::string m_name;
    };
} // RTGDEngine
