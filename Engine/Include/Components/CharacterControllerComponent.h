//
// Created by ivan on 8/28/26.
//

#pragma once

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "Systems/Physics/ICharacterController.h"
#include "Systems/Physics/PhysicalCharacterController.h"
#include "Systems/Physics/VirtualCharacterController.h"
#include "Tools/Alias.h"

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

                charComp->Controller.reset();
                JPH::ShapeRefC shape = ColliderComponent::MakeShape(collider->Shape, collider->Extents);

                if (charComp->Mode == EMode::Physical)
                {
                    charComp->Controller = std::make_unique<PhysicalCharacterController>(
                        transform->Position, transform->Rotation, shape, charComp->Mass, collider->Friction,
                        charComp->MaxSlopeAngle, collider->Layer, e);
                }
                else
                {
                    charComp->Controller = std::make_unique<VirtualCharacterController>(
                        transform->Position, transform->Rotation, shape, charComp->Mass, charComp->MaxStrength,
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
