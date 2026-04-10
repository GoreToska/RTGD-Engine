//
// Created by gorev on 27.03.2026.
//

#pragma once
#include <flecs.h>

namespace RTGDEngine
{
    struct UUIDComponent
    {
        int ID = 0;

        static void RegisterMeta(const flecs::world& world)
        {
            flecs::component<UUIDComponent>(world, "UUIDComponent")
                    .member<int>("ID");
        }
    };
}
