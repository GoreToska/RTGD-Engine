//
// Created by gorev on 13.03.2026.
//

#pragma once
#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct CameraComponent
    {
        float FOVDegrees = 75.0f;
        float AspectRatio = 16.0f / 9.0f;
        float NearPlane = 0.1f;
        float FarPlane = 1000.0f;

        int Priority = 0;

        Matrix4 ViewMatrix = Matrix4::Identity();
        Matrix4 ProjectionMatrix = Matrix4::Identity();
    };

    struct EditorCameraMovementComponent
    {
        float MovementSpeed = 5.0f;
        float SprintMultiplier = 3.0f;
        float RotationSpeed = 0.1f;
    };
}
