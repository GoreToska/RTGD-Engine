//
// Created by gorev on 13.03.2026.
//

#include "Systems/EditorCameraSystem.h"

#include "Components/CameraComponent.h"
#include "Components/TransformComponent.h"
#include "Components/VelocityComponent.h"
#include "Input/InputSystem.h"
#include "Tools/Logger.h"

namespace RTGDEngine {
    void EditorCameraSystem::Update(const flecs::world &world, float deltaTime) {
        auto &input = InputSystem::Instance();

        world.each([&](EditorCameraMovementComponent &editorCam,
                       VelocityComponent &velocity,
                       TransformComponent &transform) {
            Float3 dir{};
            velocity.Linear = {0.0f, 0.0f, 0.0f};
            velocity.Angular = {0.0f, 0.0f, 0.0f};

            if (!input.IsMouseCaptured())
                return;

            float dx = input.GetMouseDeltaX() * editorCam.RotationSpeed;
            float dy = input.GetMouseDeltaY() * editorCam.RotationSpeed;

            if (dx != 0.0f)
                transform.Rotate(TransformComponent::GlobalUp, dx, World);

            if (dy != 0.0f) {
                float desired = editorCam.CurrentPitch + dy;
                float clamped = std::clamp(desired, -editorCam.PitchLimit, editorCam.PitchLimit);
                float applied = clamped - editorCam.CurrentPitch;
                editorCam.CurrentPitch = clamped;

                if (applied != 0.0f)
                    transform.Rotate(TransformComponent::GlobalRight, applied, Local);
            }

            float speed = editorCam.MovementSpeed;
            if (input.IsDown(EInputAction::SpeedBoost))
                speed *= editorCam.SprintMultiplier;

            const auto forward = transform.GetForward();
            const auto right = transform.GetRight();

            if (input.IsDown(EInputAction::MoveForward))
                dir += forward;
            if (input.IsDown(EInputAction::MoveBackward))
                dir -= forward;
            if (input.IsDown(EInputAction::MoveRight))
                dir += right;
            if (input.IsDown(EInputAction::MoveLeft))
                dir -= right;
            if (input.IsDown(EInputAction::MoveUp))
                dir.y += 1.0f;
            if (input.IsDown(EInputAction::MoveDown))
                dir.y -= 1.0f;

            if (Diligent::length(dir) > 0.000001f)
                dir = Diligent::normalize(dir);

            velocity.Linear = dir * speed;
        });
    }
} // RTGDEngine
