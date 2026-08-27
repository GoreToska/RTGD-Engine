//
// Created by ivan on 8/27/26.
//

#pragma once
#include "Vertex.h"
#include "Components/TransformComponent.h"
#include "Tools/Alias.h"
#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    class ENGINE_API DebugDraw
    {
        DECLARE_SINGLETON(DebugDraw);

    public:
        void DrawLine(const Float3& a, const Float3& b, const Float4& color = {0, 1, 0, 1});

        void DrawArc(const Float3& center, const Float3& right, const Float3& up, float radius, float sweepRadians,
                     int segments, const Float4& color);

        void DrawBox(const Float3& center, const Float3& halfExtents, const Quaternion& rotation = QuaternionIdentity(),
                     const Float4& color = {0, 1, 0, 1});

        void DrawSphere(const Float3& center, float radius, const Float4& color = {0, 1, 0, 1});

        void DrawCapsule(const Float3& center, float radius, float halfHeight,
                         const Quaternion& rotation = QuaternionIdentity(), const Float4& color = {0, 1, 0, 1});

        std::vector<LineVertex> TakeLines();

    private:
        std::vector<LineVertex> m_lines = {};
    };

    DECLARE_GLOBAL_SINGLETON(DebugDraw, GDebugDraw);
} // RTGDEngine
