//
// Created by ivan on 9/30/26.
//

#pragma once

#include "Engine/EngineExport.h"
#include "Tools/Alias.h"
#include <flecs.h>

namespace RTGDEngine
{
    class ENGINE_API TransformSystem
    {
    public:
        static void Update(World& world);

    private:
        inline static flecs::system m_propagate {};
    };
} // RTGDEngine
