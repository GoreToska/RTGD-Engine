//
// Created by ivan on 8/27/26.
//

#pragma once
#include "Vertex.h"
#include "Components/TransformComponent.h"
#include "Tools/Alias.h"
#include "Engine/Math.h"
#include "Engine/EngineExport.h"
#include "Tools/RTGDMacros.h"

namespace RTGDEngine
{
    class ENGINE_API DebugDraw
    {
        DECLARE_SINGLETON(DebugDraw);

    public:
        static constexpr float Persistent = -1.0f;

        void DrawLine(const Float3& a, const Float3& b, const Float4& color = {0, 1, 0, 1}, float duration = 0.0f);

        void DrawArc(const Float3& center, const Float3& right, const Float3& up, float radius, float sweepRadians,
                     int segments, const Float4& color, float duration = 0.0f);

        void DrawBox(const Float3& center, const Float3& halfExtents, const Quaternion& rotation = QuaternionIdentity(),
                     const Float4& color = {0, 1, 0, 1}, float duration = 0.0f);

        void DrawSphere(const Float3& center, float radius, const Float4& color = {0, 1, 0, 1}, float duration = 0.0f);

        void DrawCapsule(const Float3& center, float radius, float halfHeight,
                         const Quaternion& rotation = QuaternionIdentity(), const Float4& color = {0, 1, 0, 1},
                         float duration = 0.0f);

        std::vector<LineVertex> TakeLines();

        void ClearPersistent();

    private:
        struct TimedLine
        {
            LineVertex A, B;
            std::chrono::steady_clock::time_point Expire;
        };

        std::vector<LineVertex> m_lines = {};
        std::vector<TimedLine> m_timedLines = {};
    };

    DECLARE_GLOBAL_SINGLETON(DebugDraw, GDebugDraw);
} // RTGDEngine
