//
// Created by ivan on 8/28/26.
//

#pragma once

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "Event/Delegate.h"
#include "Systems/Physics/ICharacterController.h"
#include "Systems/Physics/PhysicalCharacterController.h"
#include "Systems/Physics/PhysicsSystem.h"
#include "Systems/Physics/VirtualCharacterController.h"
#include "Tools/Alias.h"
#include "AssetLoader/PathResolve.h"

namespace RTGDEngine
{
    struct CharacterControllerComponent
    {
        enum class EMode
        {
            Physical,
            Virtual,
        };

        EMode Mode = EMode::Virtual;
        float MaxSlopeAngle = 50.0f;
        float Mass = 70.0f;
        float MaxStrength = 100.0f; // Only for virtual EMode - max push strength of dynamic bodies

        // transient
        std::unique_ptr<ICharacterController> Controller;
        Delegate<PhysicsSystem, const Events::CollisionEnterEvent &> OnCollisionEnter;
        Delegate<PhysicsSystem, const Events::CollisionStayEvent &> OnCollisionStay;
        Delegate<PhysicsSystem, const Events::CollisionExitEvent &> OnCollisionExit;
        Delegate<PhysicsSystem, const Events::TriggerEnterEvent &> OnTriggerEnter;
        Delegate<PhysicsSystem, const Events::TriggerStayEvent &> OnTriggerStay;
        Delegate<PhysicsSystem, const Events::TriggerExitEvent &> OnTriggerExit;

        static void RegisterMeta(const World& world)
        {
            world.component<EMode>()
                    .constant("Physical", EMode::Physical)
                    .constant("Virtual", EMode::Virtual);

            world.component<CharacterControllerComponent>("CharacterControllerComponent")
                    .member<EMode>("Mode")
                    .member<float>("MaxSlopeAngle")
                    .member<float>("Mass")
                    .member<float>("MaxStrength");

            auto createController = [](Entity e)
            {
                auto charComp = e.get_ref<CharacterControllerComponent>();
                auto transform = e.get_ref<TransformComponent>();
                auto collider = e.get_ref<ColliderComponent>();
                if (!charComp || !transform || !collider)
                    return;

                if (collider->Shape == EPhysicsShape::Mesh || collider->Shape == EPhysicsShape::ConvexHull)
                {
                    LogError("Mesh/ConvexHull collider is not allowed on character controller, entity: {}, id: {}",
                             e.name().c_str(), e.id());
                    return;
                }

                charComp->Controller.reset();

                if (!collider->NativeShape)
                {
                    LogError("Shape is not valid, entity: {}, id: {}", e.name().c_str(), e.id());
                    return;
                }

                if (charComp->Mode == EMode::Physical)
                {
                    charComp->Controller = std::make_unique<PhysicalCharacterController>(
                        transform->Position, transform->Rotation, collider->NativeShape, charComp->Mass, collider->Friction,
                        charComp->MaxSlopeAngle, collider->Layer, e);
                }
                else
                {
                    charComp->Controller = std::make_unique<VirtualCharacterController>(
                        transform->Position, transform->Rotation, collider->NativeShape, charComp->Mass, charComp->MaxStrength,
                        charComp->MaxSlopeAngle, collider->Layer, e);
                }
            };

            world.observer<CharacterControllerComponent>().event(flecs::OnSet).each(
                [createController](Entity e, CharacterControllerComponent&)
                {
                    createController(e);
                });

            world.observer<TransformComponent>().event(flecs::OnSet).each(
                [createController](Entity e, TransformComponent&)
                {
                    auto charComp = e.get_ref<CharacterControllerComponent>();
                    if (charComp && !charComp->Controller)
                        createController(e);
                });

            world.observer<CharacterControllerComponent>().event(flecs::OnRemove).each(
                [](Entity e, CharacterControllerComponent& c)
                {
                    c.Controller.reset();
                });
        }
    };
}
