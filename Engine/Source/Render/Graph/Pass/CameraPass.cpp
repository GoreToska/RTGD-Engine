//
// Created by ivan on 7/14/26.
//

#include "Render/Graph/Pass/CameraPass.h"
#include <flecs.h>

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Render/Graph/RenderContext.h"
#include "Systems/CameraSystem.h"

namespace RTGDEngine {
    void CameraPass::Execute(RenderContext &context) {
        auto cameraEntity = CameraSystem::GetActiveCamera(context.World);

        if (cameraEntity.is_valid()) {
            auto *cam = cameraEntity.try_get<CameraComponent>();
            auto *transform = cameraEntity.try_get<TransformComponent>();

            if (!cam || !transform)
                return;

            CameraConstantBuffer cb;
            cb.View = cam->ViewMatrix;
            cb.Projection = cam->ProjectionMatrix;
            cb.CameraPosition =
            {
                transform->Position.x,
                transform->Position.y,
                transform->Position.z, 1.0f
            };

            context.Frame.UpdateCamera(cb);
        }
    }

    void CameraPass::Initialize(Diligent::IRenderDevice &device, Diligent::ISwapChain &swapChain, GBuffer &gbuffer) {
    }

    const char *CameraPass::Name() const {
        return "Camera";
    }
} // RTGDEngine
