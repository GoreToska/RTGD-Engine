//
// Created by ivan on 8/27/26.
//

#include "Systems/GroundCheckSystem.h"

#include "Components/ColliderComponent.h"
#include "Components/GroundCheckComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/TransformComponent.h"

namespace
{
    float BottomOffset(RTGDEngine::EPhysicsShape shape, const Float3& extents)
    {
        switch (shape)
        {
            case RTGDEngine::EPhysicsShape::Box:
                return extents.y;
            case RTGDEngine::EPhysicsShape::Sphere:
                return extents.x;
            case RTGDEngine::EPhysicsShape::Capsule:
                return extents.x + extents.y;
        }

        return 0.0f;
    }
}

void RTGDEngine::GroundCheckSystem::Update(World& world, float deltaTime)
{
    world.each([](GroundCheckComponent& ground, const TransformComponent& transform, const ColliderComponent& collider,
                  const RigidbodyComponent& rb)
    {
        float distance = BottomOffset(collider.Shape, collider.Extents) + ground.Margin;
        JPH::BodyID self[] = {rb.BodyID};
        ground.IsGrounded = GPhysics.Raycast(transform.Position, {0, -1, 0}, distance, false, ground.LayerMask, self).
                Hit;
    });
}
