//
// Created by gorev on 05.04.2026.
//

#pragma once

#include <flecs.h>

namespace RTGDEngine
{
    void RegisterMetaTypes(const flecs::world& world);

    inline bool MetaAlreadyRegistered(const flecs::world& world, flecs::entity comp)
    {
        ecs_entity_t ecsStruct = ecs_lookup(world.get_world(), "flecs.meta.Struct");

        if (!ecsStruct)
            return false;

        return ecs_has_id(world.get_world(), comp.id(), ecsStruct);
    }
}
