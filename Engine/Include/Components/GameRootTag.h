//
// Created by ivan on 8/19/26.
//

#pragma once

#include <flecs.h>


namespace RTGDEngine {
    struct GameRootTag {
        static void RegisterMeta(const flecs::world &world) {
            flecs::component<GameRootTag>(world, "GameRootTag");
        }
    };
}
