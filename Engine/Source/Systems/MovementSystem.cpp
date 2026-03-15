//
// Created by gorev on 13.03.2026.
//

#include "Systems/MovementSystem.h"

#include "Components/TransformComponent.h"
#include "Components/VelocityComponent.h"

void RTGDEngine::MovementSystem::Update(const flecs::world& world, float deltaTime)
{
    world.each([&](const VelocityComponent& velocity,
                   TransformComponent& transform)
    {
        transform.Position += velocity.Linear * deltaTime;

        /*if (velocity.Angular.x != 0.0f ||
            velocity.Angular.y != 0.0f ||
            velocity.Angular.z != 0.0f)
        {
            auto deltaYaw = Quaternion::RotationFromAxisAngle(
                TransformComponent::GlobalUp, velocity.Angular.y);

            auto deltaRoll = Quaternion::RotationFromAxisAngle(
                TransformComponent::GlobalForward, velocity.Angular.z);

            transform.Rotation = deltaYaw * transform.Rotation;
            transform.Rotation = Diligent::normalize(transform.Rotation);

            auto localRight = transform.GetRight();
            auto deltaPitch = Quaternion::RotationFromAxisAngle(
                localRight, velocity.Angular.x);

            transform.Rotation = deltaPitch * transform.Rotation * deltaRoll;
            transform.Rotation = Diligent::normalize(transform.Rotation);
        }*/
    });
}
