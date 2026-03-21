//
// Created by gorev on 13.03.2026.
//

#include "Scene/RTGDEntityFactory.h"

#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Render/PipelineFactory.h"
#include "Render/RenderResourceManager.h"
#include "Render/RenderSystem.h"
#include "Render/SimpleMeshFactory.h"
#include "Tools/Logger.h"

namespace RTGDEngine
{
    flecs::entity RTGDEntityFactory::CreateTriangle(flecs::world& world, Diligent::IRenderDevice& device,
                                                    Diligent::ISwapChain& swapChain, const std::string& shadersPath)
    {
        MeshHandle mesh = SimpleMeshFactory::CreateTriangle(device);

        MaterialHandle mat = PipelineFactory::CreateGBufferPipeline(device, RTGDRenderSystem::Instance().GetGBuffer(),
                                                                    shadersPath);

        TextureHandle colorTex = RenderResourceManager::Instance()
                .RegisterTexture("triangle_color", 255, 128, 0, 255);

        RenderResourceManager::Instance().BindTextureToMaterial(mat, colorTex);

        flecs::entity entity = world.entity("Triangle")
                .set(TransformComponent{})
                .set(MeshComponent{mesh, mat})
                .set(RenderComponent{});

        LogInfo("Entity '{}' created — mesh: {}, material: {}",
                entity.name().c_str(), mesh, mat);

        return entity;
    }
}
