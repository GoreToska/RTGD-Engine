//
// Created by gorev on 13.03.2026.
//

#pragma once
#include "Engine/EngineExport.h"
#include <flecs.h>

namespace RTGDEngine
{
    class ENGINE_API MovementSystem
    {
    public:
        static void Update(const flecs::world& world, float deltaTime);
    };
}
