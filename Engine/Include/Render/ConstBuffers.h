//
// Created by gorev on 13.03.2026.
//

#pragma once
#include "Tools/Alias.h"

namespace RTGDEngine {
    static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 1;
    static constexpr uint32_t MAX_POINT_LIGHTS = 64;
    static constexpr uint32_t MAX_SPOT_LIGHTS = 16;
    static constexpr uint32_t MAX_SHADOW_CASCADES = 4;

    struct alignas(16) CameraConstantBuffer {
        Matrix4 View;
        Matrix4 Projection;
        Float4 CameraPosition;
    };

    struct alignas(16) ObjectConstantBuffer {
        Matrix4 Model;
#ifdef RTGD_EDITOR
        uint32_t EntityID;
        uint32_t _pad[3];
#endif
    };


    struct alignas(16) DirectionalLightData {
        Float3 Direction = {0.0f, -1.0f, 0.0f};
        float Intensity = 1.0f;
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float _pad = 0.0f;
    };

    struct alignas(16) PointLightData {
        Float3 Position = {0.0f, 0.0f, 0.0f};
        float Radius = 10.0f;
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
    };

    struct alignas(16) SpotLightData {
        Float3 Position = {0.0f, 0.0f, 0.0f};
        float InnerAngle = 0.0f; // rad
        Float3 Direction = {0.0f, -1.0f, 0.0f};
        float OuterAngle = 0.0f; // rad
        Float3 Color = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
        float Radius = 20.0f;
        float _pad[3] = {};
    };

    struct alignas(16) LightConstantBuffer {
        Float3 AmbientColor = {0.2f, 0.2f, 0.2f};
        float AmbientIntensity = 0.1f;

        uint32_t DirectionalCount = 0;
        uint32_t PointCount = 0;
        uint32_t SpotCount = 0;
        float _pad = 0.0f;

        DirectionalLightData DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
        PointLightData PointLights[MAX_POINT_LIGHTS];
        SpotLightData SpotLights[MAX_SPOT_LIGHTS];
    };

    struct alignas(16) ShadowConstantBuffer {
        Matrix4 LightViewProjection[MAX_SHADOW_CASCADES];
        Float4 CascadeSplits;
        Float4 AtlasRects[MAX_SHADOW_CASCADES]; // xy = UV offset, zw - UV scale
        Float4 Params; // x - DepthBias, y - NormalBias, z - TexelSize, W - CascadeCount
    };
}
