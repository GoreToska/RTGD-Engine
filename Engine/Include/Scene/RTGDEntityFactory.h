//
// Created by gorev on 12.03.2026.
//

#pragma once
#include <string>
#include <flecs.h>

#include "RenderDevice.h"
#include "Engine/EngineExport.h"

namespace RTGDEngine
{
    class ENGINE_API RTGDEntityFactory
    {
    public:
        static flecs::entity CreateTriangle(flecs::world& world,
                                            Diligent::IRenderDevice& device,
                                            Diligent::ISwapChain& swapChain,
                                            const std::string& shadersPath);
    };
}
