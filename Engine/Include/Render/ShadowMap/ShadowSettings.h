//
// Created by ivan on 7/22/26.
//
#pragma once
#include <cstdint>

namespace RTGDEngine {
    struct ShadowSettings {
        uint32_t Resolution = 2048;
        uint32_t CascadeCount = 4;
        float SplitLambda = 0.5f;
        float ShadowDistance = 100.0f;
        float DepthBias = 0.005f;
        float NormalBias = 0.02f;
    };
}
