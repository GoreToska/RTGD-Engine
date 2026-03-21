//
// Created by gorev on 13.03.2026.
//

#pragma once
#include "Engine/EngineExport.h"
#include <flecs.h>

namespace RTGDEngine
{
    struct CameraComponent;

    class ENGINE_API CameraSystem
    {
    public:
        static void Update(const flecs::world& world, float deltaTime);

        static flecs::entity GetActiveCamera(const flecs::world& world);
    };
} // RTGDEngine
