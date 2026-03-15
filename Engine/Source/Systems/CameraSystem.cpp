//
// Created by gorev on 13.03.2026.
//

#include "Systems/CameraSystem.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"

namespace RTGDEngine
{
    void CameraSystem::Update(const flecs::world& world, float deltaTime)
    {
        world.each([&](CameraComponent& cam,
                       const TransformComponent& transform)
        {
            const auto right = transform.GetRight();
            const auto up = transform.GetUp();
            const auto forward = transform.GetForward();
            const auto pos = transform.Position;

            auto view = Matrix4::ViewFromBasis(right, up, forward);
            view.m30 = -Diligent::dot(right, pos);
            view.m31 = -Diligent::dot(up, pos);
            view.m32 = -Diligent::dot(forward, pos);
            cam.ViewMatrix = view;

            cam.ProjectionMatrix = Diligent::float4x4::Projection(
                cam.FOVDegrees * Diligent::PI_F / 180.0f,
                cam.AspectRatio,
                cam.NearPlane,
                cam.FarPlane,
                false);
        });
    }
} // RTGDEngine
