//
// Created by gorev on 17.03.2026.
//

#pragma once

#include "Engine/EngineExport.h"
#include "Render/ConstBuffers.h"
#include <flecs.h>

namespace RTGDEngine
{
    class ENGINE_API LightSystem
    {
    public:
        static void Update(const flecs::world& world);

    private:
        static void UpdateAmbient(const flecs::world& world);

        static void UpdateDirectionalLights(const flecs::world& world);

        static void UpdatePointLights(const flecs::world& world);

        static void UpdateSpotLights(const flecs::world& world);

        inline static LightConstantBuffer m_lightCB;
    };
} // RTGDEngine
