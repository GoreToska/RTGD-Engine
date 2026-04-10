//
// Created by gorev on 05.04.2026.
//

#pragma once

struct GameObject
{
    static void RegisterMeta(const flecs::world& world)
    {
        flecs::component<GameObject>(world, "GameObject");
    }
};
