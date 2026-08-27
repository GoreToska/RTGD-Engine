//
// Created by ivan on 8/27/26.
//

#include "Render/DebugDraw.h"

namespace RTGDEngine
{
    void DebugDraw::DrawLine(const Float3& a, const Float3& b, const Float4& color)
    {
        m_lines.push_back({a, color});
        m_lines.push_back({b, color});
    }

    void DebugDraw::DrawArc(const Float3& center, const Float3& right, const Float3& up, float radius,
                            float sweepRadians, int segments, const Float4& color)
    {
        Float3 prev = center + right * radius;
        for (int i = 1; i <= segments; ++i)
        {
            float t = sweepRadians * i / segments;
            Float3 p = center + right * (cosf(t) * radius) + up * (sinf(t) * radius);
            DrawLine(prev, p, color);
            prev = p;
        }
    }

    void DebugDraw::DrawBox(const Float3& center, const Float3& halfExtents, const Quaternion& rotation,
                            const Float4& color)
    {
        Float3 right, up, fwd;
        GetBasis(rotation, right, up, fwd);
        Float3 c[8];
        int i = 0;
        for (int bx = -1; bx <= 1; bx += 2)
        {
            for (int by = -1; by <= 1; by += 2)
            {
                for (int bz = -1; bz <= 1; bz += 2)
                {
                    c[i++] = center + right * (bx * halfExtents.x) + up * (by * halfExtents.y) + fwd * (
                                 bz * halfExtents.z);
                }
            }
        }

        static constexpr int edges[12][2] = {
            {0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };

        for (auto& edge: edges)
            DrawLine(c[edge[0]], c[edge[1]], color);
    }

    void DebugDraw::DrawSphere(const Float3& center, float radius, const Float4& color)
    {
        DrawArc(center, {1, 0, 0}, {0, 1, 0}, radius, 2 * Diligent::PI, 24, color);
        DrawArc(center, {1, 0, 0}, {0, 0, 1}, radius, 2 * Diligent::PI, 24, color);
        DrawArc(center, {0, 1, 0}, {0, 0, 1}, radius, 2 * Diligent::PI, 24, color);
    }

    void DebugDraw::DrawCapsule(const Float3& center, float radius, float halfHeight, const Quaternion& rotation,
                                const Float4& color)
    {
        Float3 right, up, fwd;
        GetBasis(rotation, right, up, fwd);
        Float3 top = center + up * halfHeight;
        Float3 bottom = center - up * halfHeight;

        DrawArc(top, right, fwd, radius, 2 * Diligent::PI, 24, color);
        DrawArc(bottom, right, fwd, radius, 2 * Diligent::PI, 24, color);

        for (int i = 0; i < 4; ++i)
        {
            float a = i * (Diligent::PI * 0.5f);
            Float3 dir = right * cosf(a) + fwd * sinf(a);
            DrawArc(top, right, up, radius, Diligent::PI, 12, color);
            DrawArc(top, fwd, up, radius, Diligent::PI, 12, color);
            DrawArc(bottom, right, up, radius, Diligent::PI, 12, color);
            DrawArc(bottom, fwd, up, radius, Diligent::PI, 12, color);
        }
    }

    std::vector<LineVertex> DebugDraw::TakeLines()
    {
        std::vector<LineVertex> result;
        std::swap(result, m_lines);
        return result;
    }
} // RTGDEngine
